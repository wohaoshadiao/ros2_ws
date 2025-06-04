#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "geometry_msgs/msg/twist.hpp"

class LidarListener : public rclcpp::Node
{
public:
    LidarListener() : Node("lidar_listener_cpp")
    {
        subscription_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
            "/scan", 10,
            std::bind(&LidarListener::scan_callback, this, std::placeholders::_1));

        cmd_pub_ = this->create_publisher<geometry_msgs::msg::Twist>(
            "/cmd_vel", 10);
        RCLCPP_INFO(this->get_logger(), "LidarListener node has been started.");
    }

private:
//     void scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
//     {
//         size_t num_readings = msg->ranges.size();

//         if (num_readings == 0)
//         {
//             RCLCPP_WARN(this->get_logger(), "No laser scan data received.");
//             return;
//         }

//         float angle = msg->angle_min;

//         for (size_t i = 0; i < num_readings; ++i)
//         {
//             float distance = msg->ranges[i];
//             float angle_deg = angle * 180.0 / M_PI; // 转为角度

//             // 过滤掉无效的测距
//             if (std::isfinite(distance))
//             {
//                 RCLCPP_INFO(this->get_logger(), "Angle: %.2f°, Distance: %.2f m", angle_deg, distance);
//             }

//             angle += msg->angle_increment;
//         }
//     }

//     rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr subscription_;
// };

    void scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
    {
        // 获取激光雷达数组大小
        int range_size = msg->ranges.size();
        // 获取中间的索引号
        int center_index = range_size / 2;
        // 设定扫扫描的区间
        int range_margin = range_size / 12;

        float min_front_dist = *std::min_element(
            msg->ranges.begin() + center_index - range_margin,
            msg->ranges.begin() + center_index + range_margin);

        RCLCPP_INFO(this->get_logger(), "Front min distance: %.2f m", min_front_dist);
        
        //定义速度指令变量类型（包含线速度和角速度）
        geometry_msgs::msg::Twist cmd_msg;                  

        if (min_front_dist > 1.5)
        {
            // 前方畅通，继续前进
            cmd_msg.linear.x = 0.3;
        }
        else
        {
            // std::min_element()函数返回的是最小值的迭代器
            // 计算左侧(3/4位置处和结束)和右侧（开始位置和1/4位置）的最小距离
            float min_left = *std::min_element(msg->ranges.begin() + (3 * range_size / 4), msg->ranges.end());
            float min_right = *std::min_element(msg->ranges.begin(), msg->ranges.begin() + range_size / 4);

            if (min_left > min_right && min_left > 0.5)
            {
                // 左侧更空，左转避障
                cmd_msg.angular.z = 0.5;
            }
            else if (min_right > 0.5)
            {
                // 右侧更空，右转避障
                cmd_msg.angular.z = -0.5;
            }
            else
            {
                // 四周都不太安全，后退
                cmd_msg.linear.x = -0.2;
            }
        }

        cmd_pub_->publish(cmd_msg);
    }

    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr subscription_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_pub_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<LidarListener>());
    rclcpp::shutdown();
    return 0;
}
