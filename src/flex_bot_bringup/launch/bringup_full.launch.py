import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource

def generate_launch_description():
    teleop_share = get_package_share_directory("flex_bot_teleop")
    sensors_share = get_package_share_directory("flex_bot_sensors")
    odom_share = get_package_share_directory("flex_bot_odometry")
    bringup_share = get_package_share_directory("flex_bot_bringup")
    

    teleop = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(teleop_share, "launch", "teleop.launch.py"))
    )

    static_tfs = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(sensors_share, "launch", "static_tfs.launch.py"))
    )

    sensors = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(sensors_share, "launch", "sensors.launch.py"))
    )

    state_estimation = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(odom_share, "launch", "state_estimation.launch.py"))
    )

    slam = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(bringup_share, "launch", "slam_2d.launch.py"))
    )

    foxglove = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(bringup_share, "launch", "foxglove.launch.py"))
    )

    return LaunchDescription([
        teleop,
        static_tfs,
        sensors,
        state_estimation,
        slam,
        foxglove,
    ])
