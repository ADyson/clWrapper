#ifndef CL_MEMORY_H
#define CL_MEMORY_H

#include "clWrapper.h"

class clContext;

template <class T, template <class> class AutoPolicy> class clMemory;

template <class T, template <class> class AutoPolicy>
class clMemory : public AutoPolicy<T>
{
private:
	cl_mem Buffer;
	size_t Size;
	clContext* Context;

public:
	mutable int* refPtr;
	friend class clContext;
	typedef T MemType;
	cl_mem& GetBuffer(){ return Buffer; };
	size_t	GetSize(){ return Size*sizeof(MemType); };
	
	// Will wait for this event to complete before performing read.
	clEvent StartReadEvent;
	// This event signifies a read has been performed.
	clEvent FinishedReadEvent;
	// This event will be completed after we write to this memory.
	clEvent FinishedWriteEvent;
	// Write will not begin until this event is completed.
	clEvent StartWriteEvent;

	virtual clEvent GetFinishedWriteEvent(){return FinishedWriteEvent;};
	virtual clEvent GetFinishedReadEvent(){return FinishedReadEvent;};
	virtual clEvent GetStartWriteEvent(){return StartWriteEvent;};
	virtual clEvent GetStartReadEvent(){return StartReadEvent;};
			
	clEvent Read(std::vector<T> &data)
	{
		clEnqueueReadBuffer(Context->GetIOQueue(),Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],0,NULL,&FinishedReadEvent.event);
		return FinishedReadEvent;
	};

	// Wait on single event before reading
	clEvent Read(std::vector<T> &data, clEvent Start)
	{
		StartReadEvent = Start;
		clEnqueueReadBuffer(Context->GetIOQueue(),Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],1,&Start.event,&FinishedReadEvent.event);
		return FinishedReadEvent;
	};

	clEvent Write(std::vector<T> &data)
	{
		clEnqueueWriteBuffer(Context->GetIOQueue(),Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],0,NULL,&FinishedWriteEvent.event);
		return FinishedWriteEvent;
	};

	// Wait on single event before writing.
	clEvent Write(std::vector<T> &data, clEvent Start)
	{
		StartWriteEvent = Start;
		clEnqueueWriteBuffer(Context->GetIOQueue(),Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],1,&Start.event,&FinishedWriteEvent.event);
		return FinishedWriteEvent;
	};

	void IncrementRefCount() const
	{
		refPtr[0]++;
	}

	clMemory<T,AutoPolicy>(clContext* context, size_t size, cl_mem buffer) : Context(context), Buffer(buffer), Size(size), AutoPolicy<T>(size){ refPtr = new int[1]; refPtr[0]=1;};
	clMemory<T,AutoPolicy>(const clMemory& RHS) : Context(RHS.Context), Buffer(RHS.Buffer), Size(RHS.Size), AutoPolicy<T>(RHS.Size), StartReadEvent(RHS.StartReadEvent)
	,StartWriteEvent(RHS.StartWriteEvent),FinishedReadEvent(RHS.FinishedReadEvent),FinishedWriteEvent(RHS.FinishedWriteEvent){ RHS.IncrementRefCount(); refPtr = RHS.refPtr;};
	~clMemory<T,AutoPolicy>(){ 
		if(refPtr[0]==1)
		{
			Release();
			delete[] refPtr;
		}
		else
		{
			refPtr[0]--;
		}
	};

private:
	// No Copying Allowed
	clMemory<T,AutoPolicy>& operator= (const clMemory<T,AutoPolicy>& RHS);

	void Release()
	{
		clReleaseMemObject(Buffer);
	};
};

#endif