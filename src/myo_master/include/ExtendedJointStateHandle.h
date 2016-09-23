#ifndef HARDWARE_INTERFACE_JOINT_DISPLACEMENT_H
#define HARDWARE_INTERFACE_JOINT_DISPLACEMENT_H

#include <hardware_interface/internal/hardware_resource_manager.h>
#include <cassert>
#include <string>


class JointDisplacementHandle : public hardware_interface::JointStateHandle {
public:

	JointDisplacementHandle (const std::string& name, const double* pos, const double* vel, const double* eff,const double* tendDisp)
    : name_(name), pos_(pos), vel_(vel), eff_(eff), tendDisp_(tendDisp)
  {
     if (!pos)
     {
       throw HardwareInterfaceException("Cannot create handle '" + name + "'. Position data pointer is null.");
   }
    if (!vel)
    {
       throw HardwareInterfaceException("Cannot create handle '" + name + "'. Velocity data pointer is null.");
   }
    if (!eff)
    {
      throw HardwareInterfaceException("Cannot create handle '" + name + "'. Effort data pointer is null.");
    }
      if (!tendDisp)
    {
      throw HardwareInterfaceException("Cannot create handle '" + name + "'. Tendon Displacement data pointer is null.");
    }
 }

double getTendDisp()	const {assert (tendDisp_); return *tendDisp_;}

private:
	const double* tendDisp_;	
};

class JointDisplacementInterface : public HardwareResourceManager<JointDisplacementHandle> {};

#endif