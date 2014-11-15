#ifndef MANUAL_H
#define MANUAL_H

#include "clEvent.h"
#include "CL/Opencl.h"

// This class is inherited by OpenCL memory buffers that have to manage there own memory lifetimes.
template <class T> class Manual abstract : public Notify
{
private:
	const bool	isAuto;
	size_t		SizeX;
	size_t		SizeY;
	size_t		SizeZ;
	size_t		Dimensionality;
	clEventPtr	KernelFinished;

public:
	Manual<T>(size_t sizex): SizeX(sizex), SizeY(1), SizeZ(1), isAuto(false), Dimensionality(1) {};
	Manual<T>(size_t sizex, size_t sizey): SizeX(sizex), SizeY(sizey), SizeZ(1), isAuto(false), Dimensionality(2) {};
	Manual<T>(size_t sizex, size_t sizey, size_t sizez): SizeX(sizex), SizeY(sizey), SizeZ(sizez), isAuto(false), Dimensionality(3) {};

			bool		IsAuto(){ return isAuto; };
			size_t		GetSize() { return SizeX*SizeY*SizeZ };
			size_t		GetSizeX() { return SizeX; };
			size_t		GetSizeY() { return SizeY; };
			size_t		GetSizeZ() { return SizeZ; };
			size_t		GetDimensionality() { return Dimensionality; };
			void		Update(clEventPtr _KernelFinished){KernelFinished = _KernelFinished;};
	virtual clEventPtr	Read(std::vector<T>&data)=0;
	virtual clEventPtr	Read(std::vector<T>&data,clEventPtr KernelFinished)=0;
	virtual clEventPtr	GetStartWriteEvent()=0;
	virtual clEventPtr	GetStartReadEvent()=0;
	virtual clEventPtr	GetFinishedWriteEvent()=0;
	virtual clEventPtr	GetFinishedReadEvent()=0;
	virtual void		SetFinishedEvent(clEventPtr KernelFinished)=0;

	// This will create a vector filled with the current contents of the memory
	// Will block until the read has been completed
	std::vector<T> CreateLocalCopy()
	{
		std::vector<T> Local(GetSize());
		if(KernelFinished->GetEvent() != NULL)
		{
			clEventPtr e = Read(Local, KernelFinished);
			e->Wait();
		}
		else
		{
			clEventPtr e = Read(Local);
			e->Wait();
		}
		return Local;
	};

	void UpdateEventOnly(clEventPtr KernelFinished)
	{
		SetFinishedEvent(KernelFinished);
	};
};

#endif 