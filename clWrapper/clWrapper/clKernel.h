#ifndef CL_KERNEL_H
#define CL_KERNEL_H

#include "clMemory.h"
#include "clEvent.h"
#include "Cl/Opencl.h"
#include "clWorkgroup.h"
#include <algorithm>
#include <utility>

// Optionally passed to argument setting.
// Output types will be updated automatically when data is modified

namespace ArgumentType
{
	enum ArgTypes
	{
		Input,
		Output,
		InputOutput,
		OutputNoUpdate,
		InputOutputNoUpdate,
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

	clKernel(){ NotDefault = false; }
	~clKernel(){ if(NotDefault) { clReleaseProgram(Program); clReleaseKernel(Kernel); } }

	clKernel(clContext &_context, const char* codestring, int _NumberOfArgs, std::string _name)
		: Context(&_context), NumberOfArgs(_NumberOfArgs), Name(_name)
	{
		NotDefault = true;
		ArgType.resize(_NumberOfArgs);
		Callbacks.resize(_NumberOfArgs);
		BuildKernelFromString(codestring,_name,NumberOfArgs);
		CodeString = codestring;
	}

	// Can enter arguments as literals now...
	template <class T> void SetArg(int position, const T arg, ArgumentType::ArgTypes ArgumentType = ArgumentType::Unspecified)
	{
		ArgType[position] = ArgumentType;
		status |= clSetKernelArg(Kernel,position,sizeof(T),&arg);
	}

	clKernel& operator=(clKernel& Copy){
		if(this!=&Copy)
		{
			Context = Copy.Context;
			NumberOfArgs = Copy.NumberOfArgs;
			Name = Copy.Name;
			CodeString = Copy.CodeString;
			NotDefault = Copy.NotDefault;
			ArgType.resize(NumberOfArgs);
			Callbacks.resize(NumberOfArgs);
			BuildKernelFromString(CodeString,Name,NumberOfArgs);
		}
		return *this;
	}

	clKernel(const clKernel& Copy): Context(Copy.Context), NumberOfArgs(Copy.NumberOfArgs), Name(Copy.Name), CodeString(Copy.CodeString)
	{
		NotDefault = Copy.NotDefault;
		ArgType.resize(NumberOfArgs);
		Callbacks.resize(NumberOfArgs);
		BuildKernelFromString(CodeString,Name,NumberOfArgs);
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
	
	cl_int GetStatus(){ return status; };
	int NumberOfArgs;

private:
	bool NotDefault;
	cl_int status;
	std::vector<ArgumentType::ArgTypes> ArgType;
	std::vector<Notify*> Callbacks;
	clContext* Context;
	cl_program Program;
	cl_kernel Kernel;
	std::string Name;
	const char* CodeString;
	
	void swap(clKernel& first, clKernel& second)
	{
		std::swap(first.NotDefault,second.NotDefault);
		std::swap(first.Program,second.Program);
		std::swap(first.Kernel,second.Kernel);
		std::swap(first.Context,second.Context);
		std::swap(first.ArgType,second.ArgType);
		std::swap(first.Callbacks,second.Callbacks);
		std::swap(first.Name,second.Name);
	}

	void RunCallbacks(clEvent KernelFinished);
	void BuildKernelFromString(const char* codestring, std::string kernelname, int NumberOfArgs);
};

#endif