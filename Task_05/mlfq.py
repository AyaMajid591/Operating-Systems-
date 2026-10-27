
from collections import deque

bursts = [2, 4, 6, 18, 24]
quantums = [2, 4, 8]
boost_interval = 20


def mlfq(bursts):
    remaining = bursts.copy()
    finish = [0] * len(bursts)
    queues = [deque(range(len(bursts))), deque(), deque()]
    timeline = []
    t = 0

    while any(x > 0 for x in remaining):
        if t > 0 and t % boost_interval == 0:
            active = []
            for queue in queues:
                active.extend(queue)
                queue.clear()
            queues[0].extend(active)

        level = next(i for i in range(3) if queues[i])
        pid = queues[level].popleft()
        used = 0

        while used < quantums[level] and remaining[pid] > 0:
            remaining[pid] -= 1
            used += 1
            timeline.append(pid)
            t += 1

            if t % boost_interval == 0:
                break

        if remaining[pid] == 0:
            finish[pid] = t
        elif t % boost_interval == 0:
            queues[level].append(pid)
        else:
            queues[min(level + 1, 2)].append(pid)

    return finish, timeline


def srtf(bursts):
    remaining = bursts.copy()
    finish = [0] * len(bursts)
    timeline = []

    for t in range(sum(bursts)):
        pid = min(
            (i for i in range(len(bursts)) if remaining[i] > 0),
            key=lambda i: remaining[i]
        )

        remaining[pid] -= 1
        timeline.append(pid)

        if remaining[pid] == 0:
            finish[pid] = t + 1

    return finish, timeline


mlfq_finish, mlfq_timeline = mlfq(bursts)
srtf_finish, srtf_timeline = srtf(bursts)

print("MLFQ finish times:", mlfq_finish)
print("SRTF finish times:", srtf_finish)

print("MLFQ average turnaround:", sum(mlfq_finish) / len(bursts))
print("SRTF average turnaround:", sum(srtf_finish) / len(bursts))
