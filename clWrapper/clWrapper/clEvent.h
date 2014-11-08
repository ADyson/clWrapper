#ifndef CL_EVENT_H
#define CL_EVENT_H

#include "CL/Opencl.h"
#include "boost/shared_ptr.hpp"

// Used to synchronise OpenCL functions that depend on other results.
class clEvent
{
	bool hasBeenSet;
public:
	clEvent(): hasBeenSet(false){};
	~clEvent(){ if(isSet()){ clReleaseEvent(event); }};
	cl_event event;
	
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

// Probably need to change to handling events using weak/shared_ptrs and explicitly freeing them when the event has been waited for?
// Need to consider what happens when a kernel is used as output then as input output and and even is passed for each one, if it is accessed between the kernels
// then it should only have one event and start blocking until it is read. If it is afterwards, the second event will have
// replaced the first and access will wait for the second event only. Currently events are copied and never released.
// Destructor would destroy events when class is copied and create problems. Explicit release wouldnt be called when an event is assigned to a new event. 

// Shared ptr to events would allow it to be released as soon as no one was still holding reference to it. So if it is cleared after being waited for.


// Is there another way? Typical scenario, event is created upon enqueue of kernel, and then passed to every argument so it knows to wait before retrieving values after this enqueue.
// Now 2 options, argument requests memory or it doesnt and the buffer is used again.
// Option 1 we wait for event and then get memory - event is now over after we have waited, but this event exists in every argument, if we retrieved 2 buffers it can only be destroyed after the second one.
// Or if tthey only had referenced to the event the first one could free it, and then unset it, because its happened anyway, and when every shared_ptr goes the object will be destroyed aswell.
// If a event got passed to a buffer that was never accessed it would be kept until program termination, but if ever checked it would have been unset anyway...

// PLAN, change clEvents to shared_ptr<clEvent> unset after Wait(). Ensure wait() is used everywhere, and set,unset is checked and also set properly. destructor can call release also if its still set.
