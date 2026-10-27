
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#define N 5

typedef struct {
    atomic_ulong meals;
    atomic_ulong attempts;
    atomic_ulong failures;
    atomic_ulong max_wait_ns;
} Stats;

static pthread_mutex_t forks[N];
static pthread_barrier_t sync_point;
static sem_t waiter;
static Stats *stats;
static int mode;

static unsigned long now_ns(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (unsigned long)t.tv_sec * 1000000000UL + t.tv_nsec;
}

static void pause_short(void) {
    struct timespec t = {0, 1000000};
    nanosleep(&t, NULL);
}

static void update_max(int id, unsigned long value) {
    unsigned long old = atomic_load(&stats[id].max_wait_ns);

    while (value > old &&
           !atomic_compare_exchange_weak(
               &stats[id].max_wait_ns, &old, value)) {
    }
}

static void *philosopher(void *arg) {
    int id = *(int *)arg;
    int left = id;
    int right = (id + 1) % N;

    while (1) {
        pause_short();

        unsigned long start = now_ns();
        atomic_fetch_add(&stats[id].attempts, 1);

        if (mode == 0) {
            pthread_mutex_lock(&forks[left]);
            pthread_barrier_wait(&sync_point);
            pthread_mutex_lock(&forks[right]);
        } else if (mode == 1) {
            int first = left < right ? left : right;
            int second = left < right ? right : left;

            pthread_mutex_lock(&forks[first]);
            pthread_mutex_lock(&forks[second]);
        } else if (mode == 2) {
            pthread_mutex_lock(&forks[left]);
            pthread_barrier_wait(&sync_point);

            int ok = pthread_mutex_trylock(&forks[right]);

            if (ok != 0)
                atomic_fetch_add(&stats[id].failures, 1);

            pthread_barrier_wait(&sync_point);

            if (ok == 0)
                pthread_mutex_unlock(&forks[right]);

            pthread_mutex_unlock(&forks[left]);
            pthread_barrier_wait(&sync_point);
            continue;
        } else {
            sem_wait(&waiter);
            pthread_mutex_lock(&forks[left]);
            pthread_mutex_lock(&forks[right]);
        }

        update_max(id, now_ns() - start);
        atomic_fetch_add(&stats[id].meals, 1);
        pause_short();

        if (mode == 1) {
            int first = left < right ? left : right;
            int second = left < right ? right : left;

            pthread_mutex_unlock(&forks[second]);
            pthread_mutex_unlock(&forks[first]);
        } else {
            pthread_mutex_unlock(&forks[right]);
            pthread_mutex_unlock(&forks[left]);

            if (mode == 3)
                sem_post(&waiter);
        }
    }

    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr,
                "Usage: %s naive|ordered|trylock|waiter [seconds]\n",
                argv[0]);
        return 1;
    }

    const char *names[] = {
        "naive", "ordered", "trylock", "waiter"
    };

    mode = -1;

    for (int i = 0; i < 4; i++) {
        if (strcmp(argv[1], names[i]) == 0)
            mode = i;
    }

    if (mode < 0)
        return 1;

    int seconds = argc == 3 ? atoi(argv[2]) : 10;

    if (seconds < 1 || seconds > 30)
        return 1;

    stats = mmap(NULL, sizeof(Stats) * N,
                 PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (stats == MAP_FAILED)
        return 1;

    memset(stats, 0, sizeof(Stats) * N);

    pid_t pid = fork();

    if (pid < 0) {
        munmap(stats, sizeof(Stats) * N);
        return 1;
    }

    if (pid == 0) {
        pthread_t threads[N];
        int ids[N];

        for (int i = 0; i < N; i++)
            pthread_mutex_init(&forks[i], NULL);

        pthread_barrier_init(&sync_point, NULL, N);
        sem_init(&waiter, 0, N - 1);

        for (int i = 0; i < N; i++) {
            ids[i] = i;

            if (pthread_create(
                    &threads[i], NULL, philosopher, &ids[i]) != 0)
                _exit(2);
        }

        for (int i = 0; i < N; i++)
            pthread_join(threads[i], NULL);

        _exit(0);
    }

    sleep(seconds);

    kill(pid, SIGKILL);
    waitpid(pid, NULL, 0);

    printf("Method: %s\n", names[mode]);
    printf("Duration: %d seconds\n", seconds);

    unsigned long total = 0;

    for (int i = 0; i < N; i++) {
        unsigned long meals = atomic_load(&stats[i].meals);
        total += meals;

        printf(
            "P%d: meals=%lu attempts=%lu "
            "failed_trylocks=%lu max_completed_wait=%.3f ms\n",
            i + 1,
            meals,
            atomic_load(&stats[i].attempts),
            atomic_load(&stats[i].failures),
            atomic_load(&stats[i].max_wait_ns) / 1000000.0
        );
    }

    printf("Total meals: %lu\n", total);

    if (mode == 0)
        puts("Deadlock: all philosophers hold their left fork and wait for their right fork.");

    if (mode == 2)
        puts("Livelock: philosophers keep retrying but complete no meals.");

    munmap(stats, sizeof(Stats) * N);

    return 0;
}
