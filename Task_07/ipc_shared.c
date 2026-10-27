
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define TOTAL (1024ULL * 1024 * 1024)
#define MAX_SIZE 1048576

typedef struct {
    sem_t empty;
    sem_t full;
    size_t size;
    char data[MAX_SIZE];
} Shared;

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

int wait_sem(sem_t *sem) {
    while (!stopped) {
        if (sem_wait(sem) == 0)
            return 0;
        if (errno != EINTR)
            return -1;
    }
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./ipc_shared SIZE\n");
        return 1;
    }

    size_t size = strtoull(argv[1], NULL, 10);

    if (size != 64 && size != 4096 && size != MAX_SIZE)
        return 1;

    struct sigaction sa = {0};
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    char name[64];
    snprintf(name, sizeof(name), "/ipc_shm_%ld", (long)getpid());

    int fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);

    if (fd == -1) {
        perror("shm_open");
        return 1;
    }

    if (ftruncate(fd, sizeof(Shared)) == -1) {
        perror("ftruncate");
        close(fd);
        shm_unlink(name);
        return 1;
    }

    Shared *shared = mmap(NULL, sizeof(Shared),
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED, fd, 0);

    close(fd);

    if (shared == MAP_FAILED) {
        perror("mmap");
        shm_unlink(name);
        return 1;
    }

    if (sem_init(&shared->empty, 1, 1) != 0 ||
        sem_init(&shared->full, 1, 0) != 0) {
        perror("sem_init");
        munmap(shared, sizeof(Shared));
        shm_unlink(name);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        sem_destroy(&shared->empty);
        sem_destroy(&shared->full);
        munmap(shared, sizeof(Shared));
        shm_unlink(name);
        return 1;
    }

    if (pid == 0) {
        uint64_t received = 0;
        volatile unsigned char check = 0;

        while (received < TOTAL && !stopped) {
            if (wait_sem(&shared->full) != 0)
                break;

            for (size_t i = 0; i < size; i++)
                check ^= (unsigned char)shared->data[i];

            received += size;
            sem_post(&shared->empty);
        }

        munmap(shared, sizeof(Shared));
        _exit(received == TOTAL ? 0 : 1);
    }

    double start = now();
    uint64_t sent = 0;

    while (sent < TOTAL && !stopped) {
        if (wait_sem(&shared->empty) != 0)
            break;

        memset(shared->data, 'A', size);
        shared->size = size;

        sem_post(&shared->full);
        sent += size;
    }

    if (sent != TOTAL)
        kill(pid, SIGTERM);

    int status = 0;
    waitpid(pid, &status, 0);

    double elapsed = now() - start;

    sem_destroy(&shared->empty);
    sem_destroy(&shared->full);
    munmap(shared, sizeof(Shared));
    shm_unlink(name);

    if (sent != TOTAL || !WIFEXITED(status) ||
        WEXITSTATUS(status) != 0) {
        printf("Transfer failed or interrupted\n");
        return 1;
    }

    printf("Method: Shared Memory\n");
    printf("Message size: %zu bytes\n", size);
    printf("Transfer time: %.4f seconds\n", elapsed);
    printf("Throughput: %.2f MiB/s\n", 1024.0 / elapsed);
    printf("Average time per message: %.4f us\n",
           elapsed * 1000000.0 / (TOTAL / size));

    return 0;
}
