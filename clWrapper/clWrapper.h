// Factory object used to produce devices and queues for OpenCL operation.
#include <string>
#include <vector>
#include <stdlib.h>
#include <memory>
#include "CL/OpenCl.h"

class clContext;
template <class T> class clMemory;

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


class clKernel
{
public:
	clKernel(clContext* context) : Context(context){};

	// Can enter arguments as literals now...
	template <class T> void SetArg(int position, const T arg)
	{
		status |= clSetKernelArg(kernel,position,sizeof(T),&arg);
	}

	// Overload for OpenCL Memory Buffers
	template <class T> void SetArg(int position, clMemory<T>& arg)
	{
		status |= clSetKernelArg(kernel,position,sizeof(cl_mem),&arg.buffer);
	}
	
	// Run Kernel with just Global Work Size
	void Enqueue(WorkGroup Global);

	void Enqueue(WorkGroup Global, WorkGroup Local);

	int NumberOfArgs;
	const char* kernelcode;
	cl_program kernelprogram;
	cl_int status;
	cl_kernel kernel;
	std::string kernelname;
	size_t log;
	size_t kernelsize;
	int iter;
	int viter;
	clContext* Context;

};

// Shallow copy is OK
template <class T>
class clMemory
{
public:
	clContext Context;
	typedef T MemType;
	
	clMemory<T>(clContext context) : Context(context){};

	cl_mem buffer;
	size_t size;
	
	void clMemory<T>::Read(std::vector<T> &data)
	{
		clEnqueueReadBuffer(Context.queue,buffer,CL_TRUE,0,data.size()*sizeof(T),&data[0],0,NULL,NULL);
	};

	void clMemory<T>::Write(std::vector<T> &data)
	{
		cl_int status;
		status = clEnqueueWriteBuffer(Context.queue,buffer,CL_TRUE,0,data.size()*sizeof(T),&data[0],0,NULL,NULL);

		bool test = status==0;
	};
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
		clMemory<T> Mem(*this);
		Mem.buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, size*sizeof(T), 0, &status);
		Mem.size = size;
		return Mem;
	};

	template<class T> clMemory<T> CreateBuffer(size_t size, cl_mem_flags flags)
	{
		cl_int status;
		clMemory<T> Mem(*this);
		Mem.buffer = clCreateBuffer(context, flags, size*sizeof(T), 0, &status);
		Mem.size = size;
		return Mem;
	};

	clKernel BuildKernelFromString(const char* codestring, std::string kernelname, int NumberOfArgs);
	clKernel BuildKernelFromFile(const char* filename, std::string kernelname, int NumberOfArgs);
};



class OpenCL
{
public:
	//OpenCL(void);
	//~OpenCL(void);
	std::vector<clDevice> GetDeviceList();
	clContext MakeContext(clDevice& dev);

};