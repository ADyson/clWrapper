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
	std::vector<float> test;
	test.resize(1024);
	int testint = 2;

	OpenCL cl;

	std::vector<clDevice> DeviceList = cl.GetDeviceList();
	clContext GPUContext = cl.MakeContext(DeviceList[0]);
	clContext CPUContext = cl.MakeContext(DeviceList[1]);

	clMemory<float> Buffer1 = CPUContext.CreateBuffer<float>(1024);

	clKernel CPUKernel1 = CPUContext.BuildKernelFromString(TestSource,"clTest",4);
	clKernel GPUKernel1 = GPUContext.BuildKernelFromString(TestSource,"clTest",4);

	WorkGroup Work(1024,1,1);

	CPUKernel1.SetArg(0,Buffer1,InputOutput);
	CPUKernel1.SetArg(1,1024);
	CPUKernel1.SetArg(2,1);
	CPUKernel1.SetArg(3,5.0f);
	
	CPUKernel1.Enqueue(Work);

	Buffer1.Read(test);

	GPUContext.WaitForQueueFinish();

	return 0;
}