// Factory object used to produce devices and queues for OpenCL operation.
#include <string>
#include <vector>
#include <stdlib.h>
#include <memory>
#include "CL/OpenCl.h"

class clContext;
class clDevice;
class clKernel;
template <class T> class clMemory;
template <class T> class clMemoryImpl;


// Optionally passed to argument setting.
// Output types will be notified when data is modified via
// Changed flag.
enum ArgTypes
	{
		Input,
		Output,
		InputOutput,
		Unspecified
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

// Based class to allow for callbacks to set Changed status....
class Notify abstract
{
public:
virtual void SetChanged(){};
};


class clContext
{
public:
	clContext(clDevice _ContextDevice, cl_context _Context, cl_command_queue _Queue)
		: ContextDevice(_ContextDevice), Context(_Context), Queue(_Queue){};

	cl_context Context;
	cl_command_queue Queue;
	clDevice ContextDevice;
	void WaitForQueueFinish();

	template<class T> clMemoryImpl<T> CreateBuffer(size_t size)
	{
		cl_int status;
		clMemoryImpl<T> Mem(*this,size,clCreateBuffer(Context, CL_MEM_READ_WRITE, size*sizeof(T), 0, &status));
		return Mem;
	};

	template<class T> clMemoryImpl<T> CreateBuffer(size_t size, cl_mem_flags flags)
	{
		cl_int status;
		clMemoryImpl<T> Mem(*this,size,clCreateBuffer(Context, flags, size*sizeof(T), 0, &status));
		return Mem;
	};

	clKernel BuildKernelFromString(const char* codestring, std::string kernelname, int NumberOfArgs);
	//clKernel BuildKernelFromFile(const char* filename, std::string kernelname, int NumberOfArgs);
};



class clKernel
{
public:
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
	template <class T> void SetArg(int position, clMemory<T>& arg, ArgTypes ArgumentType = Unspecified)
	{
		ArgType[position] = ArgumentType;
		Callbacks[position] = &arg;
		status |= clSetKernelArg(Kernel,position,sizeof(cl_mem),&arg.GetBuffer());
	}

	template <class T> void SetLocalMemoryArg(int position, int size) 
	{
		status |= clSetKernelArg(Kernel,position,size*sizeof(T),NULL);
	}


	void Enqueue(WorkGroup Global);
	void Enqueue(WorkGroup Global, WorkGroup Local);
	void operator()(WorkGroup Global);
	void operator()(WorkGroup Global, WorkGroup Local);

private:
	std::vector<ArgTypes> ArgType;
	std::vector<Notify*> Callbacks;
	clContext Context;
	cl_program Program;
	cl_kernel Kernel;
	std::string Name;
	cl_int status;

	void RunCallbacks();
};

// Memory is split into a base and derived class.
// The derived class will free memory when it is destroyed, the base class doesn't
// The base class can only be constructed using the factory (clContext).
// Variables can only be created of the derived type using copy construction from base class.
// This allows for memory to be passed by value from factory without being deallocated, but
// when out of scope using derived class, it will be deallocated.

template <class T>
class clMemoryImpl : public Notify
{
public:
	friend class clContext;
	friend class clMemory<T>;
	typedef T MemType;
	bool Changed;
	cl_mem& GetBuffer(){ return Buffer; };
	size_t	GetSize(){ return Size*sizeof(MemType); };

	void SetChanged()
	{
		Changed = true;
	}
	
	void Read(std::vector<T> &data)
	{
		clEnqueueReadBuffer(Context.Queue,Buffer,CL_TRUE,0,data.size()*sizeof(T),&data[0],0,NULL,NULL);
		Changed = false;
	};

	void Write(std::vector<T> &data)
	{
		clEnqueueWriteBuffer(Context.Queue,Buffer,CL_TRUE,0,data.size()*sizeof(T),&data[0],0,NULL,NULL);
		Changed = false;
	};


private:
	// These can only be called by friend class to prevent creation of memory that doesn't deallocate itself.
	clMemoryImpl<T>(clContext context, size_t size, cl_mem buffer) : Context(context), Buffer(buffer), Size(size)
	{
		Changed = false;
	};

	clMemoryImpl<T>(const clMemoryImpl& RHS) : clMemoryImpl<T>(RHS.Context,RHS.Size,RHS.Buffer)
	{
		Changed=RHS.Changed;
	};

	void Release()
	{
		clReleaseMemObject(Buffer);
	};

	cl_mem Buffer;
	size_t Size;
	clContext Context;
};

// This type automatically destroys memory, can only be constructed from a clMemory.
template <class T> class clMemory: public clMemoryImpl<T>
{
public:
	clMemory<T>(const clMemoryImpl<T>& factory) : clMemoryImpl<T>(factory){};
	~clMemory<T>(){ Release(); };
};


class OpenCL
{
public:
	//OpenCL(void);
	//~OpenCL(void);
	std::vector<clDevice> GetDeviceList();
	clContext MakeContext(clDevice& dev);

};