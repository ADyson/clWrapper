#ifndef AUTO_H
#define AUTO_H

#include "clEvent.h"
#include "Notify.h"
#include <vector>

// This class can facilitate automatically retrieving changes to OpenCL memory buffers.
// from kernels with argument types specified.
template <class T> class Auto abstract : public Notify
{
private:
	size_t Size;
	bool isAuto;
	bool isUpToDate;

	// Reading directly from Local is unadvised as it will be updated whenever the kernel 
	// actually completes which is not in general, directly after it has been called as kernel
	// enqueue is non blocking. Use GetLocal() to force waiting for current version.
	std::vector<T> Local;

public:
	Auto<T>(size_t size): Size(size), isAuto(true), isUpToDate(true){
		Local.resize(0);
	};

	virtual clEventPtr Read(std::vector<T>&data)=0;
	virtual clEventPtr Read(std::vector<T>&data,clEventPtr KernelFinished)=0;
	
	virtual clEventPtr GetStartWriteEvent()=0;
	virtual clEventPtr GetStartReadEvent()=0;
	virtual clEventPtr GetFinishedWriteEvent()=0;
	virtual clEventPtr GetFinishedReadEvent()=0;

	virtual void SetFinishedEvent(clEventPtr KernelFinished) =0;

	bool IsAuto(){ return isAuto; };
	
	// This call will block if the Memory is currently waiting on
	// an event before updating itself.
	std::vector<T>& GetLocal()
	{	
		clEventPtr es = GetStartReadEvent();
		clEventPtr e = GetFinishedReadEvent();

		if(es->isSet())
			es->Wait();

		if(isUpToDate == false) 
		{
			Update(es);
			isUpToDate = true;

			if((es = GetFinishedReadEvent())->isSet())
				es->Wait();
		} 
		else if(e->isSet()) 
			e->Wait();

		return Local;
	};

	// Called by clKernel for Output types to generate automatic
	// memory updates (non blocking)
	void Update(clEventPtr KernelFinished)
	{
		if(Local.empty() == true || Local.size() != Size)
			Local.resize(Size);
		Read(Local,KernelFinished);
		isUpToDate = true;
	}

	void UpdateEventOnly(clEventPtr KernelFinished)
	{
		isUpToDate = false;
		SetFinishedEvent(KernelFinished);
	};
};

#endif