
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <mqueue.h>
#include <sys/wait.h>

#define TOTAL (1024ULL * 1024 * 1024)
#define CHUNK 4096

volatile sig_atomic_t stopped = 0;

void handle_signal(int sig) {
    (void)sig;
    stopped = 1;
}

double now() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./ipc_queue SIZE\n");
        return 1;
    }

    size_t size = strtoull(argv[1], NULL, 10);

    if (size != 64 && size != 4096 && size != 1048576) {
        printf("Use 64, 4096 or 1048576\n");
        return 1;
    }

    struct sigaction sa = {0};
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    char name[64];
    snprintf(name, sizeof(name), "/ipc_task7_%ld", (long)getpid());

    struct mq_attr attr = {0};
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = CHUNK;

    mqd_t queue = mq_open(name, O_CREAT | O_EXCL | O_RDWR, 0600, &attr);

    if (queue == (mqd_t)-1) {
        perror("mq_open");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        mq_close(queue);
        mq_unlink(name);
        return 1;
    }

    size_t chunk = size < CHUNK ? size : CHUNK;
    char buffer[CHUNK];
    memset(buffer, 'A', sizeof(buffer));

    if (pid == 0) {
        uint64_t received = 0;

        while (received < TOTAL && !stopped) {
            ssize_t n = mq_receive(queue, buffer, CHUNK, NULL);

            if (n > 0)
                received += n;
            else if (errno != EINTR)
                break;
        }

        mq_close(queue);
        _exit(received == TOTAL ? 0 : 1);
    }

    double start = now();
    uint64_t sent = 0;

    while (sent < TOTAL && !stopped) {
        if (mq_send(queue, buffer, chunk, 0) == 0)
            sent += chunk;
        else if (errno != EINTR)
            break;
    }

    if (sent != TOTAL) {
        kill(pid, SIGTERM);
    }

    int status = 0;
    waitpid(pid, &status, 0);

    double elapsed = now() - start;

    mq_close(queue);
    mq_unlink(name);

    if (sent != TOTAL || !WIFEXITED(status) ||
        WEXITSTATUS(status) != 0) {
        printf("Transfer failed or interrupted\n");
        return 1;
    }

    printf("Method: POSIX Message Queue\n");
    printf("Logical message size: %zu bytes\n", size);
    printf("Queue chunk size: %zu bytes\n", chunk);
    printf("Transfer time: %.4f seconds\n", elapsed);
    printf("Throughput: %.2f MiB/s\n", 1024.0 / elapsed);
    printf("Average time per logical message: %.4f us\n",
           elapsed * 1000000.0 / (TOTAL / size));

    return 0;
}
