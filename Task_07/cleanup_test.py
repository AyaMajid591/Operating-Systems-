
import subprocess
import signal
import time
import os

os.makedirs("evidence", exist_ok=True)

tests = [
    ("queue", "./ipc_queue"),
    ("shared", "./ipc_shared")
]

def resources(path, prefix):
    return {
        name for name in os.listdir(path)
        if name.startswith(prefix)
    }

for name, program in tests:
    path = "/dev/mqueue" if name == "queue" else "/dev/shm"
    prefix = "ipc_task7_" if name == "queue" else "ipc_shm_"

    before = resources(path, prefix)

    process = subprocess.Popen(
        [program, "64"],
        start_new_session=True
    )

    time.sleep(0.3)
    process.send_signal(signal.SIGTERM)

    try:
        process.wait(timeout=5)
        completed = True
    except subprocess.TimeoutExpired:
        os.killpg(process.pid, signal.SIGKILL)
        process.wait()
        completed = False

    time.sleep(0.2)

    after = resources(path, prefix)
    leftovers = after - before

    result = (
        f"Method: {name}\n"
        f"Stopped within 5 seconds: {completed}\n"
        f"Exit code: {process.returncode}\n"
        f"New resources left behind: {sorted(leftovers)}\n"
    )

    print(result)

    with open(f"evidence/cleanup_{name}.txt", "w") as f:
        f.write(result)
