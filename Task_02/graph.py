
import csv
import matplotlib.pyplot as plt

results = {"read": [], "fread": []}

with open("results.csv") as file:
    for row in csv.DictReader(file):
        results[row["method"]].append(
            (int(row["buffer_size"]), float(row["time"]))
        )

for method, values in results.items():
    values.sort()
    sizes = [x[0] for x in values]
    times = [x[1] for x in values]

    plt.plot(sizes, times, marker="o", label=method)

plt.xscale("log", base=2)
plt.xlabel("Buffer Size (bytes)")
plt.ylabel("Time (seconds)")
plt.title("File Copy Performance - 100 MiB")
plt.legend()
plt.grid(True)
plt.savefig("performance.png", dpi=150)
plt.show()
