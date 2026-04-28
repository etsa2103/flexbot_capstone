import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    pkg_share = get_package_share_directory("flex_bot_odometry")
    default_odom_params = os.path.join(pkg_share, "config", "wheel_odom.yaml")
    default_ekf_params = os.path.join(pkg_share, "config", "ekf_imu.yaml")

    odom_yaml = LaunchConfiguration("odom_yaml")
    ekf_yaml = LaunchConfiguration("ekf_yaml")

    odom_node = Node(
            package="flex_bot_odometry",
            executable="wheel_odom_node",
            name="wheel_odom_node",
            output="screen",
            parameters=[odom_yaml],
        ),

    ekf_node = Node(
        package="robot_localization",
        executable="ekf_node",
        name="ekf_filter_node",
        output="screen",
        parameters=[ekf_yaml],
    )

    odom_path_node = Node(
        package="flex_bot_odometry",
        executable="odom_to_path",
        name="odom_path_node",
        output="screen",
    )

    return LaunchDescription([
        DeclareLaunchArgument("odom_yaml", default_value=default_odom_params),
        DeclareLaunchArgument("ekf_yaml", default_value=default_ekf_params),
        odom_node,
        ekf_node,
        odom_path_node,
    ])
