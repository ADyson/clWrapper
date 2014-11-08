#ifndef NOTIFY_H
#define NOTIFY_H

#include "clEvent.h"

class Notify abstract
{
public:
	virtual void Update(clEventPtr KernelFinished)=0;
	virtual void UpdateEventOnly(clEventPtr KernelFinished) = 0;
	virtual clEventPtr GetFinishedWriteEvent()=0;
	virtual clEventPtr GetFinishedReadEvent()=0;
};

#endif