# Operating Systems — Task 1 (Level C)

## 1. Ubuntu and SSH

I used Ubuntu 26.04.1 LTS for this assignment.

I installed and enabled the SSH server and connected to Ubuntu from my Mac using SSH key authentication.

My current Ubuntu installation has a graphical interface, so it is not yet a minimal Ubuntu Server installation.

## 2. systemd vs OpenRC

| Feature | systemd (Ubuntu) | OpenRC (Alpine) |
|---|---|---|
| Service definition | Unit files | Init scripts |
| Start service | systemctl start | rc-service start |
| Stop service | systemctl stop | rc-service stop |
| Check status | systemctl status | rc-service status |
| Enable at startup | systemctl enable | rc-update add |
| View logs | journalctl -u | Service log files or syslog |
| Boot organization | Targets | Runlevels |

systemd manages services using unit files, while OpenRC uses init scripts and runlevels.

## 3. Python Time Logger

I created a Python script that writes the current date and time to `time_log.txt` every 10 seconds.

I also created a systemd unit file called `time-logger.service`.

The service starts automatically when Ubuntu boots.

I checked the service using:

- `systemctl status time-logger.service`
- `journalctl -u time-logger.service`

The service worked successfully, and the time log was updated every 10 seconds.

## 4. Boot Performance Analysis

I measured boot performance using:

- `systemd-analyze`
- `systemd-analyze blame`
- `systemd-analyze critical-chain`

The boot critical-chain output showed that the graphical target was reached approximately 4.619 seconds after systemd started.

The `snapd.seeded.service` took approximately 1.765 seconds to initialize.

Snap services could be investigated as possible optimization candidates if they are not needed. However, they should not be disabled without checking dependencies.

Because this installation uses Ubuntu Desktop, graphical services could also be removed in a future minimal server installation.

## 5. Project Files

- `time_logger.py` — Python time logger
- `time-logger.service` — systemd unit file
- `evidence/service_status.txt` — Service status
- `evidence/journal.txt` — systemd journal messages
- `evidence/boot_time.txt` — Boot time
- `evidence/boot_blame.txt` — Service startup times
- `evidence/critical_chain.txt` — Boot dependency chain

## Conclusion

In this assignment, I learned how to connect to Ubuntu using SSH, manage background services using systemd, and analyze system boot performance.

I successfully created a Python service that writes the time every 10 seconds and configured it to start automatically.

I also compared systemd with OpenRC and identified possible ways to improve boot performance.
