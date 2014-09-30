#include "clWrapper.h"


const char* TestSource = 
"__kernel void clTest(__global float* Input, int width, int height, float value) \n"
"{		\n"
"	int xid = get_global_id(0);	\n"
"	int yid = get_global_id(1);	\n"
"	if(xid < width && yid < height) \n"
"	{	\n"
"		int Index = xid + width*yid; \n"
"		Input[Index] += value; \n"
"	}	\n"
"}		\n"
;

const char* TestSource2 = 
"__kernel void clTest(__global float* Input, int width, int height, float value) \n"
"{		\n"
"	int xid = get_global_id(0);	\n"
"	int yid = get_global_id(1);	\n"
"	if(xid < width && yid < height) \n"
"	{	\n"
"		int Index = xid + width*yid; \n"
"		Input[Index] *= value; \n"
"	}	\n"
"}		\n"
;

int main()
{
	OpenCL cl;

	std::vector<clDevice> DeviceList = cl.GetDeviceList();
	clDoubleQueueContext GPUContext = cl.MakeDoubleQueueContext(DeviceList[0]);

	std::vector<float> initial(2048*1024,2);
	clMemory<float,Auto> GPUBuffer = GPUContext.CreateBuffer<float,Auto>(2048*1024);
	GPUBuffer.Write(initial);

	clKernel GPUKernel = GPUContext.BuildKernelFromString(TestSource,"clTest",4);
	clKernel GPUKernel2 = GPUContext.BuildKernelFromString(TestSource2,"clTest",4);

	WorkGroup Work(2048,1024,1);

	GPUKernel.SetArg(0,GPUBuffer,InputOutput);
	GPUKernel.SetArg(1,2048);
	GPUKernel.SetArg(2,1024);
	GPUKernel.SetArg(3,5.0f);

	GPUKernel2.SetArg(0,GPUBuffer,InputOutput);
	GPUKernel2.SetArg(1,2048);
	GPUKernel2.SetArg(2,1024);
	GPUKernel2.SetArg(3,2.0f);

	// How to set a local memory argument.
	//GPUKernel.SetLocalMemoryArg<cl_float>(4,1024);

	// Launch kernels.
	GPUKernel(Work);
	GPUBuffer.Write(initial);
	GPUKernel2(Work);
	GPUKernel(Work);
	std::vector<float> GPUResult2 = GPUBuffer.GetLocal();
	
	GPUContext.WaitForQueueFinish();

	// CPU Test Part

	clContext CPUContext = cl.MakeContext(DeviceList[1]);

	// Template defaults to Manual (in C++11), can optionally specify memory flags
	clMemory<float,Manual> CPUBuffer = CPUContext.CreateBuffer<float,Manual>(1024*1024,ReadWrite);
	clKernel CPUKernel = CPUContext.BuildKernelFromString(TestSource,"clTest",4);
	CPUKernel.SetArg(0,CPUBuffer,InputOutput);
	CPUKernel.SetArg(1,1024);
	CPUKernel.SetArg(2,1024);
	CPUKernel.SetArg(3,7.0f);
	
	CPUKernel(Work);

	// Manual reading into a vector
	std::vector<float> CPUResult = CPUBuffer.CreateLocalCopy();

	CPUContext.WaitForQueueFinish();

	return 0;
}