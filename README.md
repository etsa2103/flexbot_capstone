# High Level CPU Setup for Flex Bot

This repository provides a step-by-step guide for setting up the High-Level CPU connected to the Berkshire Grey FlexBot. This system receives sensor data over ROS 2 topics and runs high-level functionality such as teleoperation, localization, mapping, and autonomous exploration.

This guide will cover the following topics:

- **ROS Setup** — Installing ROS2 and package dependencies for this repo
- **Network Setup** — Setting up the ethernet connection between the high and low level CPUs
- **Teleoperation** — Modifying config files and running the teleop node
- **Localization** — fill in (odom pkg)
- **Lidar/Mapping** — Setting up the lidar and running slam toolbox
- **Autonomous modes** — Running autonomy node to have the flexbot explore and map the environment
- **Bringup** — How to launch all necesary ros nodes for flexbot operation
- **Visualization** — Setting up foxglove layout and running foxglove bridge

> **Note:** This repo has been tested and runs on a **Sapphire BP-FP6-SN** running **Ubuntu 22.04.5 LTS**. Though it should work on any system that can run ROS 2 (Humble or Jazzy). This repo also assumes it has been installed at the base of the root directory on the high level cpu.

---

## Attribution

This codebase is adapted from work originally developed by Kartik Virmani.
It has been modified to support the VLP-16 LiDAR and simplified for clarity and instructional use.

---

## ROS Setup

Follow this guide to [install ROS2](https://docs.ros.org/en/humble/Installation.html)

**Option 1 Use rosdep for dependancies:**

```bash
sudo rosdep init
rosdep update
cd ~/flexbot_capstone
rosdep install --from-paths src --ignore-src -r -y
```

**Option 2 Manually install dependancies:**

1. Run `sudo apt update`
2. Install foxglove bridge for visualization: `sudo apt install ros-$ROS_DISTRO-foxglove-bridge`
3. Install robot-localization package: `sudo apt install ros-$ROS_DISTRO-robot-localization`
4. Install velodyne ros package: `sudo apt install ros-$ROS_DISTRO-velodyne`
5. install keyboard teleop package: `sudo apt install ros-$ROS_DISTRO-teleop-twist-keyboard`

With all dependancies installed you can build the workspace using the following commands.

```bash
cd ~/flexbot_capstone
colcon build --symlink-install
source install/setup.bash
```

Next add ros domain id to bashrc using the following commands.

```bash
nano ~/.bashrc
export ROS_DOMAIN_ID=200
```

> **Note:** This is so your ros nodes do not talk to other ros instances on the network

---

## Network Setup

First I suggest [setting a static IP](https://www.freecodecamp.org/news/setting-a-static-ip-in-ubuntu-linux-ip-address-tutorial/) on your local wifi so you can remotely access the high level computer. We used `192.168.129.200` and our username is `flexbot` so it can be accessed using `ssh flexbot@192.168.129.200`

For the High Level CPU to communicate with the Low Level CPU each needs to assign an IP Address to the Ethernet interface connecting the two. On our system we used the following IPs:

**High Level IP:** `192.168.10.20`

**Low Level IP:** `192.168.10.2`

### Setting Static IP

1. Run `nmcli con show` to view all network connections and use `nmcli connection delete <connection_name>` to delete any connections you don't care about (Usually all but the wifi connection)
2. Find the name of the ethernet interface to the low level cpu. You can do this by unplugging the device, running `ip a`, reconnecting the device, and running `ip a` again. (For us the low level cpu was *`enp1s0`*)
3. Run the following command to create a connection between the cpus and to set its IP. Make sure you replace `<interface_name>` with the interface name you found in step 2 and replace `<high_level_ip>` with your chosen IP.

```bash
# Create connection
nmcli con add type ethernet ifname <interface_name> con-name cpu-link \
    ipv4.method manual \
    ipv4.addresses <high_level_ip>/24

# Refresh the connection
nmcli con up cpu-link
```

> **Note:** Sometimes other network management systems can override the network settings we changed with nmcli. I suggest resetting the cpu after making changes to see if they remain on reboot. If not, look into other network presets such at netplan. Also run `ip a` and make sure this ethernet connection is the only interface on your computer with this IP.

### Low Level CPU

The embedded firmware runs on the flexbot's IMX7 computer. This handles low-level motor control, sensor readings, and UDP packet formatting. See the `low_level_cpu` branch of this repository for that code and documentation

Make sure the UDP IP/port settings in `flex_bot_teleop/config/flex_bot_udp.yaml` match what is configured on the IMX7 side.

---

## Teleoperation

IN TERMINAL 1:

1. `cd flexbot_capstone`
2. `source /opt/ros/{ROS_DISTRO}/setup.bash`
3. `run colcon build`
4. `source install/setup.bash`
5. `ros2 launch flex_bot_teleop flex_bot.launch.py`

IN TERMINAL 2:

1. `cd flexbot_capstone`
2. `source /opt/ros/{ROS_DISTRO}/setup.bash`
3. `ros2 run teleop_twist_keyboard teleop_twist_keyboard`

if it says you do not have the teleop-twist-keyboard available:

1. `sudo apt update`
2. `sudo apt install ros-{ROS_DISTRO}-teleop-twist-keyboard`

we have a limit on the speed to be 10 in the config file as the teleop-twist-keyboard goes up to max speed almost immediately.

---

## Localization

To be completed.

---

## Lidar/Mapping

### Lidar Setup

1. If you followed the steps in "Network Setup" on the low level cpu branch you should have the IP address of the Lidar. (Ours was 192.168.2.201)
2. Run `sudo nano /opt/ros/humble/share/velodyne_driver/config/VLP16-velodyne_driver_node-params.yaml` and change ip to match the lidar ip
3. Use `ros2 launch velodyne velodyne-all-nodes-VLP16-launch.py` to test if you have set eveything up correctly. A scan topic should appear if you run `ros2 topic list`

### Running slam toolbox

1. modify flex_bot_bringup/config/slam_config
2. run `ros2 launch flex_bot_bringup slam_async.launch.py`

---

## Autonomous modes

To be completed.

---

## Bringup

To be completed.

---

## Visualization

To be completed.

---
