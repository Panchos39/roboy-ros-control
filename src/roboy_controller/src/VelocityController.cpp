#include <controller_interface/controller.h>
#include <hardware_interface/joint_command_interface.h>
#include <pluginlib/class_list_macros.h>
#include "std_msgs/Float32.h"
#include <common_utilities/Status.h>
#include <common_utilities/Steer.h>
#include <common_utilities/Trajectory.h>
#include <CommonDefinitions.h>
#include "timer.hpp"

using namespace std;

class VelocityController : public controller_interface::Controller<hardware_interface::VelocityJointInterface>
{
public:
	VelocityController(){
		trajectory.push_back(0);
	};

	bool init(hardware_interface::VelocityJointInterface* hw, ros::NodeHandle &n)
	{
		// get joint name from the parameter server
		if (!n.getParam("joint_name", joint_name)){
			ROS_FATAL("Could not find joint name");
			myStatus = UNDEFINED;
			return false;
		}
		joint_ = hw->getHandle(joint_name);  // throws on failure
		trajectory_srv = n.advertiseService("/roboy/trajectory_"+joint_name, &VelocityController::trajectoryPreprocess, this);
		steer_sub = n.subscribe("/roboy/steer",1000, &VelocityController::steer, this);
		status_pub = n.advertise<common_utilities::Status>("/roboy/status_"+joint_name, 1);
		trajectory_pub = n.advertise<std_msgs::Float32>("/roboy/trajectory_"+joint_name+"/plot",1);
		myStatus = INITIALIZED;
		ROS_DEBUG("PositionController for %s initialized", joint_name.c_str());
		return true;
	}

	void update(const ros::Time& time, const ros::Duration& period)
	{
		float pos = joint_.getVelocity();
		msg.data = pos;
		trajectory_pub.publish(msg);

		if(steered == PLAY_TRAJECTORY) {
			dt.push_back(timer.elapsedTime());
			positions.push_back(pos);

			if (fabs(pos - trajectory[trajpos]) < 0.2 && trajpos < trajectory.size() - 1) {
				myStatus = TRAJECTORY_PLAYING;
				trajpos++;
				setpoint_ = trajectory[trajpos];
			}

			if (trajpos == trajectory.size() - 1) {
				ROS_DEBUG_THROTTLE(1, "%s update, current pos: %f, reached endpoint of trajectory %f",
								   joint_name.c_str(), pos, trajectory[trajpos]);
				setpoint_ = trajectory[trajpos];
				myStatus = TRAJECTORY_DONE;
				steered = STOP_TRAJECTORY;
			} else {
				ROS_DEBUG_THROTTLE(1, "%s update, current pos: %f, setpoint: %f", joint_name.c_str(), pos,
								   setpoint_);
			}
			joint_.setCommand(setpoint_);
		}
	}

	void steer(const common_utilities::Steer::ConstPtr& msg){
		switch (msg->steeringCommand){
			case STOP_TRAJECTORY:
				steered = STOP_TRAJECTORY;
				trajpos = 0;
				myStatus = TRAJECTORY_READY;
				break;
			case PLAY_TRAJECTORY:
				steered = PLAY_TRAJECTORY;
				break;
			case PAUSE_TRAJECTORY:
				steered = PAUSE_TRAJECTORY;
				myStatus = TRAJECTORY_READY;
				break;
			case REWIND_TRAJECTORY:
				steered = PLAY_TRAJECTORY;
				trajpos = 0;
				myStatus = TRAJECTORY_READY;
				break;
		}
	}

	void starting(const ros::Time& time) { ROS_DEBUG("starting velocity controller for %s, gain: %f, setpoint: %f", joint_name.c_str(),gain_,setpoint_);}
	void stopping(const ros::Time& time) { ROS_DEBUG("stopping velocity controller for %s", joint_name.c_str());}

private:
	hardware_interface::JointHandle joint_;
	double gain_ = 1.25; // not used
	double setpoint_ = 0;
	std::string joint_name;
	ros::NodeHandle n;
	ros::ServiceServer trajectory_srv;
	ros::Subscriber steer_sub;
	ros::Publisher trajectory_pub, status_pub;
	std::vector<float> trajectory;
	uint trajpos = 0;
	int8_t myStatus = UNDEFINED;
	int8_t steered = STOP_TRAJECTORY;
	vector<float> dt;
	vector<float> positions;
	std_msgs::Float32 msg;
	Timer timer;
	bool trajectoryPreprocess(common_utilities::Trajectory::Request& req,
							  common_utilities::Trajectory::Response& res){
		steered = STOP_TRAJECTORY;
		myStatus = PREPROCESS_TRAJECTORY;

		ROS_DEBUG("New trajectory [%d elements] at sampleRate %d",
				  (int)req.waypoints.size(), req.samplerate);
		if(!req.waypoints.empty()) {
			trajectory = req.waypoints;
			trajpos = 0;
			myStatus = TRAJECTORY_READY;
			timer.start();
			res.state.state = TRAJECTORY_READY;
			return true;
		}else{
			myStatus = TRAJECTORY_FAILED;
			res.state.state = TRAJECTORY_FAILED;
			return false;
		}
	}
};
PLUGINLIB_EXPORT_CLASS(VelocityController, controller_interface::ControllerBase);

