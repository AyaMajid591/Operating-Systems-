# Operating Systems — Task 3: Mini Shell (Level C)

## Overview

This project is a small Linux shell written in C. It reads commands, splits arguments, executes programs using `fork()` and `execvp()`, and uses `waitpid()` for foreground commands. It includes built-in `cd` and `exit`.

The shell also supports multi-stage pipelines (`|`), input (`<`) and output (`>`) redirection, and background commands (`&`). A `SIGCHLD` handler triggers background-child cleanup using `waitpid(..., WNOHANG)` in the main loop.

## Build (Ubuntu)

```bash
sudo apt update
sudo apt install gcc strace -y
gcc -std=c11 -O2 -Wall -Wextra minishell.c -o minishell
./minishell
```

## Commands to test

Run these at the `mini>` prompt:

```text
pwd
cd /tmp
pwd
cd
ls
ls | wc -l
printf hello | cat | wc -c
echo hello > output.txt
cat < output.txt
sleep 5 &
exit
```

Use `Ctrl+C` while a foreground command such as `sleep 30` is running to test signal behavior. The shell ignores SIGINT and children restore its default behavior. Terminal job control is intentionally limited.

## strace demonstration

Run outside the mini-shell in your Ubuntu terminal (the shell will receive the piped commands):

```bash
printf 'ls | wc -l\nexit\n' | strace -f -o trace.txt -e trace=process,pipe,pipe2,dup2,dup3,close,read,write ./minishell
```

Inspect the important calls:

```bash
grep -E 'clone|fork|vfork|execve|pipe|dup2|wait4' trace.txt | head -60
```

Expected behavior: the parent creates a pipe, forks two children, they connect stdout/stdin using `dup2()`, then `execvp()` starts `ls` and `wc` via `execve()`; the parent waits for foreground children using `waitpid()` (typically visible as `wait4` under strace).

## What to submit to GitHub

- `minishell.c`
- `README.md`
- `evidence/trace_excerpt.txt` generated using:

```bash
mkdir -p evidence
grep -E 'clone|fork|vfork|execve|pipe|dup2|wait4' trace.txt | head -60 > evidence/trace_excerpt.txt
```

- Screenshots showing `ls | wc -l`, redirection, background launch and Ctrl+C testing, if required by your instructor.

Do not upload a compiled executable, personal files, or unnecessary huge trace logs.

## Limitations

This is an educational mini-shell: no quotes, expansions, wildcards, `>>`, or complete `jobs`/`fg` process-group job control. The background handler sets a flag and reaping occurs in the main loop. `jobs` and `fg` are optional assignment extensions and are not implemented.

## Test results

Fill in after running on Ubuntu:

- Ubuntu version:
- GCC version:
- `ls | wc -l` output:
- Pipeline/redirection/background test results:
- strace observations:
