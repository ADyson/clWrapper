#ifndef CL_MEMORY_H
#define CL_MEMORY_H

#include "clContext.h"
#include "clEvent.h"
#include "CL/Opencl.h"
#include "Auto.h"
#include "Manual.h"
#include "Notify.h"
#include "boost/shared_ptr.hpp"

class clContext;
class MemoryRecord;

template <class T, template <class> class AutoPolicy>
class clMemory : public AutoPolicy<T>
{
private:
	cl_mem Buffer;
	size_t Size;
	clContext* Context;
	MemoryRecord* Rec;

	// Will wait for this event to complete before performing read.
	clEventPtr StartReadEvent;
	// This event signifies a read has been performed.
	clEventPtr FinishedReadEvent;
	// This event will be completed after we write to this memory.
	clEventPtr FinishedWriteEvent;
	// Write will not begin until this event is completed.
	clEventPtr StartWriteEvent;

public:
	typedef boost::shared_ptr<clMemory<T,AutoPolicy>> Ptr;
	friend class clContext;
	typedef T MemType;
	cl_mem& GetBuffer(){ return Buffer; };
	size_t	GetSize(){ return Size*sizeof(MemType); };
	
	virtual clEventPtr GetFinishedWriteEvent(){return FinishedWriteEvent;};
	virtual clEventPtr GetFinishedReadEvent(){return FinishedReadEvent;};
	virtual clEventPtr GetStartWriteEvent(){return StartWriteEvent;};
	virtual clEventPtr GetStartReadEvent(){return StartReadEvent;};
			
	clEventPtr Read(std::vector<T> &data)
	{
		FinishedReadEvent.reset(new clEvent());
		clEnqueueReadBuffer(Context->GetIOQueue(),Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],0,NULL,&FinishedReadEvent->GetEvent());
		FinishedReadEvent->Set();
		return FinishedReadEvent;
	};

	// Wait on single event before reading
	clEventPtr Read(std::vector<T> &data, clEventPtr Start)
	{
		StartReadEvent = Start;
		FinishedReadEvent.reset(new clEvent());
		clEnqueueReadBuffer(Context->GetIOQueue(),Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],1,&Start->GetEvent(),&FinishedReadEvent->GetEvent());
		FinishedReadEvent->Set();
		return FinishedReadEvent;
	};

	clEventPtr Write(std::vector<T> &data)
	{
		FinishedWriteEvent.reset(new clEvent());
		clEnqueueWriteBuffer(Context->GetIOQueue(),Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],0,NULL,&FinishedWriteEvent->GetEvent());
		FinishedWriteEvent->Set();
		return FinishedWriteEvent;
	};

	// Wait on single event before writing.
	clEventPtr Write(std::vector<T> &data, clEventPtr Start)
	{
		StartWriteEvent = Start;
		FinishedWriteEvent.reset(new clEvent());
		clEnqueueWriteBuffer(Context->GetIOQueue(),Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],1,&Start->GetEvent(),&FinishedWriteEvent->GetEvent());
		FinishedWriteEvent->Set();
		return FinishedWriteEvent;
	};

	clMemory<T,AutoPolicy>(clContext* context, size_t size, cl_mem buffer, MemoryRecord* rec) : Context(context), Buffer(buffer), Size(size), 
		AutoPolicy<T>(size), FinishedReadEvent(new clEvent), FinishedWriteEvent(new clEvent), StartReadEvent(new clEvent), StartWriteEvent(new clEvent), Rec(rec){};
	//clMemory<T,AutoPolicy>(const clMemory<T,AutoPolicy>& RHS) : Context(RHS.Context), Buffer(RHS.Buffer), Size(RHS.Size), AutoPolicy<T>(RHS.Size), StartReadEvent(RHS.StartReadEvent)
	//,StartWriteEvent(RHS.StartWriteEvent),FinishedReadEvent(RHS.FinishedReadEvent),FinishedWriteEvent(RHS.FinishedWriteEvent){};

	void SetFinishedEvent(clEventPtr KernelFinished)
	{
		StartReadEvent = KernelFinished;
	};

	~clMemory<T,AutoPolicy>(){ 
			Context->RemoveMemRecord(Rec);
			Release();
	};

private:
	clMemory<T,AutoPolicy>& operator= (const clMemory<T,AutoPolicy>& other){};
	
	void Release()
	{
		if(Buffer) // Does this work?
			clReleaseMemObject(Buffer);
	};
};
#endif