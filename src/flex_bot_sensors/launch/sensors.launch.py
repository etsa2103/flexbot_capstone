from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    pkg_share = get_package_share_directory("flex_bot_sensors")
    udp_yaml = os.path.join(pkg_share, "config", "flex_bot_udp.yaml")

    velodyne_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            os.path.join(
                get_package_share_directory("velodyne"),
                "launch",
                "velodyne-all-nodes-VLP16-launch.py"
            )
        ])
    )

    return LaunchDescription([
        Node(
            package="flex_bot_sensors",
            executable="flex_bot_udp_bridge",
            name="flex_bot_udp_bridge",
            output="screen",
            parameters=[udp_yaml],
        ),
        velodyne_launch
    ])
