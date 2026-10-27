
import subprocess
import time
import csv

methods = ["read", "fread"]
sizes = [1, 64, 4096, 65536]
results = []

for method in methods:
    for size in sizes:
        start = time.perf_counter()

        subprocess.run(
            ["./copy", method, str(size),
             "test_100mb.bin", "output.bin"],
            check=True,
            stdout=subprocess.DEVNULL
        )

        elapsed = time.perf_counter() - start

        subprocess.run(
            ["cmp", "test_100mb.bin", "output.bin"],
            check=True
        )

        results.append([method, size, elapsed])

        print(method, size, round(elapsed, 4), "seconds")

with open("results.csv", "w", newline="") as file:
    writer = csv.writer(file)
    writer.writerow(["method", "buffer_size", "time"])
    writer.writerows(results)

print("Results saved!")
