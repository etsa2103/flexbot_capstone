from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess
from launch.substitutions import LaunchConfiguration, PythonExpression


def generate_launch_description():
    velodyne_cmd = [
        "ros2", "launch", "velodyne", "velodyne-all-nodes-VLP16-launch.py"
    ]

    return LaunchDescription([        
        ExecuteProcess(cmd=velodyne_cmd, output="screen"),
    ])
