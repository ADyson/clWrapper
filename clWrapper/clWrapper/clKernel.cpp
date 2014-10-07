#include "clKernel.h"

clEvent clKernel::operator()(clWorkGroup Global)
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
	status = clEnqueueNDRangeKernel(Context->GetQueue(),Kernel,2,NULL,Global.worksize,NULL,eventwaitlist.size(),eventwaitlist.size() ? &eventwaitlist[0] : NULL,&KernelFinished.event);

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