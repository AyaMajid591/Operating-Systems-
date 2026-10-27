
import csv
import matplotlib.pyplot as plt

rows = []

with open("lock_results.csv") as file:
    rows = list(csv.DictReader(file))

fig, axes = plt.subplots(1, 2, figsize=(11, 4))

for ax, section in zip(axes, ["Short", "Long"]):
    for lock in ["Spinlock", "Mutex"]:
        times = []
        for n in [2, 4, 16]:
            values = [
                float(row["time"])
                for row in rows
                if row["lock"] == lock
                and row["section"] == section
                and int(row["threads"]) == n
            ]
            times.append(sum(values) / len(values))

        ax.plot([2, 4, 16], times, marker="o", label=lock)

    ax.set_title(section + " Critical Section")
    ax.set_xlabel("Threads")
    ax.set_ylabel("Time (seconds)")
    ax.set_xticks([2, 4, 16])
    ax.legend()
    ax.grid(True)

plt.tight_layout()
plt.savefig("lock_comparison.png")
plt.show()
