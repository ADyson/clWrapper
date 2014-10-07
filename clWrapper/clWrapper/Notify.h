#ifndef NOTIFY_H
#define NOTIFY_H

#include "clWrapper.h"

class Notify abstract
{
public:
	virtual void Update(clEvent KernelFinished)=0;
	virtual clEvent GetFinishedWriteEvent()=0;
	virtual clEvent GetFinishedReadEvent()=0;
};

#endif