// Copyright 2021 ROScon de Reyes
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>
#include <vector>

#include <ros/ros.h>
#include <ros/console.h>

#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/point_types_conversion.h>

#include <tf/transform_broadcaster.h>
#include <tf/transform_listener.h>

#include <std_msgs/Float32.h>
#include <sensor_msgs/PointCloud2.h>

#include <boost/algorithm/string.hpp>
#include <pcl_ros/transforms.h>

#include "darknet_ros_msgs/BoundingBoxes.h"

#include "move_base_msgs/MoveBaseAction.h"
#include "actionlib/client/simple_action_client.h"

class RGBDFilter
{
public:
  RGBDFilter()
  {
    cloud_sub_ = nh_.subscribe("/camera/depth/points", 1, &RGBDFilter::cloudCB, this);
    object_sub_ = nh_.subscribe("/detected", 1, &RGBDFilter::calculatePoint2D, this);
  }

  void cloudCB(const sensor_msgs::PointCloud2::ConstPtr& cloud_in)
  {
    double x, y, z;
    sensor_msgs::PointCloud2 cloud;

    try
    {
      pcl_ros::transformPointCloud("camera_link", *cloud_in, cloud, tfListener_);
    }
    catch(tf::TransformException & ex)
    {
      ROS_ERROR_STREAM("Transform error of sensor data: " << ex.what() << ", quitting callback");
      return;
    }

    pcl::PointCloud<pcl::PointXYZRGB>::Ptr pcrgb(new pcl::PointCloud<pcl::PointXYZRGB>);
    pcl::fromROSMsg(cloud, *pcrgb);

    auto point3d = pcrgb->at(coor2dx_, coor2dy_);
    if (!(std::isnan(point3d.x) || std::isnan(point3d.y) || std::isnan(point3d.z)))
    {
      coor3dx_ = point3d.x;
      coor3dy_ = point3d.y;
      coor3dz_ = point3d.z;
    }

    tf::StampedTransform transform;
    transform.setOrigin(tf::Vector3(coor3dx_, coor3dy_, coor3dz_));
    transform.setRotation(tf::Quaternion(0.0, 0.0, 0.0, 1.0));

    transform.stamp_ = ros::Time::now();
    transform.frame_id_ = "/base_footprint";
    transform.child_frame_id_ = object_;

    try
    {
      tfBroadcaster_.sendTransform(transform);
    }
    catch(tf::TransformException& ex)
    {
      ROS_ERROR_STREAM("Transform error of sensor data: " << ex.what() << ", quitting callback");
      return;
    }
  }

  void calculatePoint2D(const darknet_ros_msgs::BoundingBox::ConstPtr& objmsg)
  {
    coor2dx_ = (objmsg->xmin + objmsg->xmax)/2;
    coor2dy_ = (objmsg->ymin + objmsg->ymax)/2;
    object_ = objmsg->Class;
    std::cout << "(" << coor2dx_ << "," << coor2dy_ << ")" << std::endl;
  }

private:
  ros::NodeHandle nh_;

  ros::Subscriber cloud_sub_;
  ros::Subscriber object_sub_;

  tf::TransformBroadcaster tfBroadcaster_;
  tf::TransformListener tfListener_;

  std::string object_;

  float coor2dx_;
  float coor2dy_;
  float coor3dx_;
  float coor3dy_;
  float coor3dz_;
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "rgbd_center");
  RGBDFilter rf;

  ros::spin();
  return 0;
}
