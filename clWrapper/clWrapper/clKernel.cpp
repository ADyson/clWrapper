#include "clKernel.h"

clEvent clKernel::operator()(clWorkGroup Global)
{
	std::vector<cl_event> eventwaitlist;

	// Check callbacks for any input types... need to wait on there write events..
	for( int arg = 0 ; arg < NumberOfArgs ; arg++)
	{
		if(ArgType[arg] == ArgumentType::Input || ArgType[arg] == ArgumentType::InputOutput)
		{
			clEvent e = Callbacks[arg]->GetFinishedWriteEvent();
			if (e.isSet())
			{
				eventwaitlist.push_back(e.event);
			}
		}
	}

	clEvent KernelFinished;
	status = clEnqueueNDRangeKernel(Context->GetQueue(),Kernel,2,NULL,Global.worksize,NULL,eventwaitlist.size(),eventwaitlist.size() ? &eventwaitlist[0] : NULL,&KernelFinished.event);
	KernelFinished.Set();

	if(!status==0)
	{
		std::string message = "Problem with Kernel Enqueue";
		std::string error = message;
		throw std::exception (error.c_str());
	}

	RunCallbacks(KernelFinished);

	return KernelFinished;
}

clEvent clKernel::operator()(clWorkGroup Global, clEvent StartEvent)
{
	std::vector<cl_event> eventwaitlist;

	// Check callbacks for any input types... need to wait on there write events..
	for( int arg = 0 ; arg < NumberOfArgs ; arg++)
	{
		if(ArgType[arg] == ArgumentType::Input || ArgType[arg] == ArgumentType::InputOutput)
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
	status = clEnqueueNDRangeKernel(Context->GetQueue(),Kernel,2,NULL,Global.worksize,NULL,eventwaitlist.size(),&eventwaitlist[0],&KernelFinished.event);

	if(!status==0)
	{
		std::string message = "Problem with Kernel Enqueue";
		std::string error = message;
		throw std::exception (error.c_str());
	}

	RunCallbacks(KernelFinished);

	return KernelFinished;
}

clEvent clKernel::operator()(clWorkGroup Global, clWorkGroup Local)
{
	std::vector<cl_event> eventwaitlist;

	// Check callbacks for any input types... need to wait on there write events..
	for( int arg = 0 ; arg < NumberOfArgs ; arg++)
	{
		if(ArgType[arg] == ArgumentType::Input || ArgType[arg] == ArgumentType::InputOutput)
		{
			cl_event e = Callbacks[arg]->GetFinishedWriteEvent().event;
			if (e!=NULL)
			{
				eventwaitlist.push_back(e);
			}
		}
	}

	clEvent KernelFinished;
	status = clEnqueueNDRangeKernel(Context->GetQueue(),Kernel,2,NULL,Global.worksize,Local.worksize,eventwaitlist.size(),eventwaitlist.size() ? &eventwaitlist[0] : NULL,&KernelFinished.event);

	if(!status==0)
	{
		std::string message = "Problem with Kernel Enqueue";
		std::string error = message;
		throw std::exception (error.c_str());
	}

	RunCallbacks(KernelFinished);

	return KernelFinished;
}

clEvent clKernel::operator()(clWorkGroup Global, clWorkGroup Local, clEvent StartEvent)
{
	std::vector<cl_event> eventwaitlist;

	// Check callbacks for any input types... need to wait on there write events..
	for( int arg = 0 ; arg < NumberOfArgs ; arg++)
	{
		if(ArgType[arg] == ArgumentType::Input || ArgType[arg] == ArgumentType::InputOutput)
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
	status = clEnqueueNDRangeKernel(Context->GetQueue(),Kernel,2,NULL,Global.worksize,Local.worksize,eventwaitlist.size(),&eventwaitlist[0],&KernelFinished.event);

	if(!status==0)
	{
		std::string message = "Problem with Kernel Enqueue";
		std::string error = message;
		throw std::exception (error.c_str());
	}

	RunCallbacks(KernelFinished);

	return KernelFinished;
}

void clKernel::RunCallbacks(clEvent KernelFinished)
{
	for( int arg = 0 ; arg < NumberOfArgs ; arg++)
	{
		if(ArgType[arg] == ArgumentType::Output || ArgType[arg] == ArgumentType::InputOutput)
		{
			Callbacks[arg]->Update(KernelFinished);
		}
	}
}

void clKernel::BuildKernelFromString(const char* codestring, std::string kernelname, int NumberOfArgs)
{
	// denorms now flushed to zero, and no checks for NaNs or infs, should be faster...
	const char options[] = "";// = "-cl-finite-math-only -cl-strict-aliasing -cl-mad-enable -cl-denorms-are-zero";
	size_t log;
	Program = clCreateProgramWithSource(Context->GetContext(),1,&codestring,NULL,&status);

	if(!status==0)
	{
		throw clKernel::BuildException("Problem with Kernel Source",status);
	}

	status = clBuildProgram(Program,1,&Context->GetContextDevice().GetDeviceID(),options,NULL,NULL);
	status = clGetProgramBuildInfo(Program,Context->GetContextDevice().GetDeviceID(), CL_PROGRAM_BUILD_LOG, 0, NULL, &log);

	char *buildlog = (char*)malloc(log*sizeof(char));
	status = clGetProgramBuildInfo(Program, Context->GetContextDevice().GetDeviceID(), CL_PROGRAM_BUILD_LOG, log, buildlog, NULL);

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
}
