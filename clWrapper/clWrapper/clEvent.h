#ifndef CL_EVENT_H
#define CL_EVENT_H

#include "CL/Opencl.h"
#include "boost/shared_ptr.hpp"

// Used to synchronise OpenCL functions that depend on other results.
class clEvent
{
private:
	bool hasBeenSet;
	cl_event event;
public:

	clEvent(): hasBeenSet(false){};
	~clEvent(){ if(isSet()){ clReleaseEvent(event); }};
	
	cl_event& GetEvent(){ return event; };
	bool isSet(){ return hasBeenSet;};
	void Set(){ hasBeenSet = true; };
	void Wait(){ clWaitForEvents(1,&event);};
	// If profiling is enable can use these functions
	cl_ulong GetStartTime(){cl_ulong param; clGetEventProfilingInfo(event,CL_PROFILING_COMMAND_START,sizeof(cl_ulong),&param,NULL); return param;};
	cl_ulong GetEndTime(){cl_ulong param; clWaitForEvents(1,&event); clGetEventProfilingInfo(event,CL_PROFILING_COMMAND_END,sizeof(cl_ulong),&param,NULL); return param;};
	cl_ulong GetElapsedTime(){return GetEndTime() - GetStartTime();};

	// Perhaps should not allow copying/or assignment.
};

typedef boost::shared_ptr<clEvent> clEventPtr;

#endif
