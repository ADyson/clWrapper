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
	BOOST_REQUIRE_EQUAL(GPUContext.Status,0);
}

BOOST_AUTO_TEST_CASE(CanAllocateBuffer)
{
	auto DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	auto GPUContext = OpenCL::MakeContext(GPU,InOrderWithProfiling);
	
	
	BOOST_REQUIRE_EQUAL(GPUContext.Status,0);
}


BOOST_AUTO_TEST_CASE(CanCompileKernel)
{
	auto DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	auto GPUContext = OpenCL::MakeContext(GPU,InOrderWithProfiling);
	
	
	BOOST_REQUIRE_EQUAL(GPUContext.Status,0);
}

BOOST_AUTO_TEST_CASE(CanSetArguments)
{
	auto DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	auto GPUContext = OpenCL::MakeContext(GPU,InOrderWithProfiling);
	
	
	BOOST_REQUIRE_EQUAL(GPUContext.Status,0);
}

BOOST_AUTO_TEST_CASE(CanEnqueueKernel)
{
	auto DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	auto GPUContext = OpenCL::MakeContext(GPU,InOrderWithProfiling);
	
	
	BOOST_REQUIRE_EQUAL(GPUContext.Status,0);
}

BOOST_AUTO_TEST_CASE(KernelProducesValidResults)
{
	auto DevList = OpenCL::GetDeviceList();
	clDevice GPU = DevList.front();
	auto GPUContext = OpenCL::MakeContext(GPU,InOrderWithProfiling);
	
	
	BOOST_REQUIRE_EQUAL(GPUContext.Status,0);
}