// Factory object used to produce devices and queues for OpenCL operation.
#include <string>
#include <vector>
#include <memory>
#include <stdlib.h>
#include "CL/OpenCl.h"

// Define clWrapper11 to allow for template functions with default arguments.
// NB: This is only possible since C++11

//#define clWrapper11

class clContext;
class clDevice;
class clKernel;
class clEvent;
class clEventWaitList;

template <class T> class Auto;
template <class T> class Manual;
template <class T, template <class> class AutoPolicy> class clMemory;
template <class T, template <class> class AutoPolicy> class clMemoryImpl;


// Optionally passed to argument setting.
// Output types will be updated automatically when data is modified
enum ArgTypes
	{
		Input,
		Output,
		InputOutput,
		Unspecified
	};

// Used to specify buffers that can be read or write only during creation.
enum MemoryFlags
	{
		ReadWrite = CL_MEM_READ_WRITE,
		ReadOnly = CL_MEM_READ_ONLY,
		WriteOnly = CL_MEM_WRITE_ONLY
	};

// Specifiy number of threads to launch
class WorkGroup
{
public:
	WorkGroup(unsigned int x, unsigned int y, unsigned int z)
	{
		worksize[0] = x;
		worksize[1] = y;
		worksize[2] = z;
	};

	WorkGroup(size_t* workgroupsize)
	{
		worksize[0] = workgroupsize[0];
		worksize[1] = workgroupsize[1];
		worksize[2] = workgroupsize[2];
	};

	size_t worksize[3];
};

// Wrapper for OpenCL devices, used to construct a context
class clDevice
{
public:
	clDevice(cl_device_id devID, int platNum, int devNum, std::string platName, std::string devName )
		: deviceID(devID), deviceNum(devNum), platformNum(platNum), platformname(platName), devicename(devName){};

	cl_device_id& GetDeviceID(){ return deviceID; };

private:
	int platformNum;
	int deviceNum;
	std::string platformname;
	std::string devicename;
	cl_device_id deviceID;
};

// Based class to allow for callbacks..
class Notify abstract
{
public:
	virtual void Update(clEvent KernelFinished)=0;
	virtual clEvent GetFinishedWriteEvent()=0;
	virtual clEvent GetFinishedReadEvent()=0;
};

// Used to synchronise OpenCL functions that depend on other results.
class clEvent
{
public:
	cl_event event;
};

// Wrapper for OpenCL context, used to create kernels and memory buffers
// Can make a different context for different devices
class clContext
{
public:
	clContext(clDevice _ContextDevice, cl_context _Context, cl_command_queue _Queue)
		: ContextDevice(_ContextDevice), Context(_Context), Queue(_Queue){};

	cl_context Context;
	cl_command_queue Queue;
	cl_int Status;

	clDevice ContextDevice;
	void WaitForQueueFinish();
	void QueueFlush();


	// IMPORTANT - we can return as clMemory instead of clMemoryImpl, this allows us to use auto.
	// it will get automatically converted to the correct type anyway
	// however it is possible that without RVO the conversion could happen and then be copied 
	// causing destructor to deallocate memory...
	
	// Use default template arguments for template functions only with c++11 or later.
#ifdef clWrapper11
	template<class T,template <class> class AutoPolicy = Manual > clMemoryImpl<T,AutoPolicy> CreateBuffer(size_t size)
	{
		clMemoryImpl<T,AutoPolicy> Mem(*this,size,clCreateBuffer(Context, MemoryFlags::ReadWrite, size*sizeof(T), 0, &Status));
		return Mem;
	};

	template<class T,template <class> class AutoPolicy = Manual > clMemoryImpl<T,AutoPolicy> CreateBuffer(size_t size, enum MemoryFlags flags)
	{
		clMemoryImpl<T,AutoPolicy> Mem(*this,size,clCreateBuffer(Context, flags, size*sizeof(T), 0, &Status));
		return Mem;
	};
#endif

#ifndef clWrapper11
	template<class T,template <class> class AutoPolicy> clMemoryImpl<T,AutoPolicy> CreateBuffer(size_t size)
	{
		clMemoryImpl<T,AutoPolicy> Mem(*this,size,clCreateBuffer(Context, MemoryFlags::ReadWrite, size*sizeof(T), 0, &Status));
		return Mem;
	};

	template<class T,template <class> class AutoPolicy > clMemoryImpl<T,AutoPolicy> CreateBuffer(size_t size, enum MemoryFlags flags)
	{
		clMemoryImpl<T,AutoPolicy> Mem(*this,size,clCreateBuffer(Context, flags, size*sizeof(T), 0, &Status));
		return Mem;
	};
#endif

	clKernel BuildKernelFromString(const char* codestring, std::string kernelname, int NumberOfArgs);
	//clKernel BuildKernelFromFile(const char* filename, std::string kernelname, int NumberOfArgs);
};

class clDoubleQueueContext: public clContext
{
public:
	clDoubleQueueContext(clDevice _ContextDevice, cl_context _Context, cl_command_queue _Queue, cl_command_queue _IOQueue)
		: clContext(_ContextDevice,_Context,_Queue), IOQueue(_IOQueue){};

	cl_command_queue IOQueue;
	void WaitForIOQueueFinish();
	void IOQueueFlush();
};

