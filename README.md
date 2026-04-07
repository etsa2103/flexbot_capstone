# Low Level CPU Setup for Flex Bot

This repository provides a step-by-step guide for setting up the Low-Level CPU connected to the Berkshire Grey FlexBot. This system talks directly to the robots sensors and actuators. It also communicates with the High-Level CPU to share sensor data and execute low level motor commands.

This guide will cover the following topics:

- **Flashing Custom OS** — This section covers the steps of flashing our custom Linux Operating System on a micro sd card and installing it on the Flexbots low level computer.
- **Network Setup** — Setting IPs so the low and high level CPUs can communicate.
- **Setting up system services** — Creating system services that auto launch scripts for arming motors and setting up communication between the low and high level CPUs. (This section might be redundent if custom OS is flashed)
- **File system** — Summary of each file and its specific use in the flexbot system

> **Note:** This repo assumes it has been installed at the base of the root directory on the low level cpu.

---

## Attribution

This codebase is adapted from work originally developed by Kartik Virmani.
It has been modified to support the VLP-16 LiDAR and simplified for clarity and instructional use.

---

## Flashing Custom OS

To be completed.

## Network Setup

The custom OS should be setup to auto connect to flex_bot_wifi6

First I suggest [setting a static IP](https://www.freecodecamp.org/news/setting-a-static-ip-in-ubuntu-linux-ip-address-tutorial/) on this wifi network so you can remotely access the low level computer for debugging.

We used `192.168.129.200` and our username is `flexbot` so it can be accessed using `ssh flexbot@192.168.129.200`

For the High Level CPU to communicate with the Low Level CPU each needs to assign an IP Address to the Ethernet interface connecting the two. On our system we used the following IPs:

**High Level IP:** `192.168.0.20`

**Low Level IP:** `192.168.0.2`

### Setting Static IP

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

> **Note:** Sometimes other network management systems can override the network settings we changed with nmcli. I suggest resetting the cpu after making changes to see if they remain on reboot. If not, look into other network presets such at netplan. Also make use ip and make sure this ethernet connection is the only interface on your computer with this IP

### High Level CPU

High level control is handled by a NAME_OF_NUC mounted on the flexbot and connected to the low level CPU via ethernet. See the `main` branch of this repository for that code and documentation.

In order for the two CPUs to communicate you must make sure they know each other's IP address. Make sure the UDP IP/port settings in `UDP/imu_udp_tx.cpp` , `UDP/motor_controller.cpp` and `UDP/udp_cmd_client`match what is configured on the high level cpu.

---

## Setting up system services

To be completed.

---

## File System

To be completed.

---
