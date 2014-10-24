#include "clStatic.h"
#include <iostream>

#include "Misc.h"


std::list<clDevice> OpenCL::GetDeviceList()
{
	std::list<clDevice> DeviceList;

	size_t valueSize;
	char* value;
	char* Pvalue;

	//Setup OpenCL
	cl_int status;
	cl_uint numPlatforms = 0;
	cl_platform_id * platforms = NULL;

	   // get all platforms
    clGetPlatformIDs(0, NULL, &numPlatforms);
    platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * numPlatforms);
    clGetPlatformIDs(numPlatforms, platforms, NULL);

	std::vector<cl_uint> DevPerPlatform;
	std::vector<cl_device_id*> devices;

    for (int i = 0; i < numPlatforms; i++) 
	{
		DevPerPlatform.push_back(0);
		devices.push_back(NULL);
       
		// get all devices
        status = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &DevPerPlatform[i]);
        devices[i] = (cl_device_id*) malloc(sizeof(cl_device_id) * DevPerPlatform[i]);
		status = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, DevPerPlatform[i], devices[i], NULL);
		status =	clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, NULL, &valueSize);
		Pvalue = (char*) malloc(valueSize);
		status =  clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, valueSize, Pvalue, NULL);
		std::string pName = Pvalue;

        // for each device get and store name, platform, and device number
        for (int j = 0; j < DevPerPlatform[i]; j++) 
		{
			// get device name
			status =    clGetDeviceInfo(devices[i][j], CL_DEVICE_NAME, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
			status =  clGetDeviceInfo(devices[i][j], CL_DEVICE_NAME, valueSize, value, NULL);
			std::string dName = value;
			free(value);
			
			clDevice newDev(devices[i][j],i,j,pName,Misc::Trim(dName));         
			DeviceList.push_back(newDev);
		}
		free(Pvalue);
	}

	free(platforms);

	return DeviceList;
};

clContext OpenCL::MakeContext(clDevice& dev, Queue::QueueType Qtype)
{
	cl_int status;
	cl_context ctx = clCreateContext(NULL,1,&dev.GetDeviceID(),NULL,NULL,&status);
	cl_command_queue q = clCreateCommandQueue(ctx,dev.GetDeviceID(),Qtype,&status);

	clContext Context(dev,ctx,q,status);
	return Context;
}

clContext OpenCL::MakeContext(std::list<clDevice> &devices, Queue::QueueType Qtype, Device::DeviceType devType)
{
	std::list<clDevice>::iterator it =  devices.begin();
	clDevice dev;

	bool found = false;

	for(int i = 1; i <= devices.size() && !found; i++)
	{
		if((*it).GetDeviceType() == devType)
		{
			dev = *it;
			found = true;
		}
		++it;
	}
	
	if(!found)
	{
		throw "No suitable device";
	}

	cl_int status;
	cl_context ctx = clCreateContext(NULL,1,&dev.GetDeviceID(),NULL,NULL,&status);
	cl_command_queue q = clCreateCommandQueue(ctx,dev.GetDeviceID(),Qtype,&status);

	clContext Context(dev,ctx,q,status);
	return Context;
}