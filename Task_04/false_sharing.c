
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>
#include <stdlib.h>

#define ITERATIONS 10000000

typedef struct {
    _Atomic long value;
    char padding[64];
} Counter;

_Atomic long adjacent[2];
Counter separated[2];

typedef struct {
    int id;
    int padded;
} Task;

void *update(void *arg) {
    Task *task = arg;

    for (long i = 0; i < ITERATIONS; i++) {
        if (task->padded) {
            atomic_fetch_add_explicit(
                &separated[task->id].value, 1, memory_order_relaxed);
        } else {
            atomic_fetch_add_explicit(
                &adjacent[task->id], 1, memory_order_relaxed);
        }
    }

    return NULL;
}

double get_time() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

double test(int padded) {
    pthread_t threads[2];
    Task tasks[2] = {{0, padded}, {1, padded}};

    double start = get_time();

    pthread_create(&threads[0], NULL, update, &tasks[0]);
    pthread_create(&threads[1], NULL, update, &tasks[1]);

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    return get_time() - start;
}

int main() {
    double a = test(0);
    double b = test(1);

    printf("Adjacent: %.4f seconds\n", a);
    printf("Separated: %.4f seconds\n", b);

    FILE *file = fopen("sharing_results.csv", "w");
    if (!file) return 1;

    fprintf(file, "type,time\n");
    fprintf(file, "Adjacent,%.6f\n", a);
    fprintf(file, "Separated,%.6f\n", b);

    fclose(file);
    return 0;
}
