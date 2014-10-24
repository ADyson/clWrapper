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
	clEvent KernelFinished;
	void Update(clEvent _KernelFinished){KernelFinished = _KernelFinished;};

	virtual clEvent Read(std::vector<T>&data)=0;
	virtual clEvent Read(std::vector<T>&data,clEvent KernelFinished)=0;
	virtual clEvent GetStartWriteEvent()=0;
	virtual clEvent GetStartReadEvent()=0;
	virtual clEvent GetFinishedWriteEvent()=0;
	virtual clEvent GetFinishedReadEvent()=0;

	// This will create a vector filled with the current contents of the memory
	// Will block until the read has been completed
	std::vector<T> CreateLocalCopy()
	{
		std::vector<T> Local(Size);
		if(KernelFinished.event!=NULL)
		{
			clEvent e =	Read(Local,KernelFinished);
			clWaitForEvents(1,&e.event);
		}
		else
		{
			clEvent e =Read(Local);
			clWaitForEvents(1,&e.event);
		}
		return Local;
	};
};

#endif 