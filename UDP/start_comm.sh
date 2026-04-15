#!/bin/bash
set -e
cd ~/flexbot_capstone/UDP

# Start motor controller
./motor_controller > motor_controller.log 2>&1 &
MC_PID=$!
echo "motor_controller PID=$MC_PID"

# Start battery UDP TX
./battery_udp_tx > battery_udp_tx.log 2>&1 &
BAT_PID=$!
echo "battery_udp_tx PID=$BAT_PID"

# Start IMU UDP TX
./imu_udp_tx > imu_udp_tx.log 2>&1 &
IMU_PID=$!
echo "imu_udp_tx PID=$IMU_PID"

# Start CMD UDP RX for LED + LCD commands
./udp_cmd_rx > udp_cmd_rx.log 2>&1 &
CMD_PID=$!
echo "udp_cmd_rx PID=$CMD_PID"

# Kill both on Ctrl+C / termination
trap "echo 'Stopping...'; kill $MC_PID $BAT_PID $IMU_PID $CMD_PID ; exit 0" INT TERM

wait
