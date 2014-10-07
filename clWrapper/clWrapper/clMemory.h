#ifndef CL_MEMORY_H
#define CL_MEMORY_H

#include "clWrapper.h"

template <class T, template <class> class AutoPolicy> class clMemory;

template <class T, template <class> class AutoPolicy>
class clMemoryImpl : public AutoPolicy<T>
{
public:
	friend class clContext;
	friend class clMemory<T,AutoPolicy>;
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

private:
	// These can only be called by friend class to prevent creation of memory that doesn't deallocate itself.
	clMemoryImpl<T,AutoPolicy>(clContext* context, size_t size, cl_mem buffer) : Context(context), Buffer(buffer), Size(size), AutoPolicy<T>(size){};
	clMemoryImpl<T,AutoPolicy>(const clMemoryImpl& RHS) : Context(RHS.Context), Buffer(RHS.Buffer), Size(RHS.Size), AutoPolicy<T>(RHS.Size){};

	void Release()
	{
		clReleaseMemObject(Buffer);
	};


	cl_mem Buffer;
	size_t Size;
	clContext* Context;
};

// This type automatically destroys memory, can only be constructed from a clMemory.
template <class T, template <class> class AutoPolicy = Manual > class clMemory: public clMemoryImpl<T,AutoPolicy>
{
public:
	// Specify which base class constructor to call
	clMemory<T,AutoPolicy>(const clMemoryImpl<T,AutoPolicy>& BaseType) : clMemoryImpl<T,AutoPolicy>(BaseType){};
	~clMemory<T,AutoPolicy>(){ Release(); };
};

#endif