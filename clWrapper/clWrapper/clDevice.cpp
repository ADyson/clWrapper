#include "clDevice.h"

Device::DeviceType clDevice::GetDeviceType()
{
	cl_int status;
	Device::DeviceType deviceType;
	status =  clGetDeviceInfo(deviceID, CL_DEVICE_TYPE, sizeof(Device::DeviceType), &deviceType, NULL);
	return deviceType;
};