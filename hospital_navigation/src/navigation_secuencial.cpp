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

#include "ros/ros.h"
#include "move_base_msgs/MoveBaseAction.h"
#include "actionlib/client/simple_action_client.h"

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

void set_goal(move_base_msgs::MoveBaseGoal& goal, double x, double y)
{
    
    goal.target_pose.pose.position.x = x;
    goal.target_pose.pose.position.y = y;
    goal.target_pose.pose.orientation.w = 0.0013;

}

void doneCb(const actionlib::SimpleClientGoalState& state,
			const move_base_msgs::MoveBaseResultConstPtr& result)
{
    ROS_INFO("Finished in state [%s]", state.toString().c_str());
}


int main(int argc, char** argv){
    int states = 0;
    int num_points = 4;
    ros::init(argc, argv, "navigation");

    // ----------- 4 DIFFERENT MAP POINTS ------------ //

    float habitacion1[2] = {-6.13, 8.2};
    float recepcion[2] = {0.0, 0.0};
    float consulta1[2] = {-5.37, 1.13};
    float consulta2[2] = {-5.32, -1.37};

    // ----------------------------------------------- //

    move_base_msgs::MoveBaseGoal goal;

    MoveBaseClient ac("move_base", true);


    while(!ac.waitForServer(ros::Duration(5.0))){

        ROS_INFO("Waiting for the move_base action server to come up");

    }

    goal.target_pose.header.frame_id = "map";

    while (states < num_points)
    {
        if (states == 0)
        {
            set_goal(goal, habitacion1[0], habitacion1[1]);
        }
        else if (states == 1)
        {
            set_goal(goal, recepcion[0], recepcion[1]);
        }
        else if (states == 2)
        {
            set_goal(goal, consulta1[0], consulta1[1]);
        }
        else if (states == 3)
        {
            set_goal(goal, consulta2[0], consulta2[1]);
        }

        goal.target_pose.header.stamp = ros::Time::now();
        ROS_INFO("Sending goal");
        ac.sendGoal(goal, doneCb);

        ac.waitForResult();

        if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
        {
            ROS_INFO("Hooray, mission accomplished");
            states++;
        }
        else
        {
            ROS_INFO("[Error] mission could not be accomplished");
        }
    }

    return 0;

}