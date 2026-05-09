# FlexBot Reverse Engineering and ROS Integration

## Overview

Berkshire Grey dropped off 30 differential drive industrial robots known as FlexBots to the University of Pennsylvania. The goal of this capstone project was to reverse engineer the FlexBot and repurpose it to be used for future research projects at Penn. I started by disassembling the robot, documenting all its components, and mapping out all the electrical connections. Next, I connected to the onboard computer and evaluated the robot's custom control software. Once I gained control of the motors and was able to read the data from the sensors, I removed the top rotary stage and mounted a LIDAR and external computer. Finally, I integrated the whole system into ROS to enable teleoperation, odometry, and SLAM.
<p align="center">
  <img src="imgs/angledView.jpeg" width="300">
  <img src="imgs/sideView.jpeg" width="300">
</p>
---

## Hardware

After disassembling the robot I separated the components into 3 subsections: The Mobile Base, Scissor Lift Stage, and Rotary Stage. Below is a quick description of each stage, but please refer to this [Miro Board](https://miro.com/app/board/uXjVJ2xI5w8=/) for the full hardware documentation.

### Mobile Base

The mobile base is the main working platform of the robot and contains most of the important systems. Two drive motors allow the robot to move around, while the onboard electronics control power distribution, computation, and sensing. The base contains components such as the Motor Control Boards (MCBs), Power Distribution Board (PDB), sensors, LEDs, WiFi antennas, and the main compute board (APB). These systems work together to help the robot operate autonomously, process sensor data, and communicate with other systems.

### Scissor Lift Stage

The scissor lift stage sits on top of the Mobile Base and gives the robot vertical movement by raising and lowering the top platform. It uses a scissor lift mechanism powered by the lift motor to extend upward when needed. It also contains a wire management channel to route cables from the mobile base to the rotary stage without getting pinched.

### Rotary Stage

The rotary stage was originally included to allow the top section of the robot to rotate independently from the base. It used a rotary motor and rotating platform mechanism to provide smooth turning motion for sensors or attachments mounted on top. This stage is present on the original flexbots donated to Penn, but for this project has been removed and power for the rotary motor has been diverted to the external computer and LIDAR.

---

## Electrical Connections

The diagram below shows the overall electrical layout of the robot and how the different PCBs connect to each other. Rather than labeling every individual wire within each cable, the diagram focuses on the main connectors and subsystem relationships to keep the system architecture easier to understand.
![circuit-diagram](imgs/flexbot-circuit-diagram.png)
**Figure 1: Circuit Diagram of Flexbot’s Internal Electronics**

---

## Software Analysis

To gain control over the FlexBot's internal systems, I began by establishing direct serial communication with the onboard CPU. This connection allowed me to monitor the boot sequence and understand the base operating system environment. I found that the CPU automatically tried connecting to a WiFi network called `bg-flexbot-wifi6`, so I spoofed the expected network to grant me full SSH access to the internal filesystem.

With access secured, gaining control of the motors required reverse engineering the internal CAN network which I did with the help of Kartik Virmani of MOD lab. By using the robot's built-in maintenance program to send known movement commands, we monitored the resulting CAN traffic. By matching the data packets to specific inputs, we mapped out the CAN IDs for motor velocities and sensor data, ultimately allowing us to write custom control scripts that bypassed the original software entirely.

All code developed for the low level computer onboard the flexbot can be found in this branch of the project Github. It also contains a clear README with everything you should need to replicate this setup on your own.

---

## Modifications

I made several hardware modifications to repurpose the FlexBot for standard research applications. First, to overcome restrictions imposed by the proprietary safety board, I disconnected it and manually rerouted 24V to each of the Safety Torque Off (STO) pins on the MCBs. This 24 volts was taken from the PDB and routed through both Emergency Stop (E-stop) buttons to ensure the robot remained safe to operate.

Next, I worked with Jaimes Romero to remove the original rotary stage mechanism and replace it with a custom-fabricated top plate, providing a stable and flat mounting surface for external hardware. We also designed and 3D-printed custom mounts for a VLP-16 LIDAR and an external high-level computer. We tapped into the robot's Power Distribution Board (PDB) to route the appropriate voltage to the new top plate. The full hardware breakdown is summarized in the figure below and the modified hardware is documented in the Hardware Miro Board.
![modified-hardware-breakdown](imgs/modified-hardware-breakdown.png)
**Figure 2: Hardware Breakdown of Modified Flexbot**

Finally, the original system relied on Berkshire Grey charging docks, which we had no access to. Instead of creating our own charging docks, for safety, I ordered a charger so the batteries could be recharged.

---

## ROS Integration

To integrate the FlexBot into ROS 2, I established a dedicated Ethernet connection between the robot's low-level embedded computer and the new external high-level CPU. The low-level system processes raw hardware telemetry and continuously streams it as UDP packets to the high-level computer. There, custom ROS 2 nodes capture this UDP data and publish it to standard ROS topics. This architecture powers a teleoperation package that translates manual inputs into safe motor commands while simultaneously calculating precise wheel odometry. By combining this odometry with data from the integrated VLP-16 LiDAR, the system successfully runs a 2D SLAM pipeline to generate real-time maps of its environment. The full software breakdown is summarized in the figure below.
![software-breakdown](imgs/software-breakdown.png)
**Figure 3: Software Breakdown of Modified Flexbot**

Finally, all operational data, including the live map, sensor feeds, and hardware status, is streamed to a custom Foxglove Studio dashboard for comprehensive, real-time monitoring as shown below.
![foxglove](imgs/foxglove.png)
**Figure 4: Foxglove**

All code developed for the high level computer can be found in this branch of the project Github. It also contains a clear README with everything you should need to replicate this setup on your own.

---

## Side Notes and Next Steps

There are several remaining limitations and future improvements for the FlexBot platform. When charging the robot, it is important to ensure that the main power switch remains on, as the batteries will not charge otherwise. While the mobile base and LiDAR are fully operational, the scissor lift stage, LEDs, LCD display, PGV, and time of flight sensors are currently not fully integrated. The `low_level_cpu` branch has some scripts that attempt to access these parts of the hardware, but this control has not yet been exposed by a ROS node running on the high level cpu. Moving forward, the next major development goal is integrating a 3D SLAM pipeline to take advantage of the full capabilities of the mounted VLP-16 LiDAR. After reliable 3D mapping is achieved, future work will focus on implementing obstacle avoidance and autonomous exploration or mapping behaviors, allowing the FlexBot to operate with greater autonomy in research environments.

---

## Appendix

### Previous Documentation

- BG Flexbot manual  
  https://cornell.box.com/s/qmkmx8giolcgj92opy3ygfzgm6xuyje4

- Cornell Box  
  https://app.box.com/folder/348944125989?s=29tjvikns65o8l0exeabmyehw10j4b1x

### Penn Documentation

- My Github  
  https://github.com/etsa2103/flexbot_capstone

- Kartik’s Github  
  https://github.com/virmani11kartik/bg_bot

- Kumar Robotic’s Github  
  https://github.com/KumarRobotics/kr_flexbot

- Miro Hardware Documentation  
  https://miro.com/app/board/uXjVJ2xI5w8=/

- Circuit diagram  
  https://www.circuitlab.com/circuit/pdmw4kpy644g/flexbot-circuit-diagram/

### Communication Channels

- Slack  
  https://app.slack.com/client/T09S1APBSHY/D09S4T6TL04