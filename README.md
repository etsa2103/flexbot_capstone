# FlexBot Reverse Engineering and ROS Integration

## Overview

The goal of this capstone project was to repurpose the FlexBot so that it can serve as a general mobile platform for future robotics research and teaching projects at Penn. The FlexBot is a differential drive industrial robot donated to GRASP by Berkshire Grey.  

The repurposing process included documenting the robot’s hardware and electrical architecture, evaluating its custom control software, removing application-specific hardware, and integrating new systems including a LiDAR, external computer, and odometry pipeline. The completed system was integrated into ROS 2 to support teleoperation, odometry, and SLAM while also producing documentation for future users and development.

<p align="center">
  <img src="imgs/angledView.jpeg" width="300">
  <img src="imgs/sideView.jpeg" width="300">
</p>

---

## Hardware

The robot hardware can be separated into three main components: the Mobile Base, the Scissor Lift Stage, and the Rotary Stage. Below is a brief description of each subsystem, while the full hardware documentation can be found on this [Miro Board](https://miro.com/app/board/uXjVJ2xI5w8=/).

### Mobile Base

The mobile base is the primary working platform of the robot and contains most of the core systems. Two drive motors provide mobility, while the onboard electronics manage power distribution, sensing, and computation. The base includes components such as the Motor Control Boards (MCBs), Power Distribution Board (PDB), sensors, LEDs, WiFi antennas, and the Application Processor Board (APB). These systems work together to support autonomous operation, sensor processing, and communication with external systems.

### Scissor Lift Stage

The scissor lift stage sits above the mobile base and provides vertical movement through a motorized scissor lift mechanism. It also contains a cable routing channel that allows wiring to safely pass from the mobile base to the upper stage without becoming pinched during movement.

### Rotary Stage

The rotary stage was originally designed to allow the top section of the robot to rotate independently from the base using a dedicated rotary motor and platform assembly. This stage remains present on the original FlexBots donated to Penn, but for this project it was removed and the available power was redirected to support the external computer and LiDAR.

---

## Electrical Connections

The diagram below shows the overall electrical layout of the robot and the connections between the major PCBs and subsystems. Rather than labeling every individual wire within each cable, the diagram focuses on the primary connectors and subsystem relationships to provide a clearer system-level overview.

<p align="center">
  <img src="imgs/flexbot-circuit-diagram.png" width="1200">
  <br>
  <font size="5"><b>Figure 1: Circuit Diagram of FlexBot Internal Electronics</b></font>
</p>

---

## Software Analysis

To gain control of the FlexBot’s internal systems, I first established direct serial communication with the onboard CPU. This allowed me to monitor the boot sequence and better understand the robot’s operating system environment. The CPU automatically attempted to connect to a WiFi network called `bg-flexbot-wifi6`, so I recreated the expected network environment in order to gain SSH access to the internal filesystem.

Reverse engineering the robot’s CAN network was necessary to gain control of the motors. With the help of Kartik Virmani from MOD Lab, CAN traffic was monitored while the robot’s built-in maintenance software executed known movement commands. By correlating CAN packets with specific robot actions, we identified the CAN IDs responsible for motor velocity commands and sensor data, ultimately allowing custom control scripts to bypass the original software stack.

All code developed for the low-level onboard computer can be found in [this branch](https://github.com/etsa2103/flexbot_capstone/tree/low_level_cpu) of the project repository. That branch also includes setup instructions and documentation for replicating the process.

---

## Modifications

Several hardware modifications were made to adapt the FlexBot for research applications and external hardware integration.

To bypass restrictions imposed by the proprietary safety board, the board was disconnected and 24V power was manually rerouted to the Safety Torque Off (STO) pins on the Motor Control Boards (MCBs). This 24V supply was sourced from the Power Distribution Board (PDB) and routed through both Emergency Stop (E-stop) buttons to preserve a functional hardware safety mechanism during operation.

In collaboration with Jaime Romero, the original rotary stage was removed and replaced with a custom top plate that provides a stable mounting surface for additional hardware. Custom 3D-printed mounts were also designed for the VLP-16 LiDAR, external high-level computer, and power distribution hardware. The CAD files for these mounts can be found [here](CAD). Power for the new hardware was sourced directly from the robot’s PDB and routed to the modified top assembly.

The resulting hardware architecture is shown below, while additional documentation can be found in the modified hardware section of the [Miro Board](https://miro.com/app/board/uXjVJ2xI5w8=/).

<p align="center">
  <img src="imgs/modified-hardware-breakdown.png" width="600">
  <br>
  <font size="5"><b>Figure 2: Hardware Breakdown of Modified FlexBot</b></font>
</p>

Because the original system relied on proprietary Berkshire Grey charging docks that were unavailable, an external charger was used to safely recharge the batteries.

---

## ROS Integration

To integrate the FlexBot into ROS 2, a dedicated Ethernet connection was established between the robot’s low-level embedded computer and the new external high-level computer. The low-level system processes raw hardware telemetry and continuously streams this data as UDP packets to the high-level computer. Custom ROS 2 nodes running on the high-level system capture the UDP data and publish it to standard ROS topics.

This architecture supports teleoperation, wheel odometry, and 2D SLAM using the integrated VLP-16 LiDAR. The overall software structure is summarized in the figure below.

<p align="center">
  <img src="imgs/software-breakdown.png" width="600">
  <br>
  <font size="5"><b>Figure 3: Software Breakdown of Modified FlexBot</b></font>
</p>

Operational data, including maps, sensor feeds, and hardware telemetry, is visualized through a custom Foxglove Studio dashboard shown below.

<p align="center">
  <img src="imgs/foxglove.png" width="1200">
  <br>
  <font size="5"><b>Figure 4: Foxglove Visualizer</b></font>
</p>

All code developed for the high-level computer can be found in [this branch](https://github.com/etsa2103/flexbot_capstone/tree/high_level_cpu) of the repository. That branch also contains setup instructions and documentation for replicating the system.

---

## Side Notes and Next Steps

There are several remaining limitations and opportunities for future improvements to the FlexBot platform. When charging the robot, the main power switch must remain on or the batteries will not charge properly.

While the mobile base and LiDAR systems are fully operational, the scissor lift stage, LEDs, LCD display, PGV, and time-of-flight sensors are not yet fully integrated into ROS. The `low_level_cpu` branch contains several scripts that attempt to access these systems, but they are not currently exposed through ROS nodes running on the high-level computer.

Future development goals include integrating a 3D SLAM pipeline to better utilize the VLP-16 LiDAR, followed by obstacle avoidance and autonomous exploration or mapping behaviors for more advanced autonomous operation. Additional documentation collected throughout the project can be found in [this folder](additional_docs) of the repository.

---

## Acknowledgment

**Advisors:** I would like to thank Fernando Cladera and Dr. M. Ani Hsieh for their consistent advice and support throughout this project.

**Technical Contributors:** I would like to recognize Jaime Romero for his CAD designs and debugging assistance throughout the project. I would also like to recognize Kartik Virmani for his contributions to the reverse engineering and software development work included in this repository.

**Institutional Support:** Special thanks to Berkshire Grey for donating 30 FlexBot platforms to GRASP for research and educational use.

**Cornell University:** Thanks to Chris Bauer for providing documentation and insights from previous work on the robot platform.

## Appendix

- Kartik’s Github  
  https://github.com/virmani11kartik/bg_bot

- Kumar Robotics Github  
  https://github.com/KumarRobotics/kr_flexbot

- Miro Hardware Documentation  
  https://miro.com/app/board/uXjVJ2xI5w8=/

- Circuit diagram  
  https://www.circuitlab.com/circuit/pdmw4kpy644g/flexbot-circuit-diagram/

- Slack  
  https://app.slack.com/client/T09S1APBSHY/D09S4T6TL04
