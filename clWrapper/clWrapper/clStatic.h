#ifndef CL_STATIC_H
#define CL_STATIC_H

#include "clWrapper.h"
#include <list>

enum QueueType
	{
		InOrder = 0,
		InOrderWithProfiling = 0 | CL_QUEUE_PROFILING_ENABLE,
		OutOfOrder = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
		OutOfOrderWithProfiling = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE
	};


class OpenCL
{
public:
	static std::list<clDevice> GetDeviceList();
	static clContext MakeContext(clDevice& dev, QueueType Qtype = QueueType::InOrder);
};

#endif