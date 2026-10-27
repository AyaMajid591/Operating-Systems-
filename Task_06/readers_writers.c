
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define READERS 8
#define WRITES 20

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t can_read = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_write = PTHREAD_COND_INITIALIZER;

int active_readers = 0;
int active_writer = 0;
int waiting_writers = 0;
int reader_priority = 1;
atomic_int shared_value = 0;

atomic_int running = 1;
atomic_long reads_done = 0;

double now() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

void *reader(void *arg) {
    (void)arg;

    while (atomic_load(&running)) {
        pthread_mutex_lock(&mutex);

        while (active_writer ||
               (!reader_priority && waiting_writers > 0)) {
            pthread_cond_wait(&can_read, &mutex);
        }

        active_readers++;
        pthread_mutex_unlock(&mutex);

        int value = atomic_load(&shared_value);
        (void)value;
        atomic_fetch_add(&reads_done, 1);
        usleep(1000);

        pthread_mutex_lock(&mutex);
        active_readers--;

        if (active_readers == 0)
            pthread_cond_signal(&can_write);

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void *writer(void *arg) {
    (void)arg;

    for (int i = 0; i < WRITES; i++) {
        pthread_mutex_lock(&mutex);
        waiting_writers++;

        while (active_writer || active_readers > 0)
            pthread_cond_wait(&can_write, &mutex);

        waiting_writers--;
        active_writer = 1;
        pthread_mutex_unlock(&mutex);

        atomic_fetch_add(&shared_value, 1);
        usleep(1000);

        pthread_mutex_lock(&mutex);
        active_writer = 0;

        if (reader_priority) {
            pthread_cond_broadcast(&can_read);
            pthread_cond_signal(&can_write);
        } else if (waiting_writers > 0) {
            pthread_cond_signal(&can_write);
        } else {
            pthread_cond_broadcast(&can_read);
        }

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: ./readers_writers reader|writer\n");
        return 1;
    }

    reader_priority = strcmp(argv[1], "reader") == 0;

    if (!reader_priority && strcmp(argv[1], "writer") != 0)
        return 1;

    pthread_t readers[READERS];
    pthread_t w;

    for (int i = 0; i < READERS; i++)
        pthread_create(&readers[i], NULL, reader, NULL);

    usleep(10000);

    double start = now();
    pthread_create(&w, NULL, writer, NULL);

    pthread_join(w, NULL);
    double elapsed = now() - start;

    atomic_store(&running, 0);

    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&can_read);
    pthread_mutex_unlock(&mutex);

    for (int i = 0; i < READERS; i++)
        pthread_join(readers[i], NULL);

    printf("Policy: %s\n", argv[1]);
    printf("Writer completion time: %.4f seconds\n", elapsed);
    printf("Completed writes: %d\n", shared_value);
    printf("Reader operations: %ld\n", atomic_load(&reads_done));

    pthread_cond_destroy(&can_read);
    pthread_cond_destroy(&can_write);
    pthread_mutex_destroy(&mutex);
    return 0;
}
