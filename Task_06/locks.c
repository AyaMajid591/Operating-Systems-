
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sched.h>
#include <string.h>

#define ROUNDS 10000

unsigned char spin = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
long counter = 0;
volatile unsigned long work = 0;

int use_spin;
int work_size;

double now() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

void spin_lock() {
    while (__atomic_test_and_set(&spin, __ATOMIC_ACQUIRE)) {
        sched_yield();
    }
}

void spin_unlock() {
    __atomic_clear(&spin, __ATOMIC_RELEASE);
}

void *worker(void *arg) {
    (void)arg;

    for (int i = 0; i < ROUNDS; i++) {
        if (use_spin)
            spin_lock();
        else
            pthread_mutex_lock(&mutex);

        for (int j = 0; j < work_size; j++)
            work++;

        counter++;

        if (use_spin)
            spin_unlock();
        else
            pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main() {
    int counts[] = {2, 4, 16};

    FILE *file = fopen("lock_results.csv", "w");
    if (!file) return 1;

    fprintf(file, "lock,threads,section,time\n");

    for (int type = 0; type < 2; type++) {
        use_spin = type;

        for (int section = 0; section < 2; section++) {
            work_size = section == 0 ? 1 : 1000;

            for (int k = 0; k < 3; k++) {
                int n = counts[k];
                pthread_t threads[16];
                counter = 0;
                work = 0;
                __atomic_clear(&spin, __ATOMIC_RELEASE);

                double start = now();

                for (int i = 0; i < n; i++) {
                    if (pthread_create(&threads[i], NULL, worker, NULL) != 0)
                        return 1;
                }

                for (int i = 0; i < n; i++)
                    pthread_join(threads[i], NULL);

                double elapsed = now() - start;

                if (counter != (long)n * ROUNDS) {
                    fprintf(stderr, "Incorrect counter\n");
                    return 1;
                }

                char *name = use_spin ? "Spinlock" : "Mutex";
                char *length = section == 0 ? "Short" : "Long";

                printf("%s, %d threads, %s: %.4f seconds\n",
                       name, n, length, elapsed);

                fprintf(file, "%s,%d,%s,%.6f\n",
                        name, n, length, elapsed);
            }
        }
    }

    fclose(file);
    pthread_mutex_destroy(&mutex);
    return 0;
}
