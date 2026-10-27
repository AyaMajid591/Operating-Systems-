
import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv("sum_results.csv")

plt.plot(data["threads"], data["speedup"], marker="o")
plt.xlabel("Number of Threads")
plt.ylabel("Speedup")
plt.title("Parallel Sum Speedup")
plt.grid(True)
plt.savefig("speedup.png")
plt.show()

data = pd.read_csv("creation_results.csv")

plt.bar(data["type"], data["time"])
plt.ylabel("Time (seconds)")
plt.title("Thread vs Process Creation")
plt.savefig("creation.png")
plt.show()

data = pd.read_csv("sharing_results.csv")

plt.bar(data["type"], data["time"])
plt.ylabel("Time (seconds)")
plt.title("False Sharing Comparison")
plt.savefig("false_sharing.png")
plt.show()
