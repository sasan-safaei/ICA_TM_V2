# Step 1 - Deploy to another device (ROS)

If you want to run the ROS parts of this project on another device, copy the following files and folders from the repository to the target machine:

- ROS packages (source): `rosWS/src` — contains `qt_gui_node_pkg`, `tm_node_pkg`, `my_msgs_pkg` and other packages.
- Runtime config: `tempTM_WorkSpace/config.cfg` (the app reads this at runtime).
- Build/run helper scripts: `rosWS/reCompile.sh`, `rosWS/runTM.sh`, `rosWS/setGPIO.sh`.
- Systemd service (optional): `Documents/my-ICA-TM.service` if you use the systemd service.
- Native binaries/sources (optional): `TM_V2` if you need the native app or its Makefile.

Quick copy + build example (adjust paths and ROS distro):

```bash
# on target device
# 1) install ROS2 and build tools (example; replace <distro>)
sudo apt update
sudo apt install -y python3-colcon-common-extensions build-essential

# 2) copy files (example using rsync from source machine)
rsync -avz user@source:/path/to/repo/rosWS/src ~/ros_ws/src
rsync -avz user@source:/path/to/repo/tempTM_WorkSpace/config.cfg ~/ros_ws/config.cfg
rsync -avz user@source:/path/to/repo/rosWS/reCompile.sh ~/ros_ws/reCompile.sh

# 3) build
cd ~/ros_ws
colcon build

# 4) source and run
source install/setup.bash
# run the node (use actual package/executable names)
ros2 run tm_node_pkg <executable>
ros2 run qt_gui_node_pkg <executable>
```

Notes:
- Copy `rosWS/src` (the source), not `install/` or `build/`, so the code can be rebuilt on the new device.
- Place `config.cfg` where the node expects it (app uses `myArg.workSpace + "config.cfg"`); either keep the same workspace path or pass the path as an argument.
- To use the systemd service, copy `Documents/my-ICA-TM.service` to `/etc/systemd/system/` and enable/start:

```bash
sudo cp Documents/my-ICA-TM.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now my-ICA-TM.service
```

If you want, I can produce a single `rsync` command that copies only the minimal files, or I can inspect the ROS packages and list the exact executables to run.
