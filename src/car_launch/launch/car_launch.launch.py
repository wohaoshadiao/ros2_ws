
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        
        Node(
            package='robocar_test_cpp',
            executable='talker',
            output='screen'
        ),
        Node(
            package='robocar_test_cpp',
            executable='listener',
            output='screen'
        ),
    ])
