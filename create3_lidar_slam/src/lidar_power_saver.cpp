#include <rclcpp/rclcpp.hpp>
#include <std_srvs/srv/empty.hpp>
#include <chrono>
#include <memory>

using namespace std::chrono_literals;

class LidarPowerSaver : public rclcpp::Node
{
public:
  LidarPowerSaver() : Node("lidar_power_saver")
  {
    // Create timer (50 ms = 0.05 s, same as Python)
    timer_ = this->create_wall_timer(
      50ms, std::bind(&LidarPowerSaver::watcher_callback, this));

    // Service clients (exactly the same services your Python node used)
    stop_motor_client_ = this->create_client<std_srvs::srv::Empty>("stop_motor");
    start_motor_client_ = this->create_client<std_srvs::srv::Empty>("start_motor");

    motor_enabled_ = true;

    // Start the motor on node launch (matches your Python code)
    RCLCPP_INFO(this->get_logger(), "Starting LIDAR motor initially");
    auto req = std::make_shared<std_srvs::srv::Empty::Request>();
    start_motor_client_->async_send_request(req);
  }

private:
  void watcher_callback()
  {
    // Exact equivalent of Python's get_subscriptions_info_by_topic('scan')
    auto sub_info = this->get_subscriptions_info_by_topic("scan");

    if (sub_info.empty() && motor_enabled_)
    {
      RCLCPP_INFO(this->get_logger(), "No subscribers on /scan → stopping LIDAR motor for power saving");
      auto req = std::make_shared<std_srvs::srv::Empty::Request>();
      stop_motor_client_->async_send_request(req);
      motor_enabled_ = false;
    }
    else if (!sub_info.empty() && !motor_enabled_)
    {
      RCLCPP_INFO(this->get_logger(), "Subscribers detected on /scan → starting LIDAR motor");
      auto req = std::make_shared<std_srvs::srv::Empty::Request>();
      start_motor_client_->async_send_request(req);
      motor_enabled_ = true;
    }
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Client<std_srvs::srv::Empty>::SharedPtr stop_motor_client_;
  rclcpp::Client<std_srvs::srv::Empty>::SharedPtr start_motor_client_;
  bool motor_enabled_;
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<LidarPowerSaver>());
  rclcpp::shutdown();
  return 0;
}