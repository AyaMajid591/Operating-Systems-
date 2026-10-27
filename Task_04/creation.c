
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define COUNT 10000

void *empty_task(void *arg) {
    (void)arg;
    return NULL;
}

double get_time() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

int main() {
    pthread_t thread;

    double start = get_time();

    for (int i = 0; i < COUNT; i++) {
        pthread_create(&thread, NULL, empty_task, NULL);
        pthread_join(thread, NULL);
    }

    double thread_time = get_time() - start;

    start = get_time();

    for (int i = 0; i < COUNT; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            _exit(0);
        }

        waitpid(pid, NULL, 0);
    }

    double process_time = get_time() - start;

    printf("10000 threads: %.4f seconds\n", thread_time);
    printf("10000 processes: %.4f seconds\n", process_time);

    FILE *file = fopen("creation_results.csv", "w");
    if (!file) return 1;

    fprintf(file, "type,time\n");
    fprintf(file, "Threads,%.6f\n", thread_time);
    fprintf(file, "Processes,%.6f\n", process_time);

    fclose(file);
    return 0;
}
