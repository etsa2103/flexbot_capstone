from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    foxglove_bridge = Node(
        package="foxglove_bridge",
        executable="foxglove_bridge",
        name="foxglove_bridge",
        output="screen", 
    )

    return LaunchDescription([foxglove_bridge])