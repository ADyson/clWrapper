#include "clStatic.h"
#include <iostream>

std::string trim(const std::string& str,
                 const std::string& whitespace = " \t")
{
    const int strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const int strEnd = str.find_last_not_of(whitespace);
    const int strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}


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
			
			clDevice newDev(devices[i][j],i,j,pName,trim(dName));         
			DeviceList.push_back(newDev);
		}
		free(Pvalue);
	}

	free(platforms);

	return DeviceList;
};

clContext OpenCL::MakeContext(clDevice& dev, QueueType Qtype)
{
	cl_int status;
	cl_context ctx = clCreateContext(NULL,1,&dev.GetDeviceID(),NULL,NULL,&status);
	cl_command_queue q = clCreateCommandQueue(ctx,dev.GetDeviceID(),Qtype,&status);

	clContext Context(dev,ctx,q,status);
	return Context;
}