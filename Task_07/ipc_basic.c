
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>
#define TOTAL (1024ULL * 1024 * 1024)

volatile sig_atomic_t stopped = 0;

void stop_handler(int sig) {
    (void)sig;
    stopped = 1;
}

double now() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

int send_data(int fd, char *buffer, size_t size) {
    size_t done = 0;

    while (done < size && !stopped) {
        ssize_t n = write(fd, buffer + done, size - done);

        if (n > 0)
            done += n;
        else if (n < 0 && errno == EINTR)
            continue;
        else
            return -1;
    }

    return done == size ? 0 : -1;
}

int receive_data(int fd, char *buffer, size_t size) {
    size_t done = 0;

    while (done < size && !stopped) {
        ssize_t n = read(fd, buffer + done, size - done);

        if (n > 0)
            done += n;
        else if (n < 0 && errno == EINTR)
            continue;
        else
            return -1;
    }

    return done == size ? 0 : -1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./ipc_basic pipe|socket SIZE\n");
        return 1;
    }

    signal(SIGTERM, stop_handler);
    signal(SIGINT, stop_handler);
    signal(SIGPIPE, SIG_IGN);

    size_t size = strtoull(argv[2], NULL, 10);

    if (size == 0 || size > 1048576 || TOTAL % size != 0)
        return 1;

    char *buffer = malloc(size);

    if (!buffer)
        return 1;

    memset(buffer, 'A', size);

    int fd[2];

    if (strcmp(argv[1], "pipe") == 0) {
        if (pipe(fd) == -1)
            return 1;
    } else if (strcmp(argv[1], "socket") == 0) {
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == -1)
            return 1;
    } else {
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0)
        return 1;

    if (pid == 0) {
        close(fd[1]);

        uint64_t received = 0;

        while (received < TOTAL && !stopped) {
            if (receive_data(fd[0], buffer, size) != 0)
                break;

            received += size;
        }

        close(fd[0]);
        free(buffer);

        _exit(received == TOTAL ? 0 : 1);
    }

    close(fd[0]);

    double start = now();
    uint64_t sent = 0;

    while (sent < TOTAL && !stopped) {
        if (send_data(fd[1], buffer, size) != 0)
            break;

        sent += size;
    }

    close(fd[1]);

    int status;
    waitpid(pid, &status, 0);

    double elapsed = now() - start;
    double throughput = (sent / 1048576.0) / elapsed;
    double avg_us = elapsed * 1000000.0 / (sent / size);

    if (sent != TOTAL || !WIFEXITED(status) ||
        WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Transfer failed or interrupted\n");
        free(buffer);
        return 1;
    }

    printf("Method: %s\n", argv[1]);
    printf("Message size: %zu bytes\n", size);
    printf("Transfer time: %.4f seconds\n", elapsed);
    printf("Throughput: %.2f MiB/s\n", throughput);
    printf("Average time per message: %.4f us\n", avg_us);

    free(buffer);
    return 0;
}
