// Test suite for functioning of OpenCL Wrapper using Boost.Test

#define BOOST_TEST_MAIN
#include "stdafx.h"
#include "clWrapper.h"

BOOST_AUTO_TEST_CASE(FindsSomeDevices)
{
    BOOST_REQUIRE_GT(OpenCL::GetDeviceList().size(),0);
}

BOOST_AUTO_TEST_CASE(CanCreateContext)
{
	auto DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	auto GPUContext = OpenCL::MakeContext(GPU,InOrderWithProfiling);
	BOOST_REQUIRE_EQUAL(GPUContext.GetStatus(),0);
}

BOOST_AUTO_TEST_CASE(CanAllocateBuffers)
{
	auto DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	auto GPUContext = OpenCL::MakeContext(GPU,InOrderWithProfiling);
	
	auto GPUBUffer = GPUContext.CreateBuffer<float,Auto>(1024);

	std::vector<clMemory<float,Auto>> Buffers;
	Buffers.push_back(GPUContext.CreateBuffer<float,Auto>(1024));
	Buffers.push_back(GPUContext.CreateBuffer<float,Auto>(1024));
	
	BOOST_REQUIRE_EQUAL(GPUContext.GetStatus(),0);
}


BOOST_AUTO_TEST_CASE(CanCompileKernel)
{
	auto DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	auto GPUContext = OpenCL::MakeContext(GPU,InOrderWithProfiling);
	
	const char* TestSource = "__kernel void clTest(__global float* Input, int width, int height, float value) \n"
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

	clKernel GPUKernel = GPUContext.BuildKernelFromString(TestSource,"clTest",4);


	
	
	BOOST_REQUIRE_EQUAL(GPUContext.GetStatus(),0);
}

BOOST_AUTO_TEST_CASE(CanSetArguments)
{
	auto DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	auto GPUContext = OpenCL::MakeContext(GPU,InOrderWithProfiling);
	
	std::vector<clMemory<float,Auto>> Buffers;
	Buffers.push_back(GPUContext.CreateBuffer<float,Auto>(1024));
	Buffers.push_back(GPUContext.CreateBuffer<float,Auto>(1024));

	const char* TestSource = "__kernel void clTest(__global float* Input, int width, int height, float value) \n"
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

	clKernel GPUKernel = GPUContext.BuildKernelFromString(TestSource,"clTest",4);

	GPUKernel.SetArg(0,Buffers[0],InputOutput);

	BOOST_REQUIRE_EQUAL(GPUKernel.GetStatus(),0);
	BOOST_REQUIRE_EQUAL(GPUContext.GetStatus(),0);
}

BOOST_AUTO_TEST_CASE(CanEnqueueKernel)
{
	auto DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	auto GPUContext = OpenCL::MakeContext(GPU,InOrderWithProfiling);
	
	auto GPUBuffer = GPUContext.CreateBuffer<float,Auto>(1024);

	const char* TestSource = "__kernel void clTest(__global float* Input, int width, int height, float value) \n"
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

	clKernel GPUKernel = GPUContext.BuildKernelFromString(TestSource,"clTest",4);

	GPUKernel.SetArg(0,GPUBuffer,InputOutput);
	GPUKernel.SetArg(1,1024);
	GPUKernel.SetArg(2,1);
	GPUKernel.SetArg(3,5.0f);

	clWorkGroup Work(1024,1,1);
	GPUKernel(Work);

	GPUContext.WaitForQueueFinish();

	BOOST_REQUIRE_EQUAL(GPUKernel.GetStatus(),0);
	BOOST_REQUIRE_EQUAL(GPUContext.GetStatus(),0);
}

BOOST_AUTO_TEST_CASE(KernelProducesValidResults)
{
	auto DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	auto GPUContext = OpenCL::MakeContext(GPU,InOrderWithProfiling);
	auto GPUBuffer = GPUContext.CreateBuffer<float,Auto>(1024);
	
	std::vector<clMemory<float,Auto>> Buffers;
	Buffers.push_back(GPUContext.CreateBuffer<float,Auto>(1024));

	const char* TestSource = "__kernel void clTest(__global float* Input, int width, int height, float value) \n"
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

	clKernel GPUKernel = GPUContext.BuildKernelFromString(TestSource,"clTest",4);

	GPUKernel.SetArg(0,GPUBuffer,InputOutput);
	GPUKernel.SetArg(1,1024);
	GPUKernel.SetArg(2,1);
	GPUKernel.SetArg(3,5.0f);

	clWorkGroup Work(1024,1,1);
	GPUKernel(Work);

	GPUContext.WaitForQueueFinish();

	GPUKernel.SetArg(0,Buffers[0],InputOutput);
	GPUKernel.SetArg(1,1024);
	GPUKernel.SetArg(2,1);
	GPUKernel.SetArg(3,5.0f);

	GPUKernel(Work);

	GPUContext.WaitForQueueFinish();

	std::vector<float> Result(1024,5.0f);

	BOOST_REQUIRE_EQUAL_COLLECTIONS(Result.begin(),Result.end(),GPUBuffer.GetLocal().begin(),GPUBuffer.GetLocal().end());
}