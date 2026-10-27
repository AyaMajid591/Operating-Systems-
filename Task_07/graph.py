
import os
import re
import csv
import matplotlib.pyplot as plt

methods = ["pipe", "socket", "queue", "shared"]
sizes = [64, 4096, 1048576]
results = []

for method in methods:
    for size in sizes:
        path = f"evidence/{method}_{size}.txt"

        if not os.path.exists(path):
            print("Missing:", path)
            continue

        with open(path) as f:
            text = f.read()

        throughput = re.search(r"Throughput:\s*([\d.]+)", text)
        latency = re.search(
            r"Average time per (?:logical )?message:\s*([\d.]+)",
            text
        )

        if not throughput or not latency:
            print("Incomplete:", path)
            continue

        results.append([
            method, size,
            float(throughput.group(1)),
            float(latency.group(1))
        ])

with open("ipc_results.csv", "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow([
        "method", "size", "throughput", "average_time_us"
    ])
    writer.writerows(results)

for column, ylabel, filename in [
    (2, "Throughput (MiB/s)", "throughput.png"),
    (3, "Average Time per Message (us)", "message_time.png")
]:
    plt.figure(figsize=(9, 5))

    for method in methods:
        values = [r for r in results if r[0] == method]

        if values:
            values.sort(key=lambda r: r[1])
            plt.plot(
                [r[1] for r in values],
                [r[column] for r in values],
                marker="o",
                label=method
            )

    plt.xscale("log", base=2)
    plt.xticks(sizes, ["64 B", "4 KiB", "1 MiB"])
    plt.xlabel("Logical Message Size")
    plt.ylabel(ylabel)
    plt.title(ylabel + " by IPC Method")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(filename, dpi=150)
    plt.show()

print("Completed measurements:", len(results), "/ 12")
