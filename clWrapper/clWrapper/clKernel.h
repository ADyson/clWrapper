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

	clKernel(clContext* _context, int _NumberOfArgs, cl_kernel _kernel, cl_program _program, std::string _name, cl_int _status)
		: Context(_context), NumberOfArgs(_NumberOfArgs), Kernel(_kernel), Program(_program), Name(_name), status(_status)
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
	template <class T, template <class> class AutoPolicy> void SetArg(int position, boost::shared_ptr<clMemory<T,AutoPolicy>>& arg, ArgTypes ArgumentType = Unspecified)
	{
		ArgType[position] = ArgumentType;
		Callbacks[position] = arg.get();
		status |= clSetKernelArg(Kernel,position,sizeof(cl_mem),&arg->GetBuffer());
	}

	template <class T> void SetLocalMemoryArg(int position, int size) 
	{
		status |= clSetKernelArg(Kernel,position,size*sizeof(T),NULL);
	}

	clEvent operator()(clWorkGroup Global);
	clEvent operator()(clWorkGroup Global, clEvent StartEvent);
	clEvent operator()(clWorkGroup Global, clWorkGroup Local);
	clEvent operator()(clWorkGroup Global, clWorkGroup Local, clEvent StartEvent);
	
	cl_int GetStatus(){return status; };
	int NumberOfArgs;

private:
	cl_int status;
	std::vector<ArgTypes> ArgType;
	std::vector<Notify*> Callbacks;
	clContext* Context;
	cl_program Program;
	cl_kernel Kernel;
	std::string Name;

	void RunCallbacks(clEvent KernelFinished);
};

#endif