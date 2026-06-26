#!/bin/sh
set -eu

SRC_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"

echo "Installing ICA GPIO permission fix files..."

install -o root -g root -m 0755 "$SRC_DIR/ICA_fix-gpio-perms.sh" /usr/local/sbin/ICA_fix-gpio-perms.sh
install -o root -g root -m 0644 "$SRC_DIR/ICA_99-gpio-sysfs-perms.rules" /etc/udev/rules.d/ICA_99-gpio-sysfs-perms.rules
install -o root -g root -m 0644 "$SRC_DIR/ICA_gpio-sysfs-fix.service" /etc/systemd/system/ICA-gpio-sysfs-fix.service

systemctl daemon-reload
systemctl enable --now ICA-gpio-sysfs-fix.service
udevadm control --reload-rules
udevadm trigger --subsystem-match=gpio

echo "Done. Validate with:"
echo "  ls -ld /sys/class/gpio"
echo "  stat -c '%A %U:%G %n' /sys/class/gpio/export /sys/class/gpio/unexport"
