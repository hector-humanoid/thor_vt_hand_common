#include <math.h>
#include <ros/ros.h>
#include <ros/subscribe_options.h>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <sensor_msgs/JointState.h>
#include <vigir_grasp_msgs/JointCommands.h>


ros::Publisher pub_joint_commands_;
vigir_grasp_msgs::JointCommands jointcommands;

void SetJointStates(const sensor_msgs::JointState::ConstPtr &_js)
{
  float key;
    //do {
       //key = getchar(); /* consume the character */
printf ("Enter a position (in rad):\n");
scanf ("%f",&key);
        printf("%f\n", key);
    //} while(key != 113);

  static ros::Time startTime = ros::Time::now();

    // for testing round trip time
    jointcommands.header.stamp = _js->header.stamp;

    // assign sinusoidal joint angle targets
    for (unsigned int i = 0; i < jointcommands.name.size(); i++)
      jointcommands.position[i] = key;

    pub_joint_commands_.publish(jointcommands);
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "pub_joint_command_test");

  ros::NodeHandle* rosnode = new ros::NodeHandle();

  // Waits for simulation time update.
  ros::Time last_ros_time_;
  bool wait = true;
  while (wait)
  {
    last_ros_time_ = ros::Time::now();
    if (last_ros_time_.toSec() > 0)
      wait = false;
  }

  jointcommands.name.push_back("r_f0_j0");
  jointcommands.name.push_back("r_f1_j0");
  jointcommands.name.push_back("r_f0_j1");
  jointcommands.name.push_back("r_f1_j1");

  unsigned int n = jointcommands.name.size();
  jointcommands.position.resize(n);
  jointcommands.velocity.resize(n);
  jointcommands.effort.resize(n);
  jointcommands.kp_position.resize(n);
  jointcommands.ki_position.resize(n);
  jointcommands.kd_position.resize(n);
  jointcommands.kp_velocity.resize(n);
  jointcommands.i_effort_min.resize(n);
  jointcommands.i_effort_max.resize(n);

  for (unsigned int i = 0; i < n; i++)
  {
    std::vector<std::string> pieces;
    boost::split(pieces, jointcommands.name[i], boost::is_any_of(":"));

  if (i < 2)
  {
    jointcommands.kp_position[i]  = 1;
    jointcommands.ki_position[i]  = 0;
    jointcommands.kd_position[i]  = 0.5;
  }
  else
  {
    jointcommands.kp_position[i]  = 0.01;
    jointcommands.ki_position[i]  = 0;
    jointcommands.kd_position[i]  = 0.005;
  }
    jointcommands.i_effort_min[i] = 0;
    jointcommands.i_effort_max[i] = 0;
    jointcommands.velocity[i]     = 0;
    jointcommands.effort[i]       = 0;
    jointcommands.kp_velocity[i]  = 0;
  }

  // ros topic subscriptions
  ros::SubscribeOptions jointStatesSo =
    ros::SubscribeOptions::create<sensor_msgs::JointState>(
    "thor_vt_hands/r_hand/joint_states", 1, SetJointStates,
    ros::VoidPtr(), rosnode->getCallbackQueue());

  // Because TCP causes bursty communication with high jitter,
  // declare a preference on UDP connections for receiving
  // joint states, which we want to get at a high rate.
  // Note that we'll still accept TCP connections for this topic
  // (e.g., from rospy nodes, which don't support UDP);
  // we just prefer UDP.
  jointStatesSo.transport_hints = ros::TransportHints().unreliable();

  ros::Subscriber subJointStates = rosnode->subscribe(jointStatesSo);
  // ros::Subscriber subJointStates =
  //   rosnode->subscribe("/thor_mang/joint_states", 1000, SetJointStates);

  pub_joint_commands_ =
    rosnode->advertise<vigir_grasp_msgs::JointCommands>(
    "thor_vt_hands/r_hand/joint_commands", 1, true);

  ros::spin();

  return 0;
}