// Wrapper for individual kernel objects
class clKernel
{
public:
	class BuildException: public std::runtime_error
	{
	public:
		BuildException(std::string message, cl_int status): runtime_error(message), Status(status){};
		cl_int Status;
	};

	int NumberOfArgs;

	clKernel(clContext _context, int _NumberOfArgs, cl_kernel _kernel, cl_program _program, std::string _name)
		: Context(_context), NumberOfArgs(_NumberOfArgs), Kernel(_kernel), Program(_program), Name(_name)
	{
		ArgType.resize(_NumberOfArgs);
		Callbacks.resize(_NumberOfArgs);
	}

	// Can enter arguments as literals now...
	template <class T> void SetArg(int position, const T arg, ArgTypes ArgumentType = Unspecified)
	{
		ArgType[position] = ArgumentType;
		status |= clSetKernelArg(Kernel,position,sizeof(T),&arg);
	}

	// Overload for OpenCL Memory Buffers
	template <class T, template <class> class AutoPolicy> void SetArg(int position, clMemory<T,AutoPolicy>& arg, ArgTypes ArgumentType = Unspecified)
	{
		ArgType[position] = ArgumentType;
		Callbacks[position] = &arg;
		status |= clSetKernelArg(Kernel,position,sizeof(cl_mem),&arg.GetBuffer());
	}

	template <class T> void SetLocalMemoryArg(int position, int size) 
	{
		status |= clSetKernelArg(Kernel,position,size*sizeof(T),NULL);
	}

	//void Enqueue(WorkGroup Global);
	//void Enqueue(WorkGroup Global, WorkGroup Local);
	clEvent operator()(WorkGroup Global);
	clEvent operator()(WorkGroup Global, clEvent StartEvent);
	//void operator()(WorkGroup Global, WorkGroup Local);

private:
	std::vector<ArgTypes> ArgType;
	std::vector<Notify*> Callbacks;
	clContext Context;
	cl_program Program;
	cl_kernel Kernel;
	std::string Name;
	cl_int status;

	void RunCallbacks(clEvent KernelFinished);
};

// This class can facilitate automatically retrieving changes to OpenCL memory buffers.
// from kernels with argument types specified.
template <class T> class Auto abstract : public Notify
{
public:
	Auto<T>(size_t size): Size(size), isAuto(true){};

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
	
	// This call will block if the Memory is currently waiting on
	// an event before updating itself.
	std::vector<T>& GetLocal()
	{	
		cl_event e = GetStartReadEvent().event;
		clWaitForEvents(1,&e);
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
};

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

// Memory is split into a base and derived class.
// The derived class will free memory when it is destroyed, the base class doesn't
// The base class can only be constructed using the factory (clContext).
// Variables can only be created of the derived type using copy construction from base class.
// This allows for memory to be passed by value from factory without being deallocated, but
// when out of scope using derived class, it will be deallocated.

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
		clEnqueueReadBuffer(Context.Queue,Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],0,NULL,&FinishedReadEvent.event);
		return FinishedReadEvent;
	};

	// Wait on single event before reading
	clEvent Read(std::vector<T> &data, clEvent Start)
	{
		StartReadEvent = Start;
		clEnqueueReadBuffer(Context.Queue,Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],1,&Start.event,&FinishedReadEvent.event);
		return FinishedReadEvent;
	};

	clEvent Write(std::vector<T> &data)
	{
		clEnqueueWriteBuffer(Context.Queue,Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],0,NULL,&FinishedWriteEvent.event);
		return FinishedWriteEvent;
	};

	// Wait on single event before writing.
	clEvent Write(std::vector<T> &data, clEvent Start)
	{
		StartWriteEvent = Start;
		clEnqueueWriteBuffer(Context.Queue,Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],1,&Start.event,&FinishedWriteEvent.event);
		return FinishedWriteEvent;
	};

private:
	// These can only be called by friend class to prevent creation of memory that doesn't deallocate itself.
	clMemoryImpl<T,AutoPolicy>(clContext context, size_t size, cl_mem buffer) : Context(context), Buffer(buffer), Size(size), AutoPolicy<T>(size){};
	clMemoryImpl<T,AutoPolicy>(const clMemoryImpl& RHS) : Context(RHS.Context), Buffer(RHS.Buffer), Size(RHS.Size), AutoPolicy<T>(RHS.Size){};

	void Release()
	{
		clReleaseMemObject(Buffer);
	};


	cl_mem Buffer;
	size_t Size;
	clContext Context;
};

// This type automatically destroys memory, can only be constructed from a clMemory.
template <class T, template <class> class AutoPolicy = Manual > class clMemory: public clMemoryImpl<T,AutoPolicy>
{
public:
	// Specify which base class constructor to call
	clMemory<T,AutoPolicy>(const clMemoryImpl<T,AutoPolicy>& BaseType) : clMemoryImpl<T,AutoPolicy>(BaseType){};
	~clMemory<T,AutoPolicy>(){ Release(); };
};

class OpenCL
{
public:
	//OpenCL(void);
	//~OpenCL(void);
	std::vector<clDevice> GetDeviceList();
	clContext MakeContext(clDevice& dev);
	clDoubleQueueContext MakeDoubleQueueContext(clDevice& dev);
};

