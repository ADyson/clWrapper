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

	// Is passing back a vector of unique ptrs OK, its by value but they cannot be copied..
	return DeviceList;
}

clContext OpenCL::MakeContext(clDevice& dev)
{
	cl_int status;
	cl_context ctx = clCreateContext(NULL,1,&dev.GetDeviceID(),NULL,NULL,&status);
	cl_command_queue q = clCreateCommandQueue(ctx,dev.GetDeviceID(),CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,&status);

	clContext Context(dev,ctx,q);

	if(status!=CL_SUCCESS)
	{
		throw "OpenCL Error setting device/platform";
	}

	return Context;
}

clDoubleQueueContext OpenCL::MakeDoubleQueueContext(clDevice& dev)
{
	cl_int status;
	cl_context ctx = clCreateContext(NULL,1,&dev.GetDeviceID(),NULL,NULL,&status);
	cl_command_queue q = clCreateCommandQueue(ctx,dev.GetDeviceID(),CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,&status);
	cl_command_queue IOq = clCreateCommandQueue(ctx,dev.GetDeviceID(),CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,&status);
	
	clDoubleQueueContext Context(dev,ctx,q,IOq);

	if(status!=CL_SUCCESS)
	{
		throw "OpenCL Error setting device/platform";
	}

	return Context;
}

void clContext::WaitForQueueFinish()
{
	clFinish(Queue);
}

void clContext::QueueFlush()
{
	clFlush(Queue);
}

void clDoubleQueueContext::WaitForIOQueueFinish()
{
	clFinish(IOQueue);
}

void clDoubleQueueContext::IOQueueFlush()
{
	clFlush(IOQueue);
}

clKernel clContext::BuildKernelFromString(const char* codestring, std::string kernelname, int NumberOfArgs)
{
	cl_int status;
	size_t log;
	cl_program Program;
	cl_kernel Kernel;

	// denorms now flushed to zero, and no checks for NaNs or infs, should be faster...
	const char options[] = "";// = "-cl-finite-math-only -cl-strict-aliasing -cl-mad-enable -cl-denorms-are-zero";
	
	Program = clCreateProgramWithSource(Context,1,&codestring,NULL,&status);

	if(!status==0)
	{
		throw clKernel::BuildException("Problem with Kernel Source",status);
	}

	status = clBuildProgram(Program,1,&ContextDevice.GetDeviceID(),options,NULL,NULL);
	status = clGetProgramBuildInfo(Program,ContextDevice.GetDeviceID(), CL_PROGRAM_BUILD_LOG, 0, NULL, &log);

	char *buildlog = (char*)malloc(log*sizeof(char));
	status = clGetProgramBuildInfo(Program, ContextDevice.GetDeviceID(), CL_PROGRAM_BUILD_LOG, log, buildlog, NULL);

	if(!status==0)
	{
		std::string message = "Problem with Kernel Building: " + std::string(buildlog);
		throw clKernel::BuildException(message,status);
	}

	Kernel = clCreateKernel(Program,kernelname.c_str(),&status);

	if(!status==0)
	{
		std::string message = "Problem with Kernel Creation: ";
		std::string logstr = std::string(buildlog);
		throw clKernel::BuildException(message+logstr,status);
	}

	free(buildlog);

	clKernel newKernel(*this,NumberOfArgs,Kernel,Program,kernelname);
	return newKernel;
}

// Not implemented yet
/* clKernel clContext::BuildKernelFromFile(const char* filename, std::string kernelname, int NumberOfArgs)
{
	clKernel newKernel(*this,NumberOfArgs);
	return newKernel;
} 
*/


//void clKernel::Enqueue(WorkGroup Global)
//{
//	status = clEnqueueNDRangeKernel(Context.Queue,Kernel,2,NULL,Global.worksize,NULL,0,NULL,NULL);
//
//	if(!status==0)
//	{
//		std::string message = "Problem with Kernel Enqueue";
//		std::string error = message;
//		throw std::exception (error.c_str());
//	}
//
//	RunCallbacks();
//}
//
//void clKernel::Enqueue(WorkGroup Global, WorkGroup Local)
//{
//	status = clEnqueueNDRangeKernel(Context.Queue,Kernel,2,NULL,Global.worksize,Local.worksize,0,NULL,NULL);
//
//	if(!status==0)
//	{
//		std::string message = "Problem with Kernel Enqueue";
//		std::string error = message;
//		throw std::exception (error.c_str());
//	}
//
//	RunCallbacks();
//}

clEvent clKernel::operator()(WorkGroup Global)
{
	std::vector<cl_event> eventwaitlist;

	// Check callbacks for any input types... need to wait on there write events..
	for( int arg = 0 ; arg < NumberOfArgs ; arg++)
	{
		if(ArgType[arg] == Input || ArgType[arg] == InputOutput)
		{
			cl_event e = Callbacks[arg]->GetFinishedWriteEvent().event;
			if (e!=NULL)
			{
				eventwaitlist.push_back(e);
			}
		}
	}

	clEvent KernelFinished;
	status = clEnqueueNDRangeKernel(Context.Queue,Kernel,2,NULL,Global.worksize,NULL,eventwaitlist.size(),&eventwaitlist[0],&KernelFinished.event);

	if(!status==0)
	{
		std::string message = "Problem with Kernel Enqueue";
		std::string error = message;
		throw std::exception (error.c_str());
	}

	RunCallbacks(KernelFinished);

	return KernelFinished;
}

clEvent clKernel::operator()(WorkGroup Global, clEvent StartEvent)
{
	std::vector<cl_event> eventwaitlist;

	// Check callbacks for any input types... need to wait on there write events..
	for( int arg = 0 ; arg < NumberOfArgs ; arg++)
	{
		if(ArgType[arg] == Input || ArgType[arg] == InputOutput)
		{
			cl_event e = Callbacks[arg]->GetFinishedWriteEvent().event;
			if (e!=NULL)
			{
				eventwaitlist.push_back(e);
			}
		}
	}

	eventwaitlist.push_back(StartEvent.event);

	clEvent KernelFinished;
	status = clEnqueueNDRangeKernel(Context.Queue,Kernel,2,NULL,Global.worksize,NULL,eventwaitlist.size(),&eventwaitlist[0],&KernelFinished.event);

	if(!status==0)
	{
		std::string message = "Problem with Kernel Enqueue";
		std::string error = message;
		throw std::exception (error.c_str());
	}

	RunCallbacks(KernelFinished);

	return KernelFinished;
}

//void clKernel::operator()(WorkGroup Global, WorkGroup Local)
//{
//	status = clEnqueueNDRangeKernel(Context.Queue,Kernel,2,NULL,Global.worksize,Local.worksize,0,NULL,NULL);
//
//	if(!status==0)
//	{
//		std::string message = "Problem with Kernel Enqueue";
//		std::string error = message;
//		throw std::exception (error.c_str());
//	}
//
//	RunCallbacks();
//}

void clKernel::RunCallbacks(clEvent KernelFinished)
{
	for( int arg = 0 ; arg < NumberOfArgs ; arg++)
	{
		if(ArgType[arg] == Output || ArgType[arg] == InputOutput)
		{
			Callbacks[arg]->Update(KernelFinished);
		}
	}
}


