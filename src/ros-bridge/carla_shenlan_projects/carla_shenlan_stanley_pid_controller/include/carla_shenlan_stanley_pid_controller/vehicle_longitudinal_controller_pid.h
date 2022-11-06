#ifndef VEHICLE_CONTROL_H
#define VEHICLE_CONTROL_H

#include "carla_shenlan_stanley_pid_controller/common.h"
#include "rclcpp/rclcpp.hpp"
// #include "pid_controller.h"
#include <std_msgs/msg/bool.hpp>

#include <nav_msgs/msg/path.hpp>
#include <nav_msgs/msg/odometry.hpp>

#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_ros/transform_broadcaster.h>

#include "carla_shenlan_stanley_pid_controller/pid_controller.h"
#include "carla_shenlan_stanley_pid_controller/stanely_controller.h"

#include "carla_msgs/msg/carla_ego_vehicle_control.hpp"
#include "carla_msgs/msg/carla_status.hpp"
#include <std_msgs/msg/float32.hpp>
#include "carla_msgs/msg/carla_vehicle_target_velocity.hpp"
#include "carla_msgs/msg/carla_ego_vehicle_status.hpp"

using std::placeholders::_1;

class VehicleControlPublisher : public rclcpp::Node
{
public:
    VehicleControlPublisher();
    ~VehicleControlPublisher();

    double PointDistanceSquare(const TrajectoryPoint &point, const double x, const double y);
    
    TrajectoryPoint QueryNearestPointByPosition(const double x, const double y);

    void odomCallback(nav_msgs::msg::Odometry::SharedPtr msg);

    void VehicleStatusCallback(carla_msgs::msg::CarlaEgoVehicleStatus::SharedPtr msg);

public:
    double V_set_;
    double T_gap_;

    bool first_record_;
    bool cout_distance_;
    bool cout_speed_;

    double wheelbase_;
    double car_length_;

    int cnt;
    int qos;

    std::vector<std::pair<double, double>> xy_points;
    std::vector<double> v_points;
    std::string _line;
    std::string count_csv;
    std::string timestamp;
    std::string x;
    std::string y;
    std::string z;
    std::string yaw;
    std::string velocity;

    double controller_frequency;

    double acceleration_cmd;
    double steer_cmd;

    std::vector<TrajectoryPoint> trajectory_points_;
    TrajectoryData planning_published_trajectory;

    // Input
    VehicleState vehicle_state_;

    TrajectoryPoint goal_point;

    ControlCmd cmd;

    tf2::Quaternion localization_quaternion_transform;

    rclcpp::TimerBase::SharedPtr vehicle_control_iteration_timer;
    void VehicleControlIterationCallback(); // 收到仿真器返回的状态后，产生控制信号
    // void VehicleControlIterationCallback(carla_msgs::msg::CarlaStatus::SharedPtr msg); // 收到仿真器返回的状态后，产生控制信号

    rclcpp::Publisher<carla_msgs::msg::CarlaEgoVehicleControl>::SharedPtr vehicle_control_publisher;
    carla_msgs::msg::CarlaEgoVehicleControl control_cmd;

    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr vehicle_control_manual_override_publisher;
    std_msgs::msg::Bool vehicle_control_manual_override;

    rclcpp::Publisher<carla_msgs::msg::CarlaVehicleTargetVelocity>::SharedPtr vehicle_control_target_velocity_publisher;
    carla_msgs::msg::CarlaVehicleTargetVelocity vehicle_control_target_velocity;

    rclcpp::Subscription<carla_msgs::msg::CarlaEgoVehicleStatus>::SharedPtr carla_status_subscriber;

    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr localization_data_subscriber;

    rclcpp::TimerBase::SharedPtr global_path_publish_timer; 
    void GlobalPathPublishCallback();


    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr history_path_visualization_publisher;
    nav_msgs::msg::Path history_path;
    geometry_msgs::msg::PoseStamped history_path_points;

    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr global_path_publisher_;
    nav_msgs::msg::Path global_path;
    geometry_msgs::msg::PoseStamped this_pose_stamped;

    std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_gps_vehicle;

    std::unique_ptr<shenlan::control::PIDController> pid_controller_longitudinal;
    std::unique_ptr<shenlan::control::StanleyController> stanely_controller_lateral;
    
};

#endif