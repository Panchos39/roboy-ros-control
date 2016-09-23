
#ifndef HARDWARE_INTERFACE_JOINT_COMMAND_INTERFACE_H
#define HARDWARE_INTERFACE_JOINT_COMMAND_INTERFACE_H

#include <cassert>
#include <string>
#include <hardware_interface/internal/hardware_resource_manager.h>
#include <hardware_interface/joint_state_interface.h>

class ExtendedJointHandle : public ExtendedJointStateHandle
{
	public:
	ExtendedJointHandle() : ExtendedJointStateHandle() {}

	ExtendedJointHandle(const ExtendedJointStateHandle& js, double* cmd)
		: ExtendedJointStateHandle(js), cmd_(cmd)
	{
		if (!cmd_)
		{
			throw HardwareInterfaceException("Cannot create handle '" + js.getName() + "'. Command data pointer is null.");
		}
	}

	void setCommand(double command) {assert(cmd_); *cmd_ = command;}
	double getCommand() const {assert(cmd_); return *cmd_;}

	private:
	double* cmd_;
};

class ExtendedJointCommandInterface : public HardwareResourceManager<ExtendedJointHandle, ClaimResources> {};

class ExtendedEffortJointInterface : public ExtendedJointCommandInterface {};

class ExtendedVelocityJointInterface : public ExtendedJointCommandInterface {};

class ExtendedPositionJointInterface : public ExtendedJointCommandInterface {};

class ExtendedDisplacementJointInterface : public ExtendedJointCommandInterface {};

#endif