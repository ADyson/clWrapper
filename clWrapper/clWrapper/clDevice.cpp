#include "clDevice.h"

DeviceType clDevice::GetDeviceType()
{
	cl_int status;
	DeviceType deviceType;
	status =  clGetDeviceInfo(deviceID, CL_DEVICE_TYPE, sizeof(DeviceType), &deviceType, NULL);
	return deviceType;
};