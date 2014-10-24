#ifndef CL_DEVICE_H
#define CL_DEVICE_H

#include "clDevice.h"
#include "Cl/OpenCL.h"
#include <string>

namespace Device
{
	enum DeviceType: cl_device_type
	{
		GPU = CL_DEVICE_TYPE_GPU,
		CPU = CL_DEVICE_TYPE_CPU,
		All = CL_DEVICE_TYPE_ALL
	};
};

class clDevice
{
	public:
	clDevice(){};
	clDevice(cl_device_id devID, int platNum, int devNum, std::string platName, std::string devName )
		: deviceID(devID), deviceNum(devNum), platformNum(platNum), platformname(platName), devicename(devName){};

	cl_device_id& GetDeviceID(){ return deviceID; };
	std::string GetDeviceName(){ return devicename; };
	std::string GetPlatformName(){ return platformname; };
	int GetPlatformNumber(){ return platformNum; };
	int GetDeviceNumber(){ return deviceNum; };
	Device::DeviceType GetDeviceType();

private:
	int platformNum;
	int deviceNum;
	std::string platformname;
	std::string devicename;
	cl_device_id deviceID;
};

#endif