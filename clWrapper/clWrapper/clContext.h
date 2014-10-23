#ifndef CL_CONTEXT_H
#define CL_CONTEXT_H

#include "clWrapper.h"
class clKernel;

enum MemoryFlags
{
	ReadWrite = CL_MEM_READ_WRITE,
	ReadOnly = CL_MEM_READ_ONLY,
	WriteOnly = CL_MEM_WRITE_ONLY
};

class clContext
{
	friend class clFourier;

private:
	cl_int Status;
	cl_context Context;
	cl_command_queue Queue;
	clDevice ContextDevice;

public:
	clContext(clDevice _ContextDevice, cl_context _Context, cl_command_queue _Queue, cl_int _Status)
		: ContextDevice(_ContextDevice), Context(_Context), Queue(_Queue), Status(_Status){};

	void WaitForQueueFinish(){clFinish(Queue);};
	void QueueFlush(){clFlush(Queue);};

	clDevice GetContextDevice(){return ContextDevice;};
	cl_context& GetContext(){return Context;};
	cl_int GetStatus(){return Status;};
	cl_command_queue& GetQueue(){ return Queue; };
	virtual cl_command_queue& GetIOQueue(){return Queue;};

	// Use default template arguments for template functions only with c++11 or later.
	#ifdef clWrapper11
	template<class T,template <class> class AutoPolicy = Manual > boost::shared_ptr<clMemory<T,AutoPolicy>> CreateBuffer(size_t size)
	{
		boost::shared_ptr<clMemory<T,AutoPolicy>> Mem = new clMemory<T,AutoPolicy>(this,size,clCreateBuffer(Context, MemoryFlags::ReadWrite, size*sizeof(T), 0, &Status));
		return Mem;
	};

	template<class T,template <class> class AutoPolicy = Manual > boost::shared_ptr<clMemory<T,AutoPolicy>> CreateBuffer(size_t size, enum MemoryFlags flags)
	{
		boost::shared_ptr<clMemory<T,AutoPolicy>> Mem = new clMemory<T,AutoPolicy>(this,size,clCreateBuffer(Context, flags, size*sizeof(T), 0, &Status));
		return Mem;
	};
	#endif

	#ifndef clWrapper11
	template<class T,template <class> class AutoPolicy> boost::shared_ptr<clMemory<T,AutoPolicy>> CreateBuffer(size_t size)
	{
		boost::shared_ptr<clMemory<T,AutoPolicy>> Mem( new clMemory<T,AutoPolicy>(this,size,clCreateBuffer(Context, MemoryFlags::ReadWrite, size*sizeof(T), 0, &Status)));
		return Mem;
	};

	template<class T,template <class> class AutoPolicy > boost::shared_ptr<clMemory<T,AutoPolicy>> CreateBuffer(size_t size, enum MemoryFlags flags)
	{
		boost::shared_ptr<clMemory<T,AutoPolicy>> Mem( new clMemory<T,AutoPolicy>(this,size,clCreateBuffer(Context, flags, size*sizeof(T), 0, &Status)));
		return Mem;
	};
	#endif
};


#endif