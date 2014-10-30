#ifndef AUTO_H
#define AUTO_H

#include "clEvent.h"
#include "Notify.h"
#include <vector>

// This class can facilitate automatically retrieving changes to OpenCL memory buffers.
// from kernels with argument types specified.
template <class T> class Auto abstract : public Notify
{
public:
	Auto<T>(size_t size): Size(size), isAuto(true){
		Local.resize(0);
	};

	size_t Size;
	bool isAuto;

	// Reading directly from Local is unadvised as it will be updated whenever the kernel 
	// actually completes which is not in general, directly after it has been called as kernel
	// enqueue is non blocking. Use GetLocal() to force waiting for current version.
	std::vector<T> Local;

	virtual clEvent Read(std::vector<T>&data)=0;
	virtual clEvent Read(std::vector<T>&data,clEvent KernelFinished)=0;
	
	virtual clEvent GetStartWriteEvent()=0;
	virtual clEvent GetStartReadEvent()=0;
	virtual clEvent GetFinishedWriteEvent()=0;
	virtual clEvent GetFinishedReadEvent()=0;

	virtual void SetFinishedEvent(clEvent KernelFinished) =0;
	
	// This call will block if the Memory is currently waiting on
	// an event before updating itself.
	std::vector<T>& GetLocal()
	{	
		clEvent e = GetFinishedReadEvent();
		if(e.isSet())
			clWaitForEvents(1,&e.event);
		return Local;
	};

	// Called by clKernel for Output types to generate automatic
	// memory updates (non blocking)
	void Update(clEvent KernelFinished)
	{
		if(Local.empty() == true || Local.size() != Size)
			Local.resize(Size);
		Read(Local,KernelFinished);
	}

	void UpdateEventOnly(clEvent KernelFinished)
	{
		SetFinishedEvent(KernelFinished);
	};
};

#endif