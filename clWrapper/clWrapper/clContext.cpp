#include "clContext.h"

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

	clKernel newKernel(this,NumberOfArgs,Kernel,Program,kernelname,status);
	return newKernel;
}

clFourier clContext::BuildOutOfPlace2DFFT(int width, int height)
{
	clFourier FFT(this);
	FFT.Setup(width,height);
	return FFT;
}