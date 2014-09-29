#include "clWrapper.h"


const char* TestSource = 
"__kernel void clTest(__global float* Input, int width, int height, float value) \n"
"{		\n"
"	int xid = get_global_id(0);	\n"
"	int yid = get_global_id(1);	\n"
"	if(xid < width && yid < height) \n"
"	{	\n"
"		int Index = xid + width*yid; \n"
"		Input[Index] = value; \n"
"	}	\n"
"}		\n"
;

int main()
{
	OpenCL cl;

	std::vector<clDevice> DeviceList = cl.GetDeviceList();
	clContext GPUContext = cl.MakeContext(DeviceList[0]);
	clContext CPUContext = cl.MakeContext(DeviceList[1]);

	clMemory<float,Auto> GPUBuffer = GPUContext.CreateBuffer<float,Auto>(1024*1024);
	
	// Template defaults to Manual (in C++11), can optionally specify memory flags
	clMemory<float> CPUBuffer = CPUContext.CreateBuffer<float>(1024*1024,ReadWrite);

	clKernel CPUKernel = CPUContext.BuildKernelFromString(TestSource,"clTest",4);
	clKernel GPUKernel = GPUContext.BuildKernelFromString(TestSource,"clTest",4);

	WorkGroup Work(1024,1024,1);

	GPUKernel.SetArg(0,GPUBuffer,InputOutput);
	GPUKernel.SetArg(1,1024);
	GPUKernel.SetArg(2,1024);
	GPUKernel.SetArg(3,5.0f);

	CPUKernel.SetArg(0,CPUBuffer,InputOutput);
	CPUKernel.SetArg(1,1024);
	CPUKernel.SetArg(2,1024);
	CPUKernel.SetArg(3,7.0f);
	
	// How to set a local memory argument.
	//GPUKernel.SetLocalMemoryArg<cl_float>(4,1024);

	// Launch kernels.
	GPUKernel(Work);
	CPUKernel(Work);
	
	// Manual reading into a vector
	std::vector<float> CPUResult = CPUBuffer.CreateLocalCopy();

	GPUContext.WaitForQueueFinish();
	CPUContext.WaitForQueueFinish();

	return 0;
}