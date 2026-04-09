# Low Level CPU Setup for Flex Bot

This repository provides a step-by-step guide for setting up the Low-Level CPU connected to the Berkshire Grey FlexBot. This system talks directly to the robots sensors and actuators. It also communicates with the High-Level CPU to share sensor data and execute low level motor commands.

This guide will cover the following topics:

- **Flashing Custom Operating System** — This section covers the steps of flashing our custom Linux Operating System on a micro sd card and installing it on the Flexbots low level computer.
- **Network Setup** — Setting IPs so the low and high level CPUs can communicate.
- **Setting up system services** — Creating system services that auto launch scripts for arming motors and setting up communication between the low and high level CPUs. (This section might be redundent if custom OS is flashed)
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

The custom OS should be setup to auto connect to wifi with SSID `bg-flexbot-wifi6` and password `rac@bg1922` which you can mimic with your own wifi hotspot. I suggest [setting a static IP](https://www.freecodecamp.org/news/setting-a-static-ip-in-ubuntu-linux-ip-address-tutorial/) on this wifi network so you can remotely access the low level computer for debugging. We used `192.168.129.200` and our username is `flexbot` so it can be accessed using `ssh flexbot@192.168.129.200`

For the High Level CPU to communicate with the Low Level CPU each needs to assign an IP Address to the Ethernet interface connecting the two. On our system we used the following IPs:

**High Level IP:** `192.168.10.20`

**Low Level IP:** `192.168.10.2`

#### Setting Static IP

1. Unplug ethernet connection, run `ip a`, reconnect ethernet, run `ip a` again, and locate name of the new ethernet connection. (Ours was *enp1s0)*
2. Run `nmcli con show`
3. Use `nmcli connection show ` to determine which wired connection is to the higher level cpu ethernet interface. (In our case "Wired connection 1" was connected to enp1s0)
4. Run the following command to rename the connection and set its IP. Make sure you replace <connection_name> with the connection you found in step 3.

```bash
nmcli con modify "<connection_name>" connection.id "cpu-link"
nmcli con modify "cpu-link" \
  connection.autoconnect yes \
  ipv4.method manual \
  ipv4.addresses 192.168.10.2/24 \
  ipv4.gateway "192.168.10.20" \

nmcli connection down "cpu-link"
nmcli connection up "cpu-link"
```

> **Note:** Sometimes other network management systems can override the network settings we changed with nmcli. I suggest resetting the cpu after making changes to see if they remain on reboot. If not, look into other network presets such at netplan. Also run `ip a` and make sure this ethernet connection is the only interface on your computer with this IP

#### High Level CPU

High level control is handled by an Sapphire BP-FP6-SN mounted on the flexbot and connected to the low level CPU via ethernet. See the `main` branch of this repository for that code and documentation.

In order for the two CPUs to communicate you must make sure they know each other's IP address. Make sure the UDP IP/port settings in `UDP/imu_udp_tx.cpp` , `UDP/motor_controller.cpp` and `UDP/udp_cmd_client` match what is configured on the high level cpu.

---

## Setting up system services

If you flashed the right OS you should already have a system service that runs the `init_bot.sh` script. This script unlocks the motors and sets up communication between the high and low level computer. To confirm it is there, run `sudo ls /etc/systemd/system/` and look for `init_bot.service`. If you don't see it, you need to create this system service yourself following the steps below.

1. Run sudo chmod +x ~/flexbot_capstone/init_bot.sh
2. sudo nano /etc/systemd/system/init_bot.service
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
