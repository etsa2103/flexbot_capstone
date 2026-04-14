#!/bin/bash
#python3 ~/flexbot_capstone/MOTORS/init_left_motor.py
#python3 ~/flexbot_capstone/MOTORS/init_right_motor.py
#g++ -std=c++17 -Wall -Wextra ~/flexbot_capstone/UDP/imu_udp_tx.cpp -o main
#g++ -std=c++17 -Wall -Wextra -pthread ~/flexbot_capstone/UDP/motor_controller.cpp -o main
bash ~/flexbot_capstone/UDP/init_motors.sh
bash ~/flexbot_capstone/UDP/start_comm.sh
