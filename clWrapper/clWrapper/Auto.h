#ifndef AUTO_H
#define AUTO_H

#include "clEvent.h"
#include "Notify.h"
#include <vector>

// This class can facilitate automatically retrieving changes to OpenCL memory buffers.
// from kernels with argument types specified.
template <class T> class Auto abstract : public Notify
{
private:
	size_t SizeX;
	size_t SizeY;
	size_t SizeZ;
	size_t Dimensionality;
	bool isAuto;
protected:
	bool isUpToDate;

	// Reading directly from Local is unadvised as it will be updated whenever the kernel 
	// actually completes which is not in general, directly after it has been called as kernel
	// enqueue is non blocking. Use GetLocal() to force waiting for current version if it is not already updated.
	std::vector<T> Local;

public:
	Auto<T>(size_t sizex): SizeX(sizex), SizeY(1), SizeZ(1), Dimensionality(1), isAuto(true), isUpToDate(true) { Local.resize(0); };
	Auto<T>(size_t sizex, size_t sizey): SizeX(sizex), SizeY(sizey), SizeZ(1), Dimensionality(2), isAuto(true), isUpToDate(true) { Local.resize(0); };
	Auto<T>(size_t sizex, size_t sizey, size_t sizez): SizeX(sizex), SizeY(sizey), SizeZ(sizez), Dimensionality(3), isAuto(true), isUpToDate(true) { Local.resize(0); };

	bool IsAuto(){ return isAuto; };
	size_t GetSize(){ return SizeX*SizeY*SizeZ; };
	size_t GetSizeX() { return SizeX; };
	size_t GetSizeY() { return SizeY; };
	size_t GetSizeZ() { return SizeZ; };
	size_t GetDimensionality() { return Dimensionality; };
	virtual clEventPtr Read(std::vector<T>&data)=0;
	virtual clEventPtr Read(std::vector<T>&data,clEventPtr KernelFinished)=0;
	virtual clEventPtr GetStartWriteEvent()=0;
	virtual clEventPtr GetStartReadEvent()=0;
	virtual clEventPtr GetFinishedWriteEvent()=0;
	virtual clEventPtr GetFinishedReadEvent()=0;
	virtual void SetFinishedEvent(clEventPtr KernelFinished) =0;
		
	// This call will block if the Memory is currently waiting on
	// an event before updating itself.
	std::vector<T>& GetLocal()
	{	
		WaitForRead();
		return Local;
	};

	// Called by clKernel for Output types to generate automatic
	// memory updates (non blocking)
	void Update(clEventPtr KernelFinished)
	{
		if(Local.empty() == true || Local.size() != GetSize())
			Local.resize(GetSize());
		Read(Local,KernelFinished);
		isUpToDate = true;
	}

	void UpdateEventOnly(clEventPtr KernelFinished)
	{
		isUpToDate = false;
		SetFinishedEvent(KernelFinished);
	};

	T& Get(int xpos) { return Get(xpos,0,0); };
	T& Get(int xpos, int ypos) { return Get(xpos,ypos,0); };
	T& Get(int xpos, int ypos, int zpos) { ValidateInput(xpos,ypos,zpos); WaitForRead(); return Local[xpos + SizeX*ypos + SizeX*SizeY*zpos]; };

private:

	void WaitForRead() {
		clEventPtr es = GetStartReadEvent();
		clEventPtr e = GetFinishedReadEvent();

		if(es->isSet())
			es->Wait();

		if(isUpToDate == false) 
		{
			Update(es);
			isUpToDate = true;

			if((es = GetFinishedReadEvent())->isSet())
				es->Wait();
		} 
		else if(e->isSet()) 
			e->Wait();
	};

	void ValidateInput(int xpos, int ypos, int zpos)
	{
		bool bValid = true;

		if(xpos < 0 || xpos >= SizeX) bValid = false;
		if(ypos < 0 || ypos >= SizeY) bValid = false;
		if(zpos < 0 || zpos >= SizeZ) bValid = false;

		if(bValid = false) throw new std::exception("Index out of bounds");
	};
};

#endif