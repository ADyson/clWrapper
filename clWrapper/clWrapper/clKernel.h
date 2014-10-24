#ifndef CL_KERNEL_H
#define CL_KERNEL_H

#include "clMemory.h"
#include "clEvent.h"
#include "Cl/Opencl.h"
#include "clWorkgroup.h"

// Optionally passed to argument setting.
// Output types will be updated automatically when data is modified

namespace ArgumentType
{
	enum ArgTypes
	{
		Input,
		Output,
		InputOutput,
		Unspecified
	};
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

	clKernel(clContext &_context, const char* codestring, int _NumberOfArgs, std::string _name)
		: Context(&_context), NumberOfArgs(_NumberOfArgs), Name(_name)
	{
		ArgType.resize(_NumberOfArgs);
		Callbacks.resize(_NumberOfArgs);
		BuildKernelFromString(codestring,_name,NumberOfArgs);
	}

	// Can enter arguments as literals now...
	template <class T> void SetArg(int position, const T arg, ArgumentType::ArgTypes ArgumentType = ArgumentType::Unspecified)
	{
		ArgType[position] = ArgumentType;
		status |= clSetKernelArg(Kernel,position,sizeof(T),&arg);
	}

	// Overload for OpenCL Memory Buffers
	template <class T, template <class> class AutoPolicy> void SetArg(int position, boost::shared_ptr<clMemory<T,AutoPolicy>>& arg, ArgumentType::ArgTypes ArgumentType = Unspecified)
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
	std::vector<ArgumentType::ArgTypes> ArgType;
	std::vector<Notify*> Callbacks;
	clContext* Context;
	cl_program Program;
	cl_kernel Kernel;
	std::string Name;

	void RunCallbacks(clEvent KernelFinished);
	void BuildKernelFromString(const char* codestring, std::string kernelname, int NumberOfArgs);
};

#endif