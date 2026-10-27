
import matplotlib.pyplot as plt
from mlfq import bursts, mlfq_finish, srtf_finish

processes = range(1, len(bursts) + 1)

plt.plot(processes, mlfq_finish, marker="o", label="MLFQ")
plt.plot(processes, srtf_finish, marker="o", label="SRTF")

plt.xlabel("Process Number")
plt.ylabel("Completion Time")
plt.title("MLFQ vs SRTF")
plt.xticks(list(processes))
plt.legend()
plt.grid(True)

plt.savefig("mlfq_vs_srtf.png")
plt.show()
