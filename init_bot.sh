python3 ~/kr_flexbot/MOTORS/init_left_motor.py
python3 ~/kr_flexbot/MOTORS/init_right_motor.py
g++ -std=c++17 -Wall -Wextra ~/kr_flexbot/UDP/imu_udp_tx.cpp -o main
g++ -std=c++17 -Wall -Wextra -pthread ~/kr_flexbot/UDP/motor_controller.cpp -o main
python3 /Home 
