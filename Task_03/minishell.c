#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_ARGS 64
#define MAX_CMDS 16
#define MAX_TOKENS 256

typedef struct {
    char *args[MAX_ARGS];
    char *input;
    char *output;
} Command;

static volatile sig_atomic_t child_finished = 0;

static void child_handler(int sig) {
    (void)sig;
    child_finished = 1;
}

static void clean_background(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        fprintf(stderr, "[finished %ld]\n", (long)pid);
    child_finished = 0;
}

static int tokenize(const char *line, char *tokens[]) {
    int count = 0;
    const char *p = line;
    while (*p && count < MAX_TOKENS - 1) {
        if (*p == ' ' || *p == '\t' || *p == '\n') { p++; continue; }
        const char *start = p;
        if (*p == '|' || *p == '<' || *p == '>' || *p == '&') p++;
        else while (*p && *p != ' ' && *p != '\t' && *p != '\n' &&
                    *p != '|' && *p != '<' && *p != '>' && *p != '&') p++;
        tokens[count] = strndup(start, (size_t)(p - start));
        if (!tokens[count]) { perror("strndup"); exit(1); }
        count++;
    }
    tokens[count] = NULL;
    return count;
}

static int parse(char *tokens[], int count, Command cmds[], int *background) {
    int n = 1, a = 0;
    memset(cmds, 0, sizeof(Command) * MAX_CMDS);
    *background = 0;
    for (int i = 0; i < count; i++) {
        if (!strcmp(tokens[i], "|")) {
            if (a == 0 || n == MAX_CMDS) return -1;
            cmds[n - 1].args[a] = NULL;
            n++; a = 0;
        } else if (!strcmp(tokens[i], "<") || !strcmp(tokens[i], ">")) {
            int input = !strcmp(tokens[i], "<");
            if (++i >= count) return -1;
            if (input) cmds[n - 1].input = tokens[i];
            else cmds[n - 1].output = tokens[i];
        } else if (!strcmp(tokens[i], "&")) {
            if (i != count - 1) return -1;
            *background = 1;
        } else {
            if (a >= MAX_ARGS - 1) return -1;
            cmds[n - 1].args[a++] = tokens[i];
        }
    }
    if (a == 0) return -1;
    cmds[n - 1].args[a] = NULL;
    return n;
}

static void run_commands(Command cmds[], int count, int background) {
    int previous_read = -1;
    pid_t pids[MAX_CMDS];
    for (int i = 0; i < count; i++) {
        int pipefd[2] = {-1, -1};
        if (i < count - 1 && pipe(pipefd) < 0) { perror("pipe"); return; }
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); return; }
        if (pid == 0) {
            signal(SIGINT, SIG_DFL);
            signal(SIGCHLD, SIG_DFL);
            if (previous_read != -1) dup2(previous_read, STDIN_FILENO);
            if (pipefd[1] != -1) dup2(pipefd[1], STDOUT_FILENO);
            if (previous_read != -1) close(previous_read);
            if (pipefd[0] != -1) close(pipefd[0]);
            if (pipefd[1] != -1) close(pipefd[1]);
            if (cmds[i].input) {
                int fd = open(cmds[i].input, O_RDONLY);
                if (fd < 0) { perror("input"); _exit(1); }
                dup2(fd, STDIN_FILENO); close(fd);
            }
            if (cmds[i].output) {
                int fd = open(cmds[i].output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) { perror("output"); _exit(1); }
                dup2(fd, STDOUT_FILENO); close(fd);
            }
            execvp(cmds[i].args[0], cmds[i].args);
            perror("execvp");
            _exit(127);
        }
        pids[i] = pid;
        if (previous_read != -1) close(previous_read);
        if (pipefd[1] != -1) close(pipefd[1]);
        previous_read = pipefd[0];
    }
    if (previous_read != -1) close(previous_read);
    if (background) {
        printf("[background %ld]\n", (long)pids[count - 1]);
    } else {
        for (int i = 0; i < count; i++) {
            while (waitpid(pids[i], NULL, 0) < 0 && errno == EINTR) {}
        }
    }
}

int main(void) {
    struct sigaction sa = {0};
    sa.sa_handler = child_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
    signal(SIGINT, SIG_IGN);
    char *line = NULL;
    size_t capacity = 0;
    while (1) {
        if (child_finished) clean_background();
        if (isatty(STDIN_FILENO)) { printf("mini> "); fflush(stdout); }
        if (getline(&line, &capacity, stdin) < 0) {
            if (errno == EINTR) { clearerr(stdin); continue; }
            break;
        }
        char *tokens[MAX_TOKENS];
        int ntok = tokenize(line, tokens);
        if (!ntok) continue;
        Command cmds[MAX_CMDS];
        int background;
        int count = parse(tokens, ntok, cmds, &background);
        if (count < 0) { fprintf(stderr, "Invalid command\n"); goto free_tokens; }
        if (count == 1 && !strcmp(cmds[0].args[0], "exit")) {
            for (int i = 0; i < ntok; i++) free(tokens[i]);
            break;
        }
        if (count == 1 && !strcmp(cmds[0].args[0], "cd")) {
            const char *path = cmds[0].args[1] ? cmds[0].args[1] : getenv("HOME");
            if (!path || chdir(path) < 0) perror("cd");
            goto free_tokens;
        }
        run_commands(cmds, count, background);
free_tokens:
        for (int i = 0; i < ntok; i++) free(tokens[i]);
    }
    free(line);
    clean_background();
    return 0;
}
