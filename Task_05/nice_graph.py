
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("nice_results.csv")

x = range(len(df))
width = 0.35

plt.bar([i - width / 2 for i in x],
        df["actual_percent"], width, label="Actual")

plt.bar([i + width / 2 for i in x],
        df["expected_percent"], width, label="Expected")

plt.xticks(list(x), df["nice"])
plt.xlabel("Nice Value")
plt.ylabel("CPU Share (%)")
plt.title("Actual vs Expected CPU Share")
plt.legend()
plt.grid(axis="y")

plt.savefig("nice_comparison.png")
plt.show()
