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


enum ArgTypes
	{
		Input,
		Output,
		InputOutput,
		Unspecified
	};


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
	int platformNum;
	int deviceNum;
	int numDevices;
	std::string platformname;
	std::string devicename;
	cl_device_id deviceID;
};

class Notify
{
public:
	virtual void SetChanged(){};
};


class clContext
{
public:
	cl_context context;
	cl_command_queue queue;
	clDevice contextdevice;

	void WaitForQueueFinish();

	template<class T> clMemory<T> CreateBuffer(size_t size)
	{
		cl_int status;
		clMemory<T> Mem(*this,size,clCreateBuffer(context, CL_MEM_READ_WRITE, size*sizeof(T), 0, &status));
		return Mem;
	};

	template<class T> clMemory<T> CreateBuffer(size_t size, cl_mem_flags flags)
	{
		cl_int status;
		clMemory<T> Mem(*this,size,clCreateBuffer(context, flags, size*sizeof(T), 0, &status));
		return Mem;
	};

	clKernel BuildKernelFromString(const char* codestring, std::string kernelname, int NumberOfArgs);
	clKernel BuildKernelFromFile(const char* filename, std::string kernelname, int NumberOfArgs);
};



class clKernel
{
public:
	clKernel(clContext _context, int _NumberOfArgs) : Context(_context), NumberOfArgs(_NumberOfArgs)
	{
		ArgType.resize(_NumberOfArgs);
		Callbacks.resize(_NumberOfArgs);
	}

	// Can enter arguments as literals now...
	template <class T> void SetArg(int position, const T arg, ArgTypes ArgumentType = Unspecified)
	{
		ArgType[position] = ArgumentType;
		status |= clSetKernelArg(kernel,position,sizeof(T),&arg);
	}

	// Overload for OpenCL Memory Buffers
	template <class T> void SetArg(int position, clMemory<T>& arg, ArgTypes ArgumentType = Unspecified)
	{
		ArgType[position] = ArgumentType;
		Callbacks[position] = &arg;
		status |= clSetKernelArg(kernel,position,sizeof(cl_mem),&arg.Buffer);
	}

	// Run Kernel with just Global Work Size
	void Enqueue(WorkGroup Global);

	void Enqueue(WorkGroup Global, WorkGroup Local);



	int NumberOfArgs;
	std::vector<ArgTypes> ArgType;
	std::vector<Notify*> Callbacks;

	const char* kernelcode;
	cl_program kernelprogram;
	cl_kernel kernel;
	std::string kernelname;
	cl_int status;
	size_t log;
	clContext Context;
private:
	void RunCallbacks();
};

// Shallow copy is OK
template <class T>
class clMemory : public Notify
{
public:
	clContext Context;
	typedef T MemType;

	clMemory<T>(clContext context, size_t size, cl_mem buffer) : Context(context), Buffer(buffer), Size(size)
	{
		Changed = false;
	};

	cl_mem Buffer;
	size_t Size;
	bool Changed;

	// Currently not used
	std::vector<T> LocalData;

	void SetChanged()
	{
		Changed = true;
	}
	
	void clMemory<T>::Read(std::vector<T> &data)
	{
			clEnqueueReadBuffer(Context.queue,Buffer,CL_TRUE,0,data.size()*sizeof(T),&data[0],0,NULL,NULL);
			Changed = false;
	};

	void clMemory<T>::Write(std::vector<T> &data)
	{
		clEnqueueWriteBuffer(Context.queue,Buffer,CL_TRUE,0,data.size()*sizeof(T),&data[0],0,NULL,NULL);
		Changed = false;
	};
};



class OpenCL
{
public:
	//OpenCL(void);
	//~OpenCL(void);
	std::vector<clDevice> GetDeviceList();
	clContext MakeContext(clDevice& dev);

};