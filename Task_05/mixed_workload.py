
from collections import deque
import matplotlib.pyplot as plt

bursts = [3, 20, 2, 25, 4, 18, 2]
arrivals = [0, 0, 2, 3, 5, 7, 9]
quantums = [2, 4, 8]
boost_interval = 20

def simulate(method):
    remaining = bursts.copy()
    finish = [0] * len(bursts)
    queues = [deque(), deque(), deque()]
    timeline = []
    time = 0

    while any(x > 0 for x in remaining):
        for i in range(len(bursts)):
            if arrivals[i] == time:
                queues[0].append(i)

        if method == "MLFQ" and time > 0 and time % boost_interval == 0:
            active = []
            for queue in queues:
                active.extend(queue)
                queue.clear()
            queues[0].extend(active)

        if method == "SRTF":
            ready = [
                i for i in range(len(bursts))
                if arrivals[i] <= time and remaining[i] > 0
            ]
            if not ready:
                timeline.append(-1)
                time += 1
                continue

            pid = min(ready, key=lambda i: remaining[i])
            remaining[pid] -= 1
            timeline.append(pid)
            time += 1

            if remaining[pid] == 0:
                finish[pid] = time
        else:
            if not any(queues):
                timeline.append(-1)
                time += 1
                continue

            level = next(i for i in range(3) if queues[i])
            pid = queues[level].popleft()
            used = 0

            while used < quantums[level] and remaining[pid] > 0:
                remaining[pid] -= 1
                timeline.append(pid)
                time += 1
                used += 1

                for i in range(len(bursts)):
                    if arrivals[i] == time:
                        queues[0].append(i)

                if time % boost_interval == 0:
                    break

            if remaining[pid] == 0:
                finish[pid] = time
            elif time % boost_interval == 0:
                queues[level].append(pid)
            else:
                queues[min(level + 1, 2)].append(pid)

    turnaround = [
        finish[i] - arrivals[i] for i in range(len(bursts))
    ]

    return sum(turnaround) / len(turnaround), timeline

mlfq_time, mlfq_timeline = simulate("MLFQ")
srtf_time, srtf_timeline = simulate("SRTF")

print("MLFQ average turnaround:", round(mlfq_time, 2))
print("SRTF average turnaround:", round(srtf_time, 2))

plt.figure(figsize=(10, 4))
plt.plot(mlfq_timeline, label="MLFQ", alpha=0.8)
plt.plot(srtf_timeline, label="SRTF", alpha=0.6)
plt.xlabel("Time")
plt.ylabel("Process ID")
plt.title("MLFQ vs SRTF on Mixed Workload")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("mixed_workload.png")
plt.show()
