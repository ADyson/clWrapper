#ifndef MANUAL_H
#define MANUAL_H

#include "clEvent.h"
#include "CL/Opencl.h"

// This class is inherited by OpenCL memory buffers that have to manage there own memory lifetimes.
template <class T> class Manual abstract : public Notify
{
public:
	Manual<T>(size_t size): Size(size), isAuto(false) {};
	const bool isAuto;
	size_t Size;
	clEventPtr KernelFinished;
	void Update(clEventPtr _KernelFinished){KernelFinished = _KernelFinished;};

	virtual clEventPtr Read(std::vector<T>&data)=0;
	virtual clEventPtr Read(std::vector<T>&data,clEventPtr KernelFinished)=0;
	virtual clEventPtr GetStartWriteEvent()=0;
	virtual clEventPtr GetStartReadEvent()=0;
	virtual clEventPtr GetFinishedWriteEvent()=0;
	virtual clEventPtr GetFinishedReadEvent()=0;

	virtual void SetFinishedEvent(clEventPtr KernelFinished) =0;

	// This will create a vector filled with the current contents of the memory
	// Will block until the read has been completed
	std::vector<T> CreateLocalCopy()
	{
		std::vector<T> Local(Size);
		if(KernelFinished->event!=NULL)
		{
			clEventPtr e =	Read(Local,KernelFinished);
			e->Wait();
		}
		else
		{
			clEventPtr e =Read(Local);
			e->Wait();
		}
		return Local;
	};

	void UpdateEventOnly(clEventPtr KernelFinished)
	{
		SetFinishedEvent(KernelFinished);
	};
};

#endif 