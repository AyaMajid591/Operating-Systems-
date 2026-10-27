# Operating Systems Assignments

This repository contains my Operating Systems assignments completed using Ubuntu Linux.

The assignments focus on Linux commands, C programming, process management, synchronization, CPU scheduling, and inter-process communication.

## Completed Tasks

### Task 1 — Ubuntu Setup and systemd
Configured SSH access using SSH keys, compared systemd with OpenRC, created a Python time logger running as a systemd service, and analyzed Ubuntu boot performance.

### Task 2 — File Copying and System Calls
Implemented file copying in C using read/write, fread/fwrite, direct syscall, and sendfile. Compared different buffer sizes, measured performance, and analyzed system calls using strace.

### Task 3 — Mini Shell
Created a simple Linux shell in C using fork, execvp, and waitpid. Added command execution, pipes, input/output redirection, background processes, and built-in commands.

### Task 4 — Threads and Processes
Used pthreads to calculate the sum of 100 million elements. Compared thread and process creation times, investigated false sharing, and generated performance graphs.

### Task 5 — CPU Scheduling
Implemented MLFQ scheduling and compared it with SRTF. Tested CPU scheduling using different nice values and investigated real-time scheduling with SCHED_FIFO.

### Task 6 — Thread Synchronization
Implemented a spinlock and compared it with pthread mutexes. Studied the readers–writers problem and investigated race conditions using Helgrind and ThreadSanitizer.

### Task 7 — Inter-Process Communication
Implemented four IPC methods: pipes, Unix domain sockets, POSIX message queues, and shared memory. Compared throughput and latency, analyzed system calls, tested resource cleanup, and explored splice.

### Task 8 — Deadlocks and Dining Philosophers
Implemented four versions of the Dining Philosophers problem. Studied deadlocks, livelocks, lock ordering, semaphores, and deadlock detection using Helgrind and Linux documentation.

## Technologies Used

- Ubuntu Linux
- C
- Python
- GCC
- pthreads
- systemd
- SSH
- strace
- Valgrind / Helgrind
- ThreadSanitizer
- perf
- Matplotlib

## Conclusion

These assignments helped me understand how operating systems manage processes, threads, memory, scheduling, synchronization, and communication between processes.

I also gained practical experience using Linux terminal commands, writing C programs, measuring performance, and debugging system-level problems.
