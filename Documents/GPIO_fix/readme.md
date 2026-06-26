# GPIO sysfs Permission Fix (ICA)

This folder contains everything needed to fix sysfs GPIO permission issues on a target device.

## Files in This Folder

- `ICA_fix-gpio-perms.sh`: Fixes owner/group/mode for one exported gpioN node.
- `ICA_99-gpio-sysfs-perms.rules`: udev rule to run the fix script when gpioN appears.
- `ICA_gpio-sysfs-fix.service`: systemd oneshot service for base sysfs GPIO paths.
- `ICA_install_gpio_fix.sh`: Installs all files into system locations.

## Problem This Solves

Typical failures include:

- `ls /sys/class/gpio` returns permission denied for non-root user.
- Writing to `/sys/class/gpio/gpioN/direction` or `value` fails as non-root user.

Root cause is usually one or both:

- `/sys/class/gpio` is not `root:gpio` with execute bit on directory.
- Newly exported `gpioN` files are created as `root:root 0644`.

## Prerequisites

1. User is in `gpio` group.
2. `gpio` group exists on the target system.
3. Device still supports sysfs GPIO (legacy interface).

Check:

```bash
id ica
grep '^gpio:' /etc/group
```

If needed:

```bash
sudo groupadd -f gpio
sudo usermod -aG gpio ica
# logout/login after this change
```

## Install From Scratch

Run on the target device from this folder:

```bash
cd ~/git_sasan/ICA_TM_V2/Documents/GPIO_problem
sudo sh ./ICA_install_gpio_fix.sh
```

This does:

1. Installs script to `/usr/local/sbin/ICA_fix-gpio-perms.sh`.
2. Installs udev rule to `/etc/udev/rules.d/ICA_99-gpio-sysfs-perms.rules`.
3. Installs service to `/etc/systemd/system/ICA-gpio-sysfs-fix.service`.
4. Reloads systemd and udev.
5. Enables and starts the service.

## Manual Install (Alternative)

```bash
sudo install -o root -g root -m 0755 ICA_fix-gpio-perms.sh /usr/local/sbin/ICA_fix-gpio-perms.sh
sudo install -o root -g root -m 0644 ICA_99-gpio-sysfs-perms.rules /etc/udev/rules.d/ICA_99-gpio-sysfs-perms.rules
sudo install -o root -g root -m 0644 ICA_gpio-sysfs-fix.service /etc/systemd/system/ICA-gpio-sysfs-fix.service

sudo systemctl daemon-reload
sudo systemctl enable --now ICA-gpio-sysfs-fix.service
sudo udevadm control --reload-rules
sudo udevadm trigger --subsystem-match=gpio
```

## Validate

1. Base path permissions:

```bash
ls -ld /sys/class/gpio
stat -c '%A %U:%G %n' /sys/class/gpio/export /sys/class/gpio/unexport
```

Expected:

- `/sys/class/gpio` similar to `drwxrwx--- root gpio`
- `export` and `unexport` similar to `-rw-rw---- root gpio`

2. Export a line and verify file permissions:

```bash
echo 26 | sudo tee /sys/class/gpio/export
stat -c '%A %U:%G %n' /sys/class/gpio/gpio26/direction /sys/class/gpio/gpio26/value /sys/class/gpio/gpio26/edge
```

Expected: each file is `-rw-rw---- root gpio`.

3. Non-root write test (as user in gpio group):

```bash
echo out > /sys/class/gpio/gpio26/direction
echo 1 > /sys/class/gpio/gpio26/value
echo 0 > /sys/class/gpio/gpio26/value
```

## If It Still Fails

1. Check service status and logs:

```bash
systemctl status ICA-gpio-sysfs-fix.service
journalctl -u ICA-gpio-sysfs-fix.service -b
```

2. Check udev activity:

```bash
journalctl -b -u systemd-udevd | grep -i gpio
journalctl -t ICA_gpio_perms -b
```

3. Re-apply one GPIO manually:

```bash
sudo /usr/local/sbin/ICA_fix-gpio-perms.sh gpio26
```

4. Re-check group membership and relogin:

```bash
id ica
```

## Notes

- sysfs GPIO is legacy on newer kernels. Prefer `libgpiod` and `/dev/gpiochip*` for long-term reliability.
- sysfs entries are recreated on reboot, so boot-time/system event fixes are required.
