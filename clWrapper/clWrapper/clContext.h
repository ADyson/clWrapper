#ifndef CL_CONTEXT_H
#define CL_CONTEXT_H

#include <list>
#include "clDevice.h"
#include "CL/Opencl.h"
#include "boost/shared_ptr.hpp"

template <class T, template <class> class AutoPolicy> class clMemory;

enum MemoryFlags
{
	ReadWrite = CL_MEM_READ_WRITE,
	ReadOnly = CL_MEM_READ_ONLY,
	WriteOnly = CL_MEM_WRITE_ONLY
};

class MemoryRecord
{
public:
	MemoryRecord(size_t _size): size(_size){};
	size_t size;
};

class clContext
{

private:
	cl_int Status;
	cl_context Context;
	cl_command_queue Queue;
	cl_command_queue IOQueue;
	clDevice ContextDevice;
	std::list<MemoryRecord*> MemList;

public:
	clContext(clDevice _ContextDevice, cl_context _Context, cl_command_queue _Queue, cl_int _Status)
		: ContextDevice(_ContextDevice), Context(_Context), Queue(_Queue), IOQueue(_Queue), Status(_Status){};
	clContext(clDevice _ContextDevice, cl_context _Context, cl_command_queue _Queue, cl_command_queue _IOQueue, cl_int _Status)
		: ContextDevice(_ContextDevice), Context(_Context), Queue(_Queue), IOQueue(_IOQueue), Status(_Status){};

	void WaitForQueueFinish(){clFinish(Queue);};
	void WaitForIOQueueFinish(){clFinish(IOQueue);};
	void QueueFlush(){clFlush(Queue);};
	void IOQueueFlush(){clFlush(Queue);};

	clDevice GetContextDevice(){return ContextDevice;};
	cl_context& GetContext(){return Context;};
	cl_int GetStatus(){return Status;};
	cl_command_queue& GetIOQueue(){ return IOQueue; };
	cl_command_queue& GetQueue(){ return Queue; };

	size_t GetOccupiedMemorySize()
	{
		std::list<MemoryRecord*>::iterator it; size_t total = 0;
		for(it = MemList.begin(); it != MemList.end(); it++)
		{
			total += (*it)->size;
		}
		return total;
	}

	void RemoveMemRecord(MemoryRecord* rec)
	{
		MemList.remove(rec);
	}

	// Need to add functions that take sizes of all the different dimensions
	template<class T,template <class> class AutoPolicy> boost::shared_ptr<clMemory<T,AutoPolicy>> CreateBuffer(size_t sizex, enum MemoryFlags eMemoryFlag = MemoryFlags::ReadWrite)
	{
		MemoryRecord* rec = new MemoryRecord(sizex*sizeof(T));
		boost::shared_ptr<clMemory<T,AutoPolicy>> Mem( new clMemory<T,AutoPolicy>(this,sizex,clCreateBuffer(Context, eMemoryFlag, sizex*sizeof(T), 0, &Status),rec));
		MemList.push_back(rec);
		return Mem;
	};

	// Need to add functions that take sizes of all the different dimensions
	template<class T,template <class> class AutoPolicy> boost::shared_ptr<clMemory<T,AutoPolicy>> CreateBuffer(size_t sizex, size_t sizey, enum MemoryFlags eMemoryFlag = MemoryFlags::ReadWrite)
	{
		MemoryRecord* rec = new MemoryRecord(sizex*sizey*sizeof(T));
		boost::shared_ptr<clMemory<T,AutoPolicy>> Mem( new clMemory<T,AutoPolicy>(this,sizex,sizey,clCreateBuffer(Context, eMemoryFlag, sizex*sizey*sizeof(T), 0, &Status),rec));
		MemList.push_back(rec);
		return Mem;
	};

	// Need to add functions that take sizes of all the different dimensions
	template<class T,template <class> class AutoPolicy> boost::shared_ptr<clMemory<T,AutoPolicy>> CreateBuffer(size_t sizex, size_t sizey, size_t sizez, enum MemoryFlags eMemoryFlag = MemoryFlags::ReadWrite)
	{
		MemoryRecord* rec = new MemoryRecord(sizex*sizey*sizez*sizeof(T));
		boost::shared_ptr<clMemory<T,AutoPolicy>> Mem( new clMemory<T,AutoPolicy>(this,sizex,sizey,sizez,clCreateBuffer(Context, eMemoryFlag, sizex*sizey*sizez*sizeof(T), 0, &Status),rec));
		MemList.push_back(rec);
		return Mem;
	};
};
#endif