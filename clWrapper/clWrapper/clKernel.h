#ifndef CL_KERNEL_H
#define CL_KERNEL_H

#include "clWrapper.h"

// Optionally passed to argument setting.
// Output types will be updated automatically when data is modified
enum ArgTypes
	{
		Input,
		Output,
		InputOutput,
		Unspecified
	};


class clKernel
{
public:
	class BuildException: public std::runtime_error
	{
	public:
		BuildException(std::string message, cl_int status): runtime_error(message), Status(status){};
		cl_int Status;
	};

	clKernel(clContext* _context, int _NumberOfArgs, cl_kernel _kernel, cl_program _program, std::string _name)
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

	//void Enqueue(clWorkGroup Global);
	//void Enqueue(clWorkGroup Global, clWorkGroup Local);
	clEvent operator()(clWorkGroup Global);
	clEvent operator()(clWorkGroup Global, clEvent StartEvent);
	//void operator()(clWorkGroup Global, clWorkGroup Local);
	
	cl_int status;
	int NumberOfArgs;

private:
	std::vector<ArgTypes> ArgType;
	std::vector<Notify*> Callbacks;
	clContext* Context;
	cl_program Program;
	cl_kernel Kernel;
	std::string Name;

	void RunCallbacks(clEvent KernelFinished);
};

#endif