#ifndef CL_DEVICE_H
#define CL_DEVICE_H


class clDevice
{
	public:
	clDevice(){};
	clDevice(cl_device_id devID, int platNum, int devNum, std::string platName, std::string devName )
		: deviceID(devID), deviceNum(devNum), platformNum(platNum), platformname(platName), devicename(devName){};

	cl_device_id& GetDeviceID(){ return deviceID; };
	std::string GetDeviceName(){ return devicename; };
	std::string GetPlatformName(){ return platformname; };

private:
	int platformNum;
	int deviceNum;
	std::string platformname;
	std::string devicename;
	cl_device_id deviceID;
};


#endif