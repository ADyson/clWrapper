#ifndef CL_EVENT_H
#define CL_EVENT_H

#include "clWrapper.h"

// Used to synchronise OpenCL functions that depend on other results.
class clEvent
{
	bool hasBeenSet;
public:
	clEvent(): hasBeenSet(false){};
	cl_event event;
	
	bool isSet(){ return hasBeenSet;};
	void Set(){ hasBeenSet = true; };
	// If profiling is enable can use these functions
	cl_ulong GetStartTime(){cl_ulong param; clGetEventProfilingInfo(event,CL_PROFILING_COMMAND_START,sizeof(cl_ulong),&param,NULL); return param;};
	cl_ulong GetEndTime(){cl_ulong param; clWaitForEvents(1,&event); clGetEventProfilingInfo(event,CL_PROFILING_COMMAND_END,sizeof(cl_ulong),&param,NULL); return param;};
	cl_ulong GetElapsedTime(){return GetEndTime() - GetStartTime();};


};

#endif