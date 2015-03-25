/*
 * Copyright 2015 Andreas Bircher, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Fadri Furrer, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Michael Burri, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Mina Kamel, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Janosch Nikolic, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Markus Achtelik, ASL, ETH Zurich, Switzerland
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thread>
#include <chrono>
#include <fstream>

#include <ros/ros.h>
#include <ros/package.h>
#include <std_srvs/Empty.h>
#include <mav_msgs/CommandTrajectory.h>
#include "nbvPlanner/nbvp_srv.h"
#include "tf/tf.h"

int main(int argc, char** argv){
  ros::init(argc, argv, "hovering_example");
  ros::NodeHandle nh;
  ros::Publisher trajectory_pub = nh.advertise<mav_msgs::CommandTrajectory>("command/trajectory", 10);
  ros::ServiceClient pathPlanner = nh.serviceClient<nbvPlanner::nbvp_srv>("pathplanning/nbvplanner",10);
  ROS_INFO("Started hovering example.");

  std_srvs::Empty srv;
  bool unpaused = ros::service::call("/gazebo/unpause_physics", srv);
  unsigned int i = 0;

  // Trying to unpause Gazebo for 10 seconds.
  while (i <= 10 && !unpaused) {
    ROS_INFO("Wait for 1 second before trying to unpause Gazebo again.");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    unpaused = ros::service::call("/gazebo/unpause_physics", srv);
    ++i;
  }

  if (!unpaused) {
    ROS_FATAL("Could not wake up Gazebo.");
    return -1;
  }
  else {
    ROS_INFO("Unpaused the Gazebo simulation.");
  }

  // Wait for 5 seconds to let the Gazebo GUI show up.
  ros::Duration(5.0).sleep();

  mav_msgs::CommandTrajectory trajectory_msg;

  ROS_INFO("Starting the planner");
  nh.param<double>("wp_x", trajectory_msg.position.x, 0.0);
  nh.param<double>("wp_y", trajectory_msg.position.y, 0.0);
  nh.param<double>("wp_z", trajectory_msg.position.z, 7.0);
  trajectory_msg.yaw = 0.0;
  trajectory_msg.header.stamp = ros::Time::now();
  trajectory_pub.publish(trajectory_msg);
  ros::Duration(3.0).sleep();
  nh.param<double>("wp_x", trajectory_msg.position.x, -5.0);
  nh.param<double>("wp_y", trajectory_msg.position.y, 0.0);
  nh.param<double>("wp_z", trajectory_msg.position.z, 7.0);
  trajectory_msg.yaw = 0.0;
  trajectory_msg.header.stamp = ros::Time::now();
  trajectory_pub.publish(trajectory_msg);
  ros::Duration(2.0).sleep();
  nh.param<double>("wp_x", trajectory_msg.position.x, -10.0);
  nh.param<double>("wp_y", trajectory_msg.position.y, 0.0);
  nh.param<double>("wp_z", trajectory_msg.position.z, 7.0);
  trajectory_msg.yaw = 0.0;
  trajectory_msg.header.stamp = ros::Time::now();
  trajectory_pub.publish(trajectory_msg);
  ros::Duration(2.0).sleep();
  nh.param<double>("wp_x", trajectory_msg.position.x, -15.0);
  nh.param<double>("wp_y", trajectory_msg.position.y, 0.0);
  nh.param<double>("wp_z", trajectory_msg.position.z, 7.0);
  trajectory_msg.yaw = 0.0;
  trajectory_msg.header.stamp = ros::Time::now();
  trajectory_pub.publish(trajectory_msg);
  ros::Duration(3.0).sleep();
  nh.param<double>("wp_x", trajectory_msg.position.x, -10.0);
  nh.param<double>("wp_y", trajectory_msg.position.y, 0.0);
  nh.param<double>("wp_z", trajectory_msg.position.z, 7.0);
  trajectory_msg.yaw = 0.0;
  trajectory_msg.header.stamp = ros::Time::now();
  trajectory_pub.publish(trajectory_msg);
  ros::Duration(5.0).sleep();
  
  std::string pkgPath = ros::package::getPath("nbvplanner");
  std::fstream file;
  file.open((pkgPath+"/data/path.m").c_str(), std::ios::out);
  if(!file.is_open())
    ROS_WARN("could not open path file");
  while (ros::ok()) {
    ROS_INFO("Initiating replanning");
    nbvPlanner::nbvp_srv planSrv;
    if(ros::service::call("nbvplanner",planSrv))
    {
      for(int i = 0; i<10&&i<planSrv.response.path.size(); i++)
      {
        tf::Pose pose;
        tf::poseMsgToTF(planSrv.response.path[i], pose);
        double yaw = tf::getYaw(pose.getRotation());
        nh.param<double>("wp_x", trajectory_msg.position.x, planSrv.response.path[i].position.x);
        nh.param<double>("wp_y", trajectory_msg.position.y, planSrv.response.path[i].position.y);
        nh.param<double>("wp_z", trajectory_msg.position.z, planSrv.response.path[i].position.z);
        nh.param<double>("wp_yaw", trajectory_msg.yaw, yaw);
        /*ROS_INFO("Publishing waypoint on namespace %s: [%f, %f, %f] [%f].",
                 nh.getNamespace().c_str(),
                 trajectory_msg.position.x,
                 trajectory_msg.position.y,
                 trajectory_msg.position.z,
                 trajectory_msg.yaw);*/
        trajectory_msg.header.stamp = ros::Time::now();
        trajectory_pub.publish(trajectory_msg);
        file << planSrv.response.path[i].position.x<<", "<<planSrv.response.path[i].position.y<<", "<<planSrv.response.path[i].position.z<<", "<<yaw<<"\n";
        ros::Duration(0.5).sleep();
      }
    }
    else
      ROS_WARN("Planner not reachable");
  }
  file.close();
}