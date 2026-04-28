from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    # tf_tof_left = Node(
    #     package="tf2_ros",
    #     executable="static_transform_publisher",
    #     name="static_tf_xsens_imu",
    #     output="screen",
    #     arguments=["0", "0", "0", "0", "0", "0", "base_link", "xsens_imu"],
    # )

    # tf_tof_right = Node(
    #     package="tf2_ros",
    #     executable="static_transform_publisher",
    #     name="static_tf_xsens_imu",
    #     output="screen",
    #     arguments=["0", "0", "0", "0", "0", "0", "base_link", "xsens_imu"],
    # )
    
    tf_lidar = Node(
        package="tf2_ros",
        executable="static_transform_publisher",
        name="static_tf_velodyne",
        output="screen",
        arguments=["0.16", "0", "0.31", "0", "0.122", "0", "base_link", "velodyne"],
    )

    tf_imu = Node(
    package="tf2_ros",
    executable="static_transform_publisher",
    name="static_tf_xsens_imu",
    output="screen",
    arguments=[
        "--qx", "0.0",
        "--qy", "0.0",
        "--qz", "0.0",
        "--qw", "1.0",
        "--x", "0.05",
        "--y", "-0.06",
        "--z", "0.15",
        "--frame-id", "base_link",
        "--child-frame-id", "xsens_imu"
    ],
    )

    return LaunchDescription([tf_lidar, tf_imu])
