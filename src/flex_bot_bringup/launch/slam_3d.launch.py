import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    pkg_share = get_package_share_directory("flex_bot_bringup")
    default_config_file = os.path.join(pkg_share, "config", "slam_config_3d.yaml")

    slam_config_file = LaunchConfiguration("slam_config_file")

    slam_share = get_package_share_directory("rko_lio")
    slam_launch = os.path.join(slam_share, "launch", "odometry.launch.py")

    return LaunchDescription([
        DeclareLaunchArgument("slam_config_file", default_value=default_config_file),

        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(slam_launch),
            launch_arguments={
                "config_file": slam_config_file
            }.items(),
        ),
    ])
