#ifndef CL_CONTEXT_H
#define CL_CONTEXT_H

#include "clWrapper.h"

class clContext
{
public:
	clContext(clDevice _ContextDevice, cl_context _Context, cl_command_queue _Queue, cl_int _Status)
		: ContextDevice(_ContextDevice), Context(_Context), Queue(_Queue), Status(_Status){};

	cl_context Context;
	cl_command_queue Queue;
	cl_int Status;

	clDevice ContextDevice;
	void WaitForQueueFinish();
	void QueueFlush();

	virtual cl_command_queue GetIOQueue(){return Queue;};


	// IMPORTANT - we can return as clMemory instead of clMemoryImpl, this allows us to use auto.
	// it will get automatically converted to the correct type anyway
	// however it is possible that without RVO the conversion could happen and then be copied 
	// causing destructor to deallocate memory...
	
//	// Use default template arguments for template functions only with c++11 or later.
//#ifdef clWrapper11
//	template<class T,template <class> class AutoPolicy = Manual > clMemoryImpl<T,AutoPolicy> CreateBuffer(size_t size)
//	{
//		clMemoryImpl<T,AutoPolicy> Mem(this,size,clCreateBuffer(Context, MemoryFlags::ReadWrite, size*sizeof(T), 0, &Status));
//		return Mem;
//	};
//
//	template<class T,template <class> class AutoPolicy = Manual > clMemoryImpl<T,AutoPolicy> CreateBuffer(size_t size, enum MemoryFlags flags)
//	{
//		clMemoryImpl<T,AutoPolicy> Mem(this,size,clCreateBuffer(Context, flags, size*sizeof(T), 0, &Status));
//		return Mem;
//	};
//#endif
//
//#ifndef clWrapper11
//	template<class T,template <class> class AutoPolicy> clMemoryImpl<T,AutoPolicy> CreateBuffer(size_t size)
//	{
//		clMemoryImpl<T,AutoPolicy> Mem(this,size,clCreateBuffer(Context, MemoryFlags::ReadWrite, size*sizeof(T), 0, &Status));
//		return Mem;
//	};
//
//	template<class T,template <class> class AutoPolicy > clMemoryImpl<T,AutoPolicy> CreateBuffer(size_t size, enum MemoryFlags flags)
//	{
//		clMemoryImpl<T,AutoPolicy> Mem(this,size,clCreateBuffer(Context, flags, size*sizeof(T), 0, &Status));
//		return Mem;
//	};
//#endif
//
	clKernel BuildKernelFromString(const char* codestring, std::string kernelname, int NumberOfArgs);
	//clKernel BuildKernelFromFile(const char* filename, std::string kernelname, int NumberOfArgs);
};


#endif