// Test suite for functioning of OpenCL Wrapper using Boost.Test

#define BOOST_TEST_MAIN
#include "stdafx.h"
#include "clWrapper.h"
#include <complex>


BOOST_AUTO_TEST_CASE(FindsSomeDevices)
{
	//auto list = OpenCL::GetDeviceList();
	//std::for_each(list.begin(),list.end(),[](clDevice dev ){printf(dev.GetDeviceName().c_str()+"\n");});

    BOOST_REQUIRE_GT(OpenCL::GetDeviceList().size(),0);
}

BOOST_AUTO_TEST_CASE(CanCreateContext)
{
	std::list<clDevice> DevList = OpenCL::GetDeviceList();
	
	clDevice GPU = *DevList.begin();
	clContext GPUContext = OpenCL::MakeContext(GPU,Queue::InOrderWithProfiling);
	BOOST_REQUIRE_EQUAL(GPUContext.GetStatus(),0);
}

BOOST_AUTO_TEST_CASE(CanAllocateBuffers)
{
	std::list<clDevice> DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	clContext GPUContext = OpenCL::MakeContext(GPU,Queue::InOrderWithProfiling);
	
	boost::shared_ptr<clMemory<float,Auto>> GPUBuffer = GPUContext.CreateBuffer<float,Auto>(1024);	
	std::vector<float> loc = GPUBuffer->GetLocal();

	BOOST_REQUIRE_EQUAL(GPUContext.GetStatus(),0);
	BOOST_REQUIRE_GT(GPUBuffer->GetSize(),0);
}


BOOST_AUTO_TEST_CASE(CanCompileKernel)
{
	std::list<clDevice> DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	clContext GPUContext = OpenCL::MakeContext(GPU,Queue::InOrderWithProfiling);
	
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

	clKernel GPUKernel(GPUContext,TestSource,4,"clTest");
	BOOST_REQUIRE_EQUAL(GPUContext.GetStatus(),0);
}

BOOST_AUTO_TEST_CASE(CanSetArguments)
{
	std::list<clDevice> DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	clContext GPUContext = OpenCL::MakeContext(GPU,Queue::InOrderWithProfiling);
	
	std::vector<boost::shared_ptr<clMemory<float,Auto>>> Buffers;
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

	clKernel GPUKernel(GPUContext,TestSource,4,"clTest");

	GPUKernel.SetArg(0,Buffers[0],ArgumentType::InputOutput);

	BOOST_REQUIRE_EQUAL(GPUKernel.GetStatus(),0);
	BOOST_REQUIRE_EQUAL(GPUContext.GetStatus(),0);
}

BOOST_AUTO_TEST_CASE(CanEnqueueKernel)
{
	std::list<clDevice> DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	clContext GPUContext = OpenCL::MakeContext(GPU,Queue::InOrderWithProfiling);
	
	boost::shared_ptr<clMemory<float,Auto>> GPUBuffer = GPUContext.CreateBuffer<float,Auto>(1024);

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

	clKernel GPUKernel(GPUContext,TestSource,4,"clTest");

	GPUKernel.SetArg(0,GPUBuffer,ArgumentType::InputOutput);
	GPUKernel.SetArg(1,1024);
	GPUKernel.SetArg(2,1);
	GPUKernel.SetArg(3,5.0f);

	clWorkGroup Work(1024,1,1);
	GPUKernel(Work);

	GPUContext.WaitForQueueFinish();

	BOOST_REQUIRE_EQUAL(GPUKernel.GetStatus(),0);
	BOOST_REQUIRE_EQUAL(GPUContext.GetStatus(),0);
}

BOOST_AUTO_TEST_CASE(CanUploadDataToGPU)
{
	std::list<clDevice> DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	clContext GPUContext = OpenCL::MakeContext(GPU,Queue::InOrder);
	
	clMemory<float,Auto>::Ptr GPUBuffer = GPUContext.CreateBuffer<float,Auto>(1024);

	std::vector<float> InitialData = std::vector<float>(1024,6.23f);
	GPUBuffer->Write(InitialData);

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

	clKernel GPUKernel(GPUContext,TestSource,4,"clTest");

	GPUKernel.SetArg(0,GPUBuffer,ArgumentType::InputOutput);
	GPUKernel.SetArg(1,1024);
	GPUKernel.SetArg(2,1);
	GPUKernel.SetArg(3,5.0f);

	clWorkGroup Work(1024,1,1);
	GPUKernel(Work);

	std::vector<float> loc = GPUBuffer->GetLocal();

	GPUContext.WaitForQueueFinish();

	BOOST_REQUIRE_EQUAL(GPUKernel.GetStatus(),0);
	BOOST_REQUIRE_EQUAL(GPUContext.GetStatus(),0);
}
BOOST_AUTO_TEST_CASE(KernelProducesValidResults)
{
	std::list<clDevice> DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	clContext GPUContext = OpenCL::MakeContext(GPU,Queue::InOrderWithProfiling);
	boost::shared_ptr<clMemory<float,Auto>> GPUBuffer = GPUContext.CreateBuffer<float,Auto>(1024);

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
	std::vector<float> InitialData = std::vector<float>(1024,6.23f);
	GPUBuffer->Write(InitialData);

	clKernel GPUKernel(GPUContext,TestSource,4,"clTest");

	GPUKernel.SetArg(0,GPUBuffer,ArgumentType::InputOutput);
	GPUKernel.SetArg(1,1024);
	GPUKernel.SetArg(2,1);
	GPUKernel.SetArg(3,5.0f);

	clWorkGroup Work(1024,1,1);
	GPUKernel(Work);

	std::vector<float> Result(1024,11.23f);

	BOOST_REQUIRE_EQUAL_COLLECTIONS(Result.begin(),Result.end(),GPUBuffer->GetLocal().begin(),GPUBuffer->GetLocal().end());
}

BOOST_AUTO_TEST_CASE(KernelCanBeProfiled)
{
	std::list<clDevice> DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	clContext GPUContext = OpenCL::MakeContext(GPU,Queue::InOrderWithProfiling);
	boost::shared_ptr<clMemory<float,Auto>> GPUBuffer = GPUContext.CreateBuffer<float,Auto>(1024);
	
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
	std::vector<float> InitialData = std::vector<float>(1024,6.23f);
	GPUBuffer->Write(InitialData);

	clKernel GPUKernel(GPUContext,TestSource,4,"clTest");

	GPUKernel.SetArg(0,GPUBuffer,ArgumentType::InputOutput);
	GPUKernel.SetArg(1,1024);
	GPUKernel.SetArg(2,1);
	GPUKernel.SetArg(3,5.0f);

	clWorkGroup Work(1024,1,1);
	clEvent profile = GPUKernel(Work);

	cl_ulong time = profile.GetElapsedTime();
	GPUContext.WaitForQueueFinish();

	BOOST_REQUIRE_GT(time,0);


}

BOOST_AUTO_TEST_CASE(FourierTransformWorks)
{
	clContext GPUContext = OpenCL::MakeContext(OpenCL::GetDeviceList(),Queue::InOrder,Device::GPU);

	clMemory<std::complex<float>,Auto>::Ptr GPUBuffer = GPUContext.CreateBuffer<std::complex<float>,Auto>(1024*1024);
	clMemory<std::complex<float>,Auto>::Ptr GPUBuffer2 = GPUContext.CreateBuffer<std::complex<float>,Auto>(1024*1024);

	std::vector<std::complex<float>> InitialData = std::vector<std::complex<float>>(1024*1024,1.0f);
	GPUBuffer->Write(InitialData);

	clFourier GPUKernel = clFourier(GPUContext,1024,1024);

	GPUKernel(GPUBuffer,GPUBuffer2,Direction::Forwards);

	// Value of first element should be sqrt(1024*1024)*1 = 1024...
	BOOST_REQUIRE_EQUAL(std::complex<float>(1024,0),GPUBuffer2->GetLocal()[0]);

}

BOOST_AUTO_TEST_CASE(FourierTransformCanBeCopied)
{
	clContext GPUContext = OpenCL::MakeTwoQueueContext(OpenCL::GetDeviceList());

	clMemory<std::complex<float>,Auto>::Ptr GPUBuffer = GPUContext.CreateBuffer<std::complex<float>,Auto>(1024*1024);
	clMemory<std::complex<float>,Auto>::Ptr GPUBuffer2 = GPUContext.CreateBuffer<std::complex<float>,Auto>(1024*1024);

	std::vector<std::complex<float>> InitialData = std::vector<std::complex<float>>(1024*1024,1.0f);
	GPUBuffer->Write(InitialData);

	clFourier GPUKernel = clFourier(GPUContext,1024,1024);
	clFourier GPUKernel2(GPUKernel); // Copy constructor
	GPUKernel = GPUKernel2; // Copy assignment

	GPUKernel2(GPUBuffer,GPUBuffer2,Direction::Forwards);
	GPUKernel(GPUBuffer,GPUBuffer2,Direction::Forwards);

	// Value of first element should be sqrt(1024*1024)*1 = 1024...
	BOOST_REQUIRE_EQUAL(std::complex<float>(1024,0),GPUBuffer2->GetLocal()[0]);
}

BOOST_AUTO_TEST_CASE(KernelCanBeCopied)
{
	clContext GPUContext = OpenCL::MakeTwoQueueContext(OpenCL::GetDeviceList());

	clMemory<float,Auto>::Ptr GPUBuffer = GPUContext.CreateBuffer<float,Auto>(1024*1024);

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

	std::vector<float> InitialData = std::vector<float>(1024*1024,6.23f);
	GPUBuffer->Write(InitialData);

	clKernel DummyKernel;
	clKernel GPUKernel(GPUContext,TestSource,4,"clTest");
	DummyKernel = GPUKernel;
	clKernel DummyKernel2(DummyKernel);

	DummyKernel.SetArg(0,GPUBuffer,ArgumentType::InputOutput);
	DummyKernel.SetArg(1,1024);
	DummyKernel.SetArg(2,1024);
	DummyKernel.SetArg(3,5.0f);

	DummyKernel2.SetArg(0,GPUBuffer,ArgumentType::InputOutput);
	DummyKernel2.SetArg(1,1024);
	DummyKernel2.SetArg(2,1024);
	DummyKernel2.SetArg(3,5.0f);

	clWorkGroup Work(1024,1024,1);
	DummyKernel(Work);
	DummyKernel2(Work);

	// Value of first element should be 6.23 + 5.0 + 5.0
	BOOST_REQUIRE_EQUAL(16.23f,GPUBuffer->GetLocal()[0]);
}

