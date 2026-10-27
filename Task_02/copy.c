
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/sendfile.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: ./copy METHOD BUFFER INPUT OUTPUT\n");
        return 1;
    }

    char *method = argv[1];
    size_t size = atol(argv[2]);
    char *input = argv[3];
    char *output = argv[4];

    if (size == 0) return 1;

    if (strcmp(method, "fread") == 0) {
        FILE *in = fopen(input, "rb");
        FILE *out = fopen(output, "wb");

        if (!in || !out) {
            perror("fopen");
            return 1;
        }

        char *buffer = malloc(size);
        if (!buffer) return 1;

        size_t n;
        while ((n = fread(buffer, 1, size, in)) > 0) {
            if (fwrite(buffer, 1, n, out) != n) {
                perror("fwrite");
                return 1;
            }
        }

        if (ferror(in)) return 1;
        if (fclose(out) != 0) return 1;
        fclose(in);
        free(buffer);

    } else {
        int in = open(input, O_RDONLY);
        int out = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (in < 0 || out < 0) {
            perror("open");
            return 1;
        }

        if (strcmp(method, "sendfile") == 0) {
            struct stat st;
            if (fstat(in, &st) != 0) return 1;

            off_t remaining = st.st_size;

            while (remaining > 0) {
                ssize_t n = sendfile(out, in, NULL, remaining);
                if (n <= 0) {
                    perror("sendfile");
                    return 1;
                }
                remaining -= n;
            }

        } else {
            char *buffer = malloc(size);
            if (!buffer) return 1;

            ssize_t n;
            while ((n = read(in, buffer, size)) > 0) {
                ssize_t written = 0;

                while (written < n) {
                    ssize_t m;

                    if (strcmp(method, "syscall") == 0) {
                        m = syscall(SYS_write, out,
                                    buffer + written, n - written);
                    } else {
                        m = write(out, buffer + written, n - written);
                    }

                    if (m <= 0) {
                        perror("write");
                        return 1;
                    }

                    written += m;
                }
            }

            if (n < 0) {
                perror("read");
                return 1;
            }

            free(buffer);
        }

        close(in);
        if (close(out) != 0) return 1;
    }

    printf("Copy completed using %s\n", method);
    return 0;
}
