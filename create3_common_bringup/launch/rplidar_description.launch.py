# @author Scottcandy34
#
# Launch Create(R) 3 state publishers.

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction
from launch.substitutions import Command, LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node, PushRosNamespace


ARGUMENTS = [
    DeclareLaunchArgument('visualize_rays', default_value='false',
                          choices=['true', 'false'],
                          description='Enable/disable ray visualization'),
    DeclareLaunchArgument('namespace', default_value='',
                          description='Robot namespace'),
]


def generate_launch_description():
    # Directory
    pkg_create3_description = get_package_share_directory('create3_description')
    
    # Path
    xacro_file = PathJoinSubstitution([pkg_create3_description, 'urdf', 'sensors', 'rplidar.urdf.xacro'])
    
    # Launch Configurations
    visualize_rays = LaunchConfiguration('visualize_rays')
    namespace = LaunchConfiguration('namespace')

    rplidar_group = GroupAction(
        actions=[
            Node(
                package='robot_state_publisher',
                executable='robot_state_publisher',
                name='rplidar_state_publisher',
                output='screen',
                parameters=[
                    {'use_sim_time': False},
                    {'robot_description':
                    Command(
                        ['xacro', ' ', xacro_file, ' ',
                        'namespace:=', namespace, ' ',
                        'visualize_rays:=', visualize_rays])},
                    {'publish_frequency': 0.0},  # No TF publishing needed
                    {'frame_prefix': ''},  # No TF prefix
                ],
                remappings=[
                    ('robot_description', 'rplidar_description'),
                    ('/tf', 'tf'),
                    ('/tf_static', 'tf_static')
                ],
            ),
            Node(
                package='tf2_ros',
                executable='static_transform_publisher',
                name='tf_base_link_mount_link_publisher',
                output='screen',
                arguments=[
                    '--x', '-0.015',
                    '--y', '0',
                    '--z', '0.1458',
                    '--yaw', '0',
                    '--pitch', '0',
                    '--roll', '0',
                    '--frame-id', 'base_link',
                    '--child-frame-id', 'mount_link'
                ],
                remappings=[
                    ('/tf', 'tf'),
                    ('/tf_static', 'tf_static')
                ],
            )
        ]
    )

    # Define LaunchDescription variable
    ld = LaunchDescription(ARGUMENTS)

    # Add nodes to LaunchDescription
    ld.add_action(rplidar_group)

    return ld
