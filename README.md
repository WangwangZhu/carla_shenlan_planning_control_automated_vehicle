# carla 里面方向盘左正右负

# PID & Foxy 启动流程
1. cd /carla-ros-bridge
2. source source_env.sh 
3. colcon build
4. source source_env.sh
5. ros2 launch carla_shenlan_bridge_ego_vis carla_bridge_ego_vehilce.launch.py
6. 在新的终端里面: ros2 run carla_shenlan_pid_controller carla_shenlan_pid_controller_node

# Stanley & Foxy 需要完成的内容
1. src/ros-bridge/carla_shenlan_projects/carla_shenlan_stanley_pid_controller/src/stanley_controller.cpp 中的 TODO 部分

# Stanley & Foxy 启动流程
1. cd /carla-ros-bridge
2. source source_env.sh
3. colcon build
4. source source_env.sh
5. ros2 launch carla_shenlan_bridge_ego_vis carla_bridge_ego_vehilce.launch.py
6. 在新的终端里面: ros2 run carla_shenlan_stanley_pid_controller carla_shenlan_stanley_pid_controller_node

# LQR & Foxy 需要完成的内容
1. carla-ros-bridge/src/ros-bridge/carla_shenlan_projects/carla_shenlan_lqr_pid_controller/src/lqr_controller.cpp 中的 TODO 部分

# LQR & Foxy 启动流程
1. cd /carla-ros-bridge
2. source source_env.sh
3. colcon build
4. source source_env.sh
5. ros2 launch carla_shenlan_bridge_ego_vis carla_bridge_ego_vehilce.launch.py
6. 在新的终端里面: ros2 launch carla_shenlan_lqr_pid_controller lqr_launch.py

# MPC & Foxy 需要完成的内容
1. carla-ros-bridge/src/ros-bridge/carla_shenlan_projects/carla_shenlan_mpc_controller/src/mpc_controller.cpp 中的 TODO 部分

# MPC & Foxy 启动流程
1. cd /carla-ros-bridge
2. source source_env.sh
3. colcon build
4. source source_env.sh
5. ros2 launch carla_shenlan_bridge_ego_vis carla_bridge_ego_vehilce.launch.py
6. 在新的终端里面: ros2 launch carla_shenlan_mpc_controller mpc_launch.py

# Lattice & Foxy 启动流程
1. 需要完成部分： lattice_planner.cpp 中的TODO部分
2. cd /carla-ros-bridge
3. source source_env.sh
4. colcon build
5. source source_env.sh
6. ros2 launch carla_shenlan_bridge_ego_vis carla_bridge_ego_vehilce.launch.py
7. 在新的终端里面: ros2 launch carla_shenlan_lattice_planner lattice_launch.py

# A* & Foxy 启动流程
1. cd /carla-ros-bridge
2. source source_env.sh
3. colcon build
4. source source_env.sh
5. ros2 launch carla_shenlan_a_star_planner a_star_planner.launch.py
## 作业要求
1. carla-ros-bridge/src/ros-bridge/carla_shenlan_projects/carla_shenlan_a_star_planner/src/Astar_searcher.cpp 中的 TODO部分
2. 对比分析不同的启发函数的计算耗时，每次运行后在终端内会打印计算时间，需要截图放入文档中上传。
![alt](./2022-11-06_15-06.png)
