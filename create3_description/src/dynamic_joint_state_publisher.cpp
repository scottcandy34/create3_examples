#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <irobot_create_msgs/msg/hazard_detection_vector.hpp>

class VisualizerNode : public rclcpp::Node {
public:
  VisualizerNode() : Node("dynamic_joint_state_publisher"),
                     wheel_separation_(0.233), wheel_radius_(0.03575),
                     left_wheel_pos_(0.0), right_wheel_pos_(0.0),
                     left_drop_pos_(0.0), right_drop_pos_(0.0),
                     bumper_pos_(0.0),
                     drop_threshold_(-0.036),
                     last_time_(now()) {
    odom_sub_ = create_subscription<nav_msgs::msg::Odometry>("/odom", 10, std::bind(&VisualizerNode::odomCallback, this, std::placeholders::_1));
    hazard_sub_ = create_subscription<irobot_create_msgs::msg::HazardDetectionVector>("/hazard_detection", 10, std::bind(&VisualizerNode::hazardCallback, this, std::placeholders::_1));
    joint_pub_ = create_publisher<sensor_msgs::msg::JointState>("/joint_states", 10);
    timer_ = create_wall_timer(std::chrono::milliseconds(16), std::bind(&VisualizerNode::publishJointStates, this));  // ~60 Hz
  }

private:
  void odomCallback(const nav_msgs::msg::Odometry::SharedPtr msg) {
    auto current_time = now();
    double dt = (current_time - last_time_).seconds();
    last_time_ = current_time;

    double linear_x = msg->twist.twist.linear.x;
    double angular_z = msg->twist.twist.angular.z;

    left_wheel_pos_ += (linear_x - angular_z * wheel_separation_ / 2.0) * dt / wheel_radius_;
    right_wheel_pos_ += (linear_x + angular_z * wheel_separation_ / 2.0) * dt / wheel_radius_;
  }

  void hazardCallback(const irobot_create_msgs::msg::HazardDetectionVector::SharedPtr msg) {
    bool left_drop = false, right_drop = false, bump = false;
    for (const auto& detection : msg->detections) {
      if (detection.type == irobot_create_msgs::msg::HazardDetection::STALL) {
        if (detection.header.frame_id.find("left_wheel") != std::string::npos) left_drop = true;
        if (detection.header.frame_id.find("right_wheel") != std::string::npos) right_drop = true;
      }
      if (detection.type == irobot_create_msgs::msg::HazardDetection::BUMP) {
        bump = true;
      }
    }

    // Direct state for drops (negative for dropped)
    left_drop_pos_ = left_drop ? -drop_threshold_ : 0.0;
    right_drop_pos_ = right_drop ? -drop_threshold_ : 0.0;

    // Direct state for bumper
    bumper_pos_ = bump ? -0.003 : 0.0;
  }

  void publishJointStates() {
    sensor_msgs::msg::JointState js;
    js.header.stamp = now();
    js.name = {"left_wheel_joint", "right_wheel_joint", "wheel_drop_left_joint", "wheel_drop_right_joint", "bumper_joint"};
    js.position = {left_wheel_pos_, right_wheel_pos_, left_drop_pos_, right_drop_pos_, bumper_pos_};
    joint_pub_->publish(js);
  }

  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Subscription<irobot_create_msgs::msg::HazardDetectionVector>::SharedPtr hazard_sub_;
  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_pub_;
  rclcpp::TimerBase::SharedPtr timer_;
  double wheel_separation_, wheel_radius_;
  double left_wheel_pos_, right_wheel_pos_, left_drop_pos_, right_drop_pos_, bumper_pos_;
  double drop_threshold_;
  rclcpp::Time last_time_;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<VisualizerNode>());
  rclcpp::shutdown();
  return 0;
}