#include "clWrapper.h"

enum DeviceType
{
	GPU = CL_DEVICE_TYPE_GPU,
	CPU = CL_DEVICE_TYPE_CPU,
	All = CL_DEVICE_TYPE_ALL
};

enum MemoryFlags
{
	ReadWrite = CL_MEM_READ_WRITE,
	ReadOnly = CL_MEM_READ_ONLY,
	WriteOnly = CL_MEM_WRITE_ONLY
};

enum ArgTypes
	{
		Input,
		Output,
		InputOutput,
		Unspecified
	};