#ifndef MPC_LATERAL_LONGITUDINAL_H_
#define MPC_LATERAL_LONGITUDINAL_H_

#include <stdint.h>
#include <memory>
#include <nav_msgs/msg/odometry.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <nav_msgs/msg/path.hpp>
#include <rclcpp/rclcpp.hpp>

#include "carla_shenlan_lattice_planner/mpc_controller.h"
#include "carla_shenlan_lattice_planner/pid_controller.h"

#include "carla_shenlan_lattice_planner/helpers.h"
#include "carla_shenlan_lattice_planner/spline.h"
#include "carla_shenlan_lattice_planner/coordinate_transform.h"
#include "carla_shenlan_lattice_planner/lattice_planner.h"
#include "carla_shenlan_lattice_planner/frenet_path.h"

#include <fstream>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_ros/transform_broadcaster.h>
#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

#include "carla_msgs/msg/carla_ego_vehicle_control.hpp"
#include "carla_msgs/msg/carla_status.hpp"
#include <std_msgs/msg/float32.hpp>
#include "carla_msgs/msg/carla_vehicle_target_velocity.hpp"
#include "carla_msgs/msg/carla_ego_vehicle_status.hpp"

#include <cppad/cppad.hpp>
#include <cppad/ipopt/solve.hpp>
#include <vector>
#include "rclcpp/rate.hpp"
#include <cstdio>
#include <thread>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <iomanip>

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/QR>
#include <eigen3/unsupported/Eigen/Splines>

#include <geometry_msgs/msg/quaternion.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>

#include <nav_msgs/msg/odometry.hpp>
#include <visualization_msgs/msg/marker.hpp>

#include <tf2/LinearMath/Quaternion.h>
#include <tf2/transform_datatypes.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <tf2/convert.h>

#include <visualization_msgs/msg/marker.hpp>
#include <visualization_msgs/msg/marker_array.hpp>


using namespace shenlan::control;
// using namespace std;

using namespace std::chrono_literals; // ?????????????????????????????????
//??????????????????std::chrono_literals????????????
using std::cout;
using std::endl;
using std::vector;
using std::placeholders::_1;

using CppAD::AD;
using Eigen::VectorXd;

template <typename U, typename V>
double DistanceXY(const U &u, const V &v) {
    return std::hypot(u.x - v.x, u.y - v.y);
}

class LatticePlannerNode : public rclcpp::Node {
   public:
    LatticePlannerNode();
    ~LatticePlannerNode();
    bool init();

    // ???????????????
    rclcpp::TimerBase::SharedPtr vehicle_control_iteration_timer;
    void VehicleControllerIterationCallback();   

    rclcpp::TimerBase::SharedPtr global_path_publish_timer;
    void GlobalPathPublishCallback();

    rclcpp::TimerBase::SharedPtr obstacles_vis_publish_timer;
    void ObstacleVisPublishCallback();

    // ????????????????????????????????????????????????????????????????????????

    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr localization_data_subscriber;
    void OdomCallback(nav_msgs::msg::Odometry::SharedPtr msg);

    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr lacalization_data_imu_subscriber;
    void IMUCallback(sensor_msgs::msg::Imu::SharedPtr msg);

    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr history_path_visualization_publisher;
    nav_msgs::msg::Path history_path;
    geometry_msgs::msg::PoseStamped history_path_points;

    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr global_path_publisher_;
    nav_msgs::msg::Path global_path;
    geometry_msgs::msg::PoseStamped this_pose_stamped;

    // ????????????????????????
    void loadRoadmap(const std::string &roadmap_path);

    //???????????????????????????
    double pointDistance(const TrajectoryPoint &point, const double x, const double y) {
        double dx = point.x - x;
        double dy = point.y - y;
        return sqrt(dx * dx + dy * dy);
    }
    double pointDistance(const double x1, const double y1, const double x, const double y) {
        double dx = x1 - x;
        double dy = y1 - y;
        return sqrt(dx * dx + dy * dy);
    }

    rclcpp::Publisher<carla_msgs::msg::CarlaVehicleTargetVelocity>::SharedPtr vehicle_control_target_velocity_publisher;
    carla_msgs::msg::CarlaVehicleTargetVelocity vehicle_control_target_velocity;

    rclcpp::Subscription<carla_msgs::msg::CarlaEgoVehicleStatus>::SharedPtr carla_status_subscriber;
    void VehicleStatusCallback(carla_msgs::msg::CarlaEgoVehicleStatus::SharedPtr msg);
    
    rclcpp::Publisher<carla_msgs::msg::CarlaEgoVehicleControl>::SharedPtr vehicle_control_publisher;
    carla_msgs::msg::CarlaEgoVehicleControl control_cmd;

    rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr obstacles_visualization_publisher;
    visualization_msgs::msg::MarkerArray obstacles_all;
    visualization_msgs::msg::Marker obstacle_single;

    TrajectoryPoint QueryNearestPointByPosition(const double x, const double y);
    double PointDistanceSquare(const TrajectoryPoint &point, const double x, const double y);

   private:
    double targetSpeed_ = 5;
    double controlFrequency_ = 100;                 //????????????
    double goalTolerance_ = 0.5;                    //????????????????????????
    bool isReachGoal_ = false;
    bool firstRecord_ = true;
    int cnt;

    std::string _line;
    std::vector<std::pair<double, double>> xy_points;
    std::vector<double> v_points;

    TrajectoryData planning_published_trajectory;
    TrajectoryPoint goal_point;
    std::vector<TrajectoryPoint> trajectory_points_;

    std::string roadmap_path;
    double speed_P, speed_I, speed_D, target_speed;

    VehicleState vehicleState_;
    // TrajectoryData planningPublishedTrajectory_;    //???????????????
    // TrajectoryPoint goalPoint_;                     //??????

    std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_gps_vehicle;
    // std::unique_ptr<shenlan::control::PIDController> pid_controller_longitudinal;
    // std::unique_ptr<shenlan::control::MPCController> mpc_controller_lateral;

    double delta; // the current steering angle from carla
    double a_longitudinal; // the acceleration 
    double a_lateral;

    double steer_value;
    double throttle_value;

    double px;
    double py;
    double psi;
    double yaw_rate;
    double v_longitudinal;
    double v_lateral;

    vector<double> global_path_remap_x;
    vector<double> global_path_remap_y;

    int former_point_of_current_position;
    double old_steer_value;
    double old_throttle_value;

    visualization_msgs::msg::Marker reference_path;
    visualization_msgs::msg::Marker mpc_output_path;

    rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr mpc_reference_path_publisher;
    rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr mpc_output_path_publisher;
    int reference_path_id = 101;

    int mpc_cte_weight_int;
    int mpc_epsi_weight_int;
    int mpc_v_weight_int;
    int mpc_steer_actuator_cost_weight_int;
    int mpc_acc_actuator_cost_weight_int;
    int mpc_change_steer_cost_weight_int;
    int mpc_change_accel_cost_weight_int;

    double ref_v;

    double vehicle_steering_ratio_double;
    double vehicle_Lf_double;

    int mpc_control_horizon_length_int;
    double mpc_control_step_length_double;

    bool mpc_tracking_enable_bool;

    mpc_controller mpc;

    double target_v; // km/h

    // ?????????????????????????????????????????????????????????????????????
    double steering_ratio;
    double kinamatic_para_Lf;

    // MPC????????????????????????????????????
    int mpc_control_horizon_length; // MPC???????????????,?????????????????? MPC ???????????????,????????????,?????? MPC ??????????????????????????????????????????.
    double mpc_control_step_length; //Original 0.1 ??????????????????,??????????????????????????????????????????????????????

    // MPC ???????????????????????????????????????
    int cte_weight; //Original2000
    int epsi_weight;
    int v_weight;
    int steer_actuator_cost_weight; //Original 6000
    int acc_actuator_cost_weight; //Original 6000
    int change_steer_cost_weight;
    int change_accel_cost_weight;

    double ins_delay;

    // MPC ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
    double reference_path_length;

    int reference_path_points_number;

    double controller_delay_compensation; // ????????????????????????

    // ??? RVIZ ???????????????????????????????????????????????????????????????????????????,??????????????????????????????????????????
    double point_distance_of_reference_line_visualization; // step on x
    int points_number_of_reference_line_visualization;     /* how many point "int the future" to be plotted. */
    bool mpc_enable_signal = true;
    int working_mode;
    int with_planner_flag; // MPC???????????????????????????????????????????????????????????? ??? 0 ?????????????????? 1 ???????????????????????????
    std_msgs::msg::Float32 mpc_iteration_duration_msg = std_msgs::msg::Float32();
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr mpc_iteration_time_publisher;

    void GetWayPoints();
    Vec_f wx_, wy_;
    Spline2D *csp_obj_;
    void GenerateGlobalPath();
    int GetNearestReferenceIndex(const VehicleState &ego_state);    // ???????????????????????????????????????????????????????????????id
    void UpdateStaticObstacle();
    double GetNearestReferenceLength(const VehicleState &ego_state);
    double GetNearestReferenceLatDist(const VehicleState &ego_state);
    bool LeftOfLine(const VehicleState &p, const geometry_msgs::msg::PoseStamped &p1, const geometry_msgs::msg::PoseStamped &p2);
    // nav_msgs::msg::Path global_plan_;
    // double end_x_, end_y_, end_s_;
    auto createQuaternionMsgFromYaw(double yaw) {
        tf2::Quaternion q;
        q.setRPY(0, 0, yaw);
        return tf2::toMsg(q);
    };
    std::vector<Poi_f> obstcle_list_;
    rclcpp::TimerBase::SharedPtr lattice_planner_timer;
    void LatticePlannerCallback();
    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr replan_path_publisher_;
    TrajectoryData GetTrajectoryFromFrenetPath(const FrenetPath &path);
    float c_speed_ = 10.0 / 3.6;
    float c_d_ = 0;
    float c_d_d_ = 0.0;
    float c_d_dd_ = 0.0;
    float s0_ = 0.0;
    nav_msgs::msg::Path global_plan_;
    double end_x_, end_y_, end_s_;
    bool near_goal_ = false;
    bool use_reference_line_ = false;
    TrajectoryData planningPublishedTrajectoryDebug_;    //?????????????????????
    TrajectoryData last_trajectory_;                     //?????????????????????
    bool plannerFlag_ = false;


};
#endif /* __MPC_CONTROLLER_NODE_H__ */
