
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define SIZE 100000000
#define MAX_THREADS 16

long long *array;

typedef struct {
    long start;
    long end;
    long long sum;
} Task;

void *calculate(void *arg) {
    Task *task = arg;
    task->sum = 0;

    for (long i = task->start; i < task->end; i++) {
        task->sum += array[i];
    }

    return NULL;
}

double get_time() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

int main() {
    array = malloc(SIZE * sizeof(long long));

    if (array == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    for (long i = 0; i < SIZE; i++) {
        array[i] = 1;
    }

    int counts[] = {1, 2, 4, 8, 16};

    FILE *file = fopen("sum_results.csv", "w");
    if (!file) return 1;

    fprintf(file, "threads,time,speedup\n");

    double baseline = 0;

    for (int k = 0; k < 5; k++) {
        int n = counts[k];
        pthread_t threads[MAX_THREADS];
        Task tasks[MAX_THREADS];

        double start = get_time();

        for (int i = 0; i < n; i++) {
            tasks[i].start = (long)i * SIZE / n;
            tasks[i].end = (long)(i + 1) * SIZE / n;

            if (pthread_create(&threads[i], NULL, calculate, &tasks[i]) != 0) {
                return 1;
            }
        }

        long long total = 0;

        for (int i = 0; i < n; i++) {
            pthread_join(threads[i], NULL);
            total += tasks[i].sum;
        }

        double elapsed = get_time() - start;

        if (k == 0) baseline = elapsed;

        printf("%d threads: %.4f seconds, sum = %lld\n",
               n, elapsed, total);

        fprintf(file, "%d,%.6f,%.3f\n",
                n, elapsed, baseline / elapsed);
    }

    fclose(file);
    free(array);
    return 0;
}

