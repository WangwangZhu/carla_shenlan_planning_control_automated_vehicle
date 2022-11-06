
#!/usr/bin/env python
#
# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
# Copyright (c) 2019 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


from __future__ import print_function

# import datetime
import math
from threading import Thread

import numpy
from transforms3d.euler import quat2euler

import ros_compatibility as roscomp
from ros_compatibility.node import CompatibleNode
from ros_compatibility.qos import QoSProfile, DurabilityPolicy

from carla_msgs.msg import CarlaEgoVehicleControl
from nav_msgs.msg import Odometry
import numpy as np
import threading

# ==============================================================================
# -- ShenlanControl -----------------------------------------------------------
# ==============================================================================

class ShenlanControl(CompatibleNode):
    """
    Handle input events
    """
    def __init__(self):
        super(ShenlanControl, self).__init__("ShenlanControl")
        self.role_name = self.get_param("role_name", "ego_vehicle")
        self.data_lock = threading.Lock()
        self._control = CarlaEgoVehicleControl()
        self._steer_cache = 0.0

        fast_qos = QoSProfile(depth=10)

        self.vehicle_control_manual_override = True

        self.vehicle_control_publisher = self.new_publisher(
            CarlaEgoVehicleControl,
            "/carla/{}/vehicle_control_cmd".format(self.role_name),
            qos_profile=fast_qos)
        
        self._odometry_subscriber = self.new_subscription(
            Odometry,
            "/carla/{}/odometry".format(self.role_name),
            self.odometry_cb,
            qos_profile=10)
        
        self._K_P = 0.206
        self._K_D = 0.515
        self._K_I = 0.0206
        self.error = 0.0
        self.error_integral = 0.0
        self.error_derivative = 0.0
        
        self._current_speed = 0
        self._current_pose = None
        
        self.target_speed = 30 # km/h

    def odometry_cb(self, odometry_msg):
        with self.data_lock:
            self.loginfo("I hear imu.")
            self._current_pose = odometry_msg.pose.pose
            self._current_speed = math.sqrt(odometry_msg.twist.twist.linear.x ** 2 +
                                            odometry_msg.twist.twist.linear.y ** 2 +
                                            odometry_msg.twist.twist.linear.z ** 2) * 3.6

    def _on_new_carla_frame(self):
        """
        callback on new frame

        As CARLA only processes one vehicle control command per tick,
        send the current from within here (once per frame)
        """
        acceleration_cmd = self._pid_run_step(self.target_speed, self._current_speed)
        print("acceleration_cmd: {}".format(acceleration_cmd))
        try:
            self._parse_vehicle_keys(acceleration_cmd, 0.0, 0.0)
            self.vehicle_control_publisher.publish(self._control)
        except Exception as error:
            self.node.logwarn("Could not send vehicle control: {}".format(error))

    def _pid_run_step(self, target_speed, _current_speed):
        """
        Estimate the throttle of the vehicle based on the PID equations

        :param target_speed:  target speed in Km/h
        :param current_speed: current speed of the vehicle in Km/h
        :return: throttle control in the range [0, 1]
        """
        previous_error = self.error
        self.error = target_speed - _current_speed
        print("velocity error: {}".format(self.error))
        # restrict integral term to avoid integral windup
        self.error_integral = np.clip(self.error_integral + self.error, -40.0, 40.0)
        self.error_derivative = self.error - previous_error
        output = self._K_P * self.error + self._K_I * self.error_integral + self._K_D * self.error_derivative
        return np.clip(output, 0.0, 1.0)
    
    def _parse_vehicle_keys(self, throttle, steer, brake):
        """
        parse key events
        """
        self._control.header.stamp = self.get_clock().now().to_msg()
        self._control.throttle = throttle
        self._control.steer = steer  # round(self._steer_cache, 1)
        self._control.brake = brake
        self._control.hand_brake = False
        self._control.reverse = False
        self._control.gear = 1
        self._control.manual_gear_shift = False
# ==============================================================================
# -- main() --------------------------------------------------------------------
# ==============================================================================

def main(args=None):
    """
    main function
    """
    roscomp.init("ShenlanControl", args=args)

    try:
        shenlan_control_node = ShenlanControl()
        executor = roscomp.executors.MultiThreadedExecutor()
        executor.add_node(shenlan_control_node)
        update_timer = shenlan_control_node.new_timer(0.01, lambda timer_event=None: shenlan_control_node._on_new_carla_frame())
        target = shenlan_control_node.spin()
        
    except KeyboardInterrupt:
        roscomp.loginfo("User requested shut down.")
    finally:
        roscomp.shutdown()


if __name__ == '__main__':
    main()
