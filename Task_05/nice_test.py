
import os
import csv
import time
import subprocess

nice_values = [0, 5, 10, 15]
weights = [1024, 335, 110, 36]
core = min(os.sched_getaffinity(0))
processes = []

def cpu_time(pid):
    with open(f"/proc/{pid}/schedstat") as file:
        return int(file.read().split()[0])

try:
    for value in nice_values:
        p = subprocess.Popen([
            "taskset", "-c", str(core),
            "nice", "-n", str(value),
            "./cpu_worker"
        ])
        processes.append(p)

    time.sleep(2)
    before = [cpu_time(p.pid) for p in processes]

    time.sleep(10)
    after = [cpu_time(p.pid) for p in processes]

finally:
    for p in processes:
        p.terminate()

    for p in processes:
        p.wait()

times = [(b - a) / 1e9 for a, b in zip(before, after)]
total = sum(times)
weight_total = sum(weights)

with open("nice_results.csv", "w", newline="") as file:
    writer = csv.writer(file)
    writer.writerow(["nice", "cpu_seconds", "actual_percent", "expected_percent"])

    for i in range(4):
        actual = times[i] / total * 100
        expected = weights[i] / weight_total * 100

        writer.writerow([
            nice_values[i],
            round(times[i], 3),
            round(actual, 2),
            round(expected, 2)
        ])

        print("Nice:", nice_values[i])
        print("CPU time:", round(times[i], 3))
        print("Actual share:", round(actual, 2), "%")
        print("Expected share:", round(expected, 2), "%")
        print()
