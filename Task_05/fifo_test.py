
import os
import time
import subprocess

core = min(os.sched_getaffinity(0))

def cpu_time(pid):
    with open(f"/proc/{pid}/schedstat") as f:
        return int(f.read().split()[0]) / 1e9

def test(use_fifo):
    normal = subprocess.Popen(
        ["taskset", "-c", str(core), "./cpu_worker"]
    )

    competitor = None

    try:
        time.sleep(0.5)
        start = cpu_time(normal.pid)

        if use_fifo:
            competitor = subprocess.Popen([
                "sudo", "-n", "chrt", "-f", "1",
                "taskset", "-c", str(core),
                "timeout", "-k", "1s", "2s",
                "./cpu_worker"
            ])

            time.sleep(2)
            competitor.wait(timeout=5)
        else:
            time.sleep(2)

        end = cpu_time(normal.pid)

        return end - start

    finally:
        if competitor and competitor.poll() is None:
            competitor.terminate()

        normal.terminate()
        normal.wait()

baseline = test(False)
fifo = test(True)

print("Normal CPU time:", round(baseline, 3), "seconds")
print("With SCHED_FIFO:", round(fifo, 3), "seconds")
print("Difference:", round(baseline - fifo, 3), "seconds")

with open("fifo_results.txt", "w") as f:
    f.write(f"Normal: {baseline:.3f} seconds\n")
    f.write(f"With FIFO: {fifo:.3f} seconds\n")
