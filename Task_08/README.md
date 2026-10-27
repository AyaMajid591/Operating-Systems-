# Operating Systems — Task 8 (Level C)

## 1. Introduction

In this assignment, I studied deadlocks and thread synchronization using C and pthreads.

I implemented four versions of the Dining Philosophers problem and compared their behavior.

## 2. Dining Philosophers

I implemented four methods:

- Naive locking, which can cause deadlock.
- Ordered locking, which prevents circular waiting.
- Trylock with rollback, which can cause livelock.
- Waiter using a semaphore that allows only four philosophers to compete for forks.

I ran each experiment for 10 seconds.

### Results

| Method | Total meals | Maximum completed wait |
|---|---|---|
| Naive | [Result] | [Result] |
| Ordered | [Result] | [Result] |
| Trylock | [Result] | [Result] |
| Waiter | [Result] | [Result] |

The naive version demonstrates deadlock because philosophers wait for forks held by each other.

The trylock version demonstrates livelock under deliberately synchronized retries. The philosophers remain active but make no progress.

Ordered locking and the waiter solution allow the philosophers to continue eating.

## 3. Helgrind and ThreadSanitizer

I used Helgrind and ThreadSanitizer to investigate thread synchronization.

Helgrind reported lock-order warnings for the waiter solution. This happens because philosophers acquire forks in different orders, even though the waiter semaphore prevents all five philosophers from competing simultaneously.

The diagnostic findings are saved in the `evidence` folder.

ThreadSanitizer result: [Insert verified result or limitation.]

These tools can detect data races and some unsafe locking patterns, but they cannot detect every possible deadlock or livelock.

## 4. Linux Deadlock Detection

Linux provides tools such as lockdep and hung_task.

Lockdep checks locking dependencies inside the kernel and can detect possible circular lock dependencies.

The hung_task mechanism reports some tasks that remain blocked in an uninterruptible state for too long.

These mechanisms do not normally detect deadlocks between pthread mutexes in user-space programs.

The hung-task timeout on my system was 120 seconds.

References:

- https://docs.kernel.org/locking/lockdep-design.html
- https://docs.kernel.org/admin-guide/sysctl/kernel.html

## 5. Conclusion

In this assignment, I learned how deadlocks and livelocks can occur when several threads share resources.

I compared four synchronization methods and measured how many times each philosopher could eat.

I also learned how Helgrind, ThreadSanitizer, and Linux kernel debugging mechanisms help detect synchronization problems.
