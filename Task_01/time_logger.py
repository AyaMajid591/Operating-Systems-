
from datetime import datetime
import time

while True:
    with open("time_log.txt", "a") as file:
        file.write(str(datetime.now()) + "\n")

    print("Time recorded", flush=True)
    time.sleep(10)
