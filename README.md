# Low Level CPU Setup for Flex Bot

This repository provides a step-by-step guide for setting up the Low-Level CPU connected to the Berkshire Grey FlexBot. This system talks directly to the robots sensors and actuators. It also communicates with the High-Level CPU to share sensor data and execute low level motor commands.

This guide will cover the following topics:

- **Flashing Custom Operating System** — This section covers the steps of flashing our custom Linux Operating System on a micro sd card and installing it on the Flexbots low level computer.
- **Network Setup** — Setting IPs so the low and high level CPUs can communicate.
- **Setting up system services** — Creating system services that auto launch scripts for arming motors and setting up communication between the low and high level CPUs.
- **File system** — Summary of each file and its specific use in the flexbot system

> **Note:** This repo assumes it has been installed at the base of the root directory on the low level cpu.

---

## Attribution

This codebase is adapted from work originally developed by Kartik Virmani.
It has been modified to support the VLP-16 LiDAR and simplified for clarity and instructional use.

---

## Flashing Custom Operating System

This platform is designed to be used as a mobile base for research at UPENN. If your FlexBot already has the custom OS installed, you can skip this section. To verify, run `cat /etc/os-release` and confirm that it includes: `NAME="UPENN_Flexbot_OS"`

#### Booting Behavior

The FlexBot boots directly from the microSD card installed on the low-level CPU. To switch operating systems, simply replace the microSD card with one containing the desired OS image.

#### Accessing the microSD Card

To access the microSD card:

1. Power off the robot completely.
2. Flip the robot upside down.
3. Remove the charging pad bracket.
4. Locate the microSD card inserted in the APB (Auxiliary Processing Board).

> **Note:** For a detailed overview of FlexBot hardware components, refer to the [hardware documentation](https://miro.com/app/board/uXjVJ2xI5w8=/?share_link_id=938604789528).

#### Obtaining the Custom OS

If you do not already have a microSD card with the custom OS:

* Contact another lab or team that has access to the UPENN_FlexBot_OS image.
* Alternatively, request the image file directly if available.

#### Copying the OS to a New microSD Card

Once you have access to a working microSD card or image, follow the appropriate [flashing instructions](https://emteria.com/kb/clone-sd-cards-linux) to copy the OS onto your own microSD card.

---

## Network Setup

The custom OS should be setup to auto connect to wifi with SSID `bg-flexbot-wifi6` and password `rac@bg1922` which you can mimic with your own wifi hotspot. I suggest [setting a static IP](https://www.freecodecamp.org/news/setting-a-static-ip-in-ubuntu-linux-ip-address-tutorial/) on this wifi network so you can remotely access the low level computer for debugging.

For the High Level CPU to communicate with the Low Level CPU and the lidar, each needs to assign an IP Address to the Ethernet interface connecting them. You can chose your own IPs as long as you are consistant when setting up the high level CPU. On our system we used the following IPs:

**High Level IP:** `192.168.10.20`

**Low Level IP:** `192.168.10.2`

**Lidar IP:** `192.168.2.201`

> **Note:** You can find the lidar IP by doing the following:
>
> 1. Plug VLP16 ethernet in and run `ip a` to find the ethernet interface name. (Ours was eth0)
> 2. Run `sudo tcpdump -i <ethernet_interface_name> udp port 2368` replacing <ethernet_interface_name> with the name you found in step 1. ( you might need to run `sudo apt install tcpdump -y` to install tcpdump)
> 3. Look at the output to ensure you are recieving data and to determine the ip address of the lidar.

#### Setting Static IP

1. Run `nmcli con show` to view all network connections and use `nmcli connection delete <connection_name>` to delete any connections you don't care about (Usually all but the wifi connection)
2. Find the name of the ethernet interface to the lidar and to the high level cpu. You can do this by unplugging the device, running `ip a`, reconnecting the device, and running `ip a` again. (For us the lidar was *`eth0`* and the high level cpu was *`eth1`*)
3. Run the following command to bridge both ethernet connections and assign thier IPs. Make sure you replace <low_level_cpu_ip>, <lidar_ip>,<lidar_connection_name>, <cpu_connection_name>,  and <high_level_cpu_ip>

```bash
# 1. Recreate the bridge with both IP addresses
nmcli con add type bridge ifname br0 con-name br0 \
    connection.autoconnect yes \
    ipv4.method manual \
    ipv4.gateway 192.168.10.20 \
    ipv4.dns "8.8.8.8,1.1.1.1" \
    ipv4.addresses "192.168.10.2/24, 192.168.2.201/24"

# 2. Add the physical ports to the bridge
nmcli con add type ethernet ifname eth0 con-name br-port-lidar master br0
nmcli con add type ethernet ifname eth1 con-name br-port-cpu master br0

# 3. Bring the bridge up
nmcli con up br0
```

> **Note:** Sometimes other network management systems can override the network settings we changed with nmcli. I suggest resetting the cpu after making changes to see if they remain on reboot. If not, look into other network presets such at netplan. Also run `ip a` and make sure each the same IP is not used for multiple interfaces.

#### High Level CPU

High level control is handled by an Sapphire BP-FP6-SN mounted on the flexbot and connected to the low level CPU via ethernet. See the `master` branch of this repository for that code and documentation.

In order for the two CPUs to communicate, you must make sure they know each other's IP address. Make sure the UDP IP/port settings in `UDP/imu_udp_tx.cpp` , `UDP/motor_controller.cpp` and `UDP/udp_cmd_client` match what is configured on the high level cpu.

---

## Setting up system services

If you flashed the right OS you should already have a system service that runs the `init_bot.sh` script. This script unlocks the motors and sets up communication between the high and low level computer. To confirm it is there, run `sudo ls /etc/systemd/system/` and look for `init_bot.service`. If you don't see it, you need to create this system service yourself following the steps below.

1. Run `sudo chmod +x ~/flexbot_capstone/init_bot.sh`
2. Run `sudo nano /etc/systemd/system/init_bot.service`
3. Copy the following text into the file

```yml
[Unit]
Description=Run bot initialization script
After=network.target

[Service]
Type=simple
User=root
ExecStart=/root/flexbot_capstone/init_bot.sh

[Install]
WantedBy=multi-user.target
```

4. Run the following commands to apply the changes

```bash
sudo systemctl daemon-reexec
sudo systemctl daemon-reload
sudo systemctl enable minit_bot.service
sudo systemctl start init_bot.service
```

---

## File System

### BAT

To be completed.

### IMU

To be completed.

### LCD

To be completed.

### LED

To be completed.

### MOTORS

To be completed.

### PGV

To be completed.

### UDP

To be completed.

---
sudo ip link set can1 down
sudo ip link set can1 up type can bitrate 1000000
sudo ip link set can1 down
sudo ip link set can1 up type can bitrate 125000
