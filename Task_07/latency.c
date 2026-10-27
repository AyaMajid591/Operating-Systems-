
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>

#define COUNT 10000

double now() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

int main() {
    int forward[2], backward[2];

    if (pipe(forward) != 0 || pipe(backward) != 0)
        return 1;

    pid_t pid = fork();

    if (pid < 0)
        return 1;

    char c = 'A';

    if (pid == 0) {
        close(forward[1]);
        close(backward[0]);

        for (int i = 0; i < COUNT; i++) {
            if (read(forward[0], &c, 1) != 1)
                _exit(1);

            if (write(backward[1], &c, 1) != 1)
                _exit(1);
        }

        close(forward[0]);
        close(backward[1]);
        _exit(0);
    }

    close(forward[0]);
    close(backward[1]);

    double start = now();

    for (int i = 0; i < COUNT; i++) {
        if (write(forward[1], &c, 1) != 1)
            return 1;

        if (read(backward[0], &c, 1) != 1)
            return 1;
    }

    double elapsed = now() - start;

    close(forward[1]);
    close(backward[0]);

    int status;
    waitpid(pid, &status, 0);

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        return 1;

    printf("Round-trip latency: %.3f microseconds\n",
           elapsed * 1000000 / COUNT);

    printf("Estimated one-way latency: %.3f microseconds\n",
           elapsed * 1000000 / COUNT / 2);

    return 0;
}
