#!/usr/bin/env bash

### ========= USER CONFIG ========= ###
SESSION_NAME="ros_session"
ROS_SETUP="/opt/ros/humble/setup.bash" 
FLEXBOT_WS="/home/flexbot/flexbot_capstone/"
### ================================= ###

if tmux has-session -t $SESSION_NAME 2>/dev/null; then
    echo "⚠️  Session exists. Killing old session..."
    tmux kill-session -t $SESSION_NAME
fi

echo "🚀 Starting tmux session..."

tmux new-session -d -s $SESSION_NAME

# Enable mouse support
tmux set-option -t $SESSION_NAME mouse on

# Create 4-pane layout
# First split into left/right
tmux split-window -h -t $SESSION_NAME:0
# Split left into top/bottom
tmux split-window -v -t $SESSION_NAME:0.0
# Split right into top/bottom
tmux split-window -v -t $SESSION_NAME:0.1
# Force equal sizing
tmux select-layout -t $SESSION_NAME tiled

# Pane indexes:
# 0 = top-left
# 1 = top-right
# 2 = bottom-left
# 3 = bottom-right

# Load ROS environment in each pane
for pane in 0 1 2 3; do
  tmux send-keys -t $SESSION_NAME:0.$pane "source $ROS_SETUP && cd $FLEXBOT_WS && source install/setup.bash" C-m
done

### ====== ROS COMMANDS ====== ###
tmux send-keys -t $SESSION_NAME:0.0 "ros2 launch flex_bot_bringup bringup_full.launch.py" C-m
sleep 5
tmux send-keys -t $SESSION_NAME:0.1 "ros2 run teleop_twist_keyboard teleop_twist_keyboard" C-m
sleep 5
tmux send-keys -t $SESSION_NAME:0.2 "ros2 topic list" C-m
sleep 5
tmux send-keys -t $SESSION_NAME:0.3 " " C-m

### ======================================== ###
# Attach to session
tmux attach -t $SESSION_NAME
