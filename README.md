# High Level CPU Setup for Flex Bot

This repository provides a step-by-step guide for setting up the High-Level CPU connected to the Berkshire Grey FlexBot. This system receives sensor data over ROS 2 topics and runs high-level functionality such as teleoperation, localization, mapping, and autonomous exploration.

This guide will cover the following topics:

- **ROS Setup** — fill in
- **Network Setup** — fill in
- **Teleoperation** — sending UDP commands to the IMX7 and receiving state feedback
- **Localization** — fill in
- **Lidar/Mapping** — fill in
- **Autonomous modes** — fill in
- **Visualization** — fill in

> **Note:** This repo has been tested and runs on a (FILL IN CPU NAME) running (FILL IN OS NAME). Though it should work on any system that can run ROS 2 (Humble or Jazzy). This repo also assumes it has been installed at the base of the root directory on the high level cpu.

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

## Network Setup

First I suggest [setting a static IP](https://www.freecodecamp.org/news/setting-a-static-ip-in-ubuntu-linux-ip-address-tutorial/) on your local wifi so you can remotely access the high level computer.

We used `192.168.129.200` and our username is `flexbot` so it can be accessed using `ssh flexbot@192.168.129.200`

For the High Level CPU to communicate with the Low Level CPU each needs to assign an IP Address to the Ethernet interface connecting the two. On our system we used the following IPs:

**High Level IP:** `192.168.10.20`

**Low Level IP:** `192.168.10.2`

### Setting Static IP

1. Unplug ethernet connection, run `ip a`, reconnect ethernet, run `ip a` again, and locate name of the new ethernet connection. (Ours was *enp1s0)*
2. Run `nmcli con show`
3. Use `nmcli connection show ` to determine which wired connection is to the higher level cpu ethernet interface. (In our case "Wired connection 1" was connected to enp1s0)
4. Run the following command to rename the connection and set its IP. Make sure you replace <connection_name> with the connection you found in step 3.

```bash
nmcli con modify "<connection_name>" connection.id "cpu-link"
nmcli con modify "cpu-link" 
  connection.autoconnect yes 
  ipv4.method manual 
  ipv4.addresses 192.168.10.20/24 
  ipv4.gateway ""

nmcli connection down "cpu-link"
nmcli connection up "cpu-link"
```

> **Note:** Sometimes other network management systems can override the network settings we changed with nmcli. I suggest resetting the cpu after making changes to see if they remain on reboot. If not, look into other network presets such at netplan. Also make use ip and make sure this ethernet connection is the only interface on your computer with this IP

### Low Level CPU

The embedded firmware runs on the flexbot's IMX7 computer. This handles low-level motor control, sensor readings, and UDP packet formatting. See the `low_level_cpu` branch of this repository for that code and documentation

Make sure the UDP IP/port settings in `flex_bot_teleop/config/flex_bot_udp.yaml` match what is configured on the IMX7 side.

---

## Teleoperation

To be completed.

---

## Localization

To be completed.

---

## Lidar/Mapping

### Lidar Setup

1. Plug VLP16 ethernet in and run `ip a` to find the ethernet interface name. (Ours was enx7cc2c642f053)
2. Run `sudo tcpdump -i <ethernet_interface_name> udp port 2368` replacing <ethernet_interface_name> with the name you found in step 1.
3. Look at the output to ensure you are recieving data and to determine the ip address of the lidar.
4. Run `sudo nano /opt/ros/humble/share/velodyne_driver/config/VLP16-velodyne_driver_node-params.yaml` and change ip to match lidar ip you found in step 2.
5. Assign ip to ethernet interface with lidar using the following command.

```bash
nmcli con add type ethernet ifname fill_ethernet_interface_name con-name lidar-net
ipv4.addresses 192.168.0.100/24 ipv4.method manual ipv4.never-default yes
```

> **Note:** Use `ros2 launch velodyne velodyne-all-nodes-VLP16-launch.py` to test if you have set eveything up correctly

---

## Autonomous modes

To be completed.

---

## Visualization

To be completed.

---
