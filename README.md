# (add credit to kartik and link his repo)

# High Level CPU Setup for Flex Bot

This repo takes you step by step through the setup of High Level CPU connected to the Berkshire Grey Flexbot. This computer is recieves sensor data over ROS2 topics and runs high level operations such as teleoperation, localization, mapping, and autonomous exploration.

This guide will cover the following topics:

- **ROS Setup** — fill in
- **Network Setup** — fill in
- **Teleoperation** — sending UDP commands to the IMX7 and receiving state feedback
- **Localization** — fill in
- **Lidar/Mapping** — fill in
- **Autonomous modes** — fill in
- **Visualization** — fill in

> **Note:** This repo has been tested and runs on a (FILL IN CPU NAME) running (FILL IN OS NAME). Though it should work on any system that can run ROS 2 (Humble or Jazzy)

---

## ROS Setup

Follow this guide to [install ROS2](https://docs.ros.org/en/humble/Installation.html)

Run `sudo apt update`

Install foxglove bridge for visualization: `sudo apt install ros-$ROS_DISTRO-foxglove-bridge`

Install robot-localization package: `sudo apt install ros-$ROS_DISTRO-robot-localization`

*or use rosdep install*

With all dependancies installed you can build the workspace

```bash
cd ~/flexbot_capstone
colcon build --symlink-install
source install/setup.bash
```

*Add ros domain id to bashrc*

## Network Setup

For the High Level CPU to communicate with the Low Level CPU each needs to assign an IP Address to the Ethernet interface connecting the two. On our system we used the following IPs:

**High Level IP** = 192.168.0.20

**Low Level IP** = 192.168.0.2

### Method 1

add nmcli connection method

### Method 2

1. Unplug ethernet connection, run `ip a`, reconnect ethernet, run `ip a` again, and locate name of the new ethernet connection. Ours was ***enp1s0.***
2. Go to netplan folder `cd /etc/netplan/` and look for config yaml. Ours was `01-network-manager-all.yaml`
3. Update config file to look like this making sure you use your ethernet connection name and High Level IP:

```yaml
network:
  version: 2
  renderer: NetworkManager
  ethernets:
    enp1s0:
      dhcp4: no
      addresses:
        - 192.168.0.20/24
      routes:
        - to: default
          via: 192.168.0.1
      nameservers:
        addresses:
          - 8.8.8.8
          - 8.8.4.4
```

4. Run `sudo netplan apply` to apply changes

> **Note:** Make sure this ethernet connection is the only interface on your computer with this IP

### Low Level CPU

The embedded firmware runs on the flexbot's IMX7 computer. This handles low-level motor control, sensor readings, and UDP packet formatting. See the `low_level_cpu` branch of this repository for that code and documentation

Make sure the UDP IP/port settings in `flex_bot_teleop/config/flex_bot_udp.yaml` match what is configured on the IMX7 side.

---

## Teleoperation

## Localization

## Lidar/Mapping

## Autonomous modes

## Visualization

---

# Kartiks stuff i have not upacked yet

## Prerequisites

- ROS 2 (Humble or Jazzy)
- `robot_localization` — for EKF state estimation
- `tf2_ros`
- `rviz2` (optional, for visualization)

Network configuration (adjust to your setup):

- IMX7 IP: Always check `192.168.0.2`
- Companion Computer IP (UDP receiver): Set your computers wired connection ip. `192.168.0.20`

---

## Package Overview

### `flex_bot_teleop`

Handles bidirectional UDP communication with the IMX7.

- `udp_bridge_node` — bridges ROS 2 topics to/from UDP packets exchanged with the IMX7
- `teleop_node` — converts joystick or keyboard input into velocity commands

Configuration is in `config/flex_bot_udp.yaml` and `config/teleop.yaml`. Adjust UDP ports and IP addresses there to match your network.

Launch teleop:

```bash
ros2 launch flex_bot_teleop flex_bot.launch.py
```

---

### `flex_bot_odometry`

Computes wheel odometry from encoder feedback received over UDP from the IMX7.

Publishes `odom → base_link` as an odometry source (later fused by the EKF).

Configuration is in `config/wheel_odom.yaml`.

Launch wheel odometry:

```bash
ros2 launch flex_bot_odometry wheel_odom.launch.py
```

To visualize the odometry path:

```bash
ros2 run flex_bot_odometry odom_to_path.py
```

---

### State Estimation (`flex_bot_bringup` — EKF only)

The EKF (via `robot_localization`) fuses wheel odometry and IMU data to produce a smooth, stable `odom → base_link` transform.

Configuration is in `flex_bot_bringup/config/ekf_imu.yaml`. Key settings:

- `two_d_mode: true` — recommended for ground robots
- Fuse yaw and yaw-rate from IMU; avoid fusing raw linear acceleration until a reliable odometry source is confirmed, as it will cause drift

Launch state estimation:

```bash
ros2 launch flex_bot_bringup bringup_state_estimation.launch.py
```

Or run the EKF node directly:

```bash
ros2 run robot_localization ekf_node --ros-args --params-file \
  ~/flex_bot/src/flex_bot_bringup/config/ekf_imu.yaml
```

---

## Static TF Setup

The robot requires static transforms declaring sensor mounting positions relative to `base_link`. Publish these before starting any other nodes.

**base_link → IMU:**

```bash
ros2 run tf2_ros static_transform_publisher \
  0 0 0 0 0 0 base_link xsens_imu
```

> Replace the zeros with the real mounting offset once measured.

Verify the full TF tree at any time:

```bash
ros2 run tf2_tools view_frames
```

---

## TF Chain

The expected transform chain during normal operation:

```
odom → base_link        # published by robot_localization EKF
         └→ xsens_imu  # static TF
```

> `map → odom` is not published in this minimal setup. It would require a SLAM or localization node, which is out of scope here.

---

## Recommended Launch Order

1. Static TFs
2. Teleop (UDP bridge + command input)
3. Wheel Odometry
4. State Estimation (EKF)

```bash
# Terminal 1 — Static TFs
ros2 launch flex_bot_bringup static_tfs.launch.py

# Terminal 2 — Teleop + UDP bridge
ros2 launch flex_bot_teleop flex_bot.launch.py

# Terminal 3 — Wheel odometry
ros2 launch flex_bot_odometry wheel_odom.launch.py

# Terminal 4 — EKF state estimation
ros2 launch flex_bot_bringup bringup_state_estimation.launch.py
```
