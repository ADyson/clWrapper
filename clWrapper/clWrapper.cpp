#include "clWrapper.h"
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


std::vector<clDevice> OpenCL::GetDeviceList()
{
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

	std::vector<clDevice> DeviceList;

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
			clDevice newDev;

			// get device name
			status =    clGetDeviceInfo(devices[i][j], CL_DEVICE_NAME, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
			status =  clGetDeviceInfo(devices[i][j], CL_DEVICE_NAME, valueSize, value, NULL);
			std::string dName = value;

			newDev.deviceID = devices[i][j];
			newDev.deviceNum = j;
			newDev.platformNum = i;
			//newDev.devicename = std::to_string(i) + ": " + trim(pName) + ", " + std::to_string(j) + ": " + trim(dName);
			newDev.devicename = trim(pName) + ", "  + trim(dName);
			newDev.numDevices = DevPerPlatform[i];
            free(value);

			DeviceList.push_back(newDev);
		}
		free(Pvalue);
	}

	free(platforms);

	// Is passing back a vector of unique ptrs OK, its by value but they cannot be copied..
	return DeviceList;
}

clContext OpenCL::MakeContext(clDevice& dev)
{
	clContext Context;
	cl_int status;

	Context.context = clCreateContext(NULL,1,&dev.deviceID,NULL,NULL,&status);
	Context.queue = clCreateCommandQueue(Context.context,dev.deviceID,0,&status);
	Context.contextdevice = dev;

	if(status!=CL_SUCCESS)
	{
		throw "OpenCL Error setting device/platform";
	}

	return Context;
}

void clContext::WaitForQueueFinish()
{
	clFinish(queue);
}

clKernel clContext::BuildKernelFromString(const char* codestring, std::string kernelname, int NumberOfArgs)
{
	cl_int status;

	clKernel newKernel(*this,NumberOfArgs);

	// denorms now flushed to zero, and no checks for NaNs or infs, should be faster...
	const char options[] = "";// = "-cl-finite-math-only -cl-strict-aliasing -cl-mad-enable -cl-denorms-are-zero";
	
	newKernel.kernelprogram = clCreateProgramWithSource(context,1,&codestring,NULL,&status);

	if(!status==0)
	{
		throw std::exception ("Problem with Kernel Source");
	}

	status = clBuildProgram(newKernel.kernelprogram,1,&contextdevice.deviceID,options,NULL,NULL);

	status = clGetProgramBuildInfo(newKernel.kernelprogram,contextdevice.deviceID, CL_PROGRAM_BUILD_LOG, 0, NULL, &newKernel.log);

	char *buildlog = (char*)malloc(newKernel.log*sizeof(char));
	status = clGetProgramBuildInfo(newKernel.kernelprogram, contextdevice.deviceID, CL_PROGRAM_BUILD_LOG, newKernel.log, buildlog, NULL);

	if(!status==0)
	{
		std::string message = "Problem with Kernel Building";
		throw std::exception (message.c_str());
	}

	free(buildlog);

	newKernel.kernel = clCreateKernel(newKernel.kernelprogram,kernelname.c_str(),&status);

	if(!status==0)
	{
		std::string message = "Problem with Kernel Creation";
		std::string error = message;
		throw std::exception (error.c_str());
	}

	return newKernel;
}

clKernel clContext::BuildKernelFromFile(const char* filename, std::string kernelname, int NumberOfArgs)
{
	clKernel newKernel(*this,NumberOfArgs);
	return newKernel;
}


void clKernel::Enqueue(WorkGroup Global)
{
	status = clEnqueueNDRangeKernel(Context.queue,kernel,2,NULL,Global.worksize,NULL,0,NULL,NULL);

	if(!status==0)
	{
		std::string message = "Problem with Kernel Enqueue";
		std::string error = message;
		throw std::exception (error.c_str());
	}

	RunCallbacks();
}

void clKernel::Enqueue(WorkGroup Global, WorkGroup Local)
{
	status = clEnqueueNDRangeKernel(Context.queue,kernel,2,NULL,Global.worksize,Local.worksize,0,NULL,NULL);

	if(!status==0)
	{
		std::string message = "Problem with Kernel Enqueue";
		std::string error = message;
		throw std::exception (error.c_str());
	}

	RunCallbacks();
}

void clKernel::RunCallbacks()
{
	for( int arg = 0 ; arg < NumberOfArgs ; arg++)
	{
		if(ArgType[arg] == Output || ArgType[arg] == InputOutput)
		{
			Callbacks[arg]->SetChanged();
		}
	}
}