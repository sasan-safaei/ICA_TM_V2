#!/bin/sh
# Fix permissions for a single exported sysfs GPIO, e.g. gpio26.

GPIO="$1"
[ -n "$GPIO" ] || exit 0

BASE="/sys/class/gpio/$GPIO"

# Wait briefly for sysfs entries to appear to avoid udev timing races.
i=0
while [ $i -lt 20 ]; do
  [ -e "$BASE/direction" ] && [ -e "$BASE/value" ] && break
  i=$((i + 1))
  sleep 0.05
done

[ -d "$BASE" ] || exit 0

chgrp gpio "$BASE" 2>/dev/null || true
chmod 0770 "$BASE" 2>/dev/null || true

for f in direction value edge active_low; do
  if [ -e "$BASE/$f" ]; then
    chgrp gpio "$BASE/$f" 2>/dev/null || true
    chmod 0660 "$BASE/$f" 2>/dev/null || true
  fi
done

logger -t ICA_gpio_perms "fixed $GPIO"
exit 0
