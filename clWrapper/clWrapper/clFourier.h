#ifndef CL_FOURIER_H
#define CL_FOURIER_H

#include "clContext.h"
#include "CL/Opencl.h"
#include "boost/shared_ptr.hpp"
#include "clFFT.h"
#include "clEvent.h"
#include "clMemory.h"

class AutoTeardownFFT;
template <class T, template <class> class AutoPolicy> class clMemory;
template <class T> class Manual;

// Can't use scoped enums for legacy code
namespace Direction
{
	enum TransformDirection
	{
		Forwards,
		Inverse
	};
};

class clFourier
{
	clContext* Context;
	clfftStatus fftStatus;
	clfftSetupData fftSetupData;
	clfftPlanHandle fftplan;
	
	//intermediate buffer	
	boost::shared_ptr<clMemory<char,Manual>> clMedBuffer;
	cl_int medstatus;
	size_t buffersize;	
	void Setup(int width, int height);
	int width,height;

public:

	clFourier(clContext &Context, int _width, int _height);

	clFourier(const clFourier &RHS): Context(RHS.Context), width(RHS.width), height(RHS.height){
		Setup(width,height);
	};

	clFourier& operator=(const clFourier &RHS){
		if(this != &RHS){
			clfftDestroyPlan(&fftplan);
			width = RHS.width;
			height = RHS.height;
			Setup(width,height);
		}		
		return *this;
	};

	~clFourier(void);
	template <class T, template <class> class AutoPolicy, template <class> class AutoPolicy2> 
	clEvent operator()(boost::shared_ptr<clMemory<T,AutoPolicy2>>& input, boost::shared_ptr<clMemory<T,AutoPolicy>>& output, Direction::TransformDirection Direction)
	{
		clfftDirection Dir = (Direction == Direction::Forwards) ? CLFFT_FORWARD : CLFFT_BACKWARD;
		
		std::vector<cl_event> eventwaitlist;
		clEvent e = input->GetFinishedWriteEvent();
		clEvent e2 = input->GetFinishedReadEvent();
		if (e.isSet())
		{
			eventwaitlist.push_back(e.event);
		}
		if (e2.isSet())
		{
			eventwaitlist.push_back(e2.event);
		}

		clEvent finished;

		if(buffersize)
			fftStatus = clfftEnqueueTransform( fftplan, Dir, 1, &Context->GetQueue(), eventwaitlist.size(),eventwaitlist.size() ? &eventwaitlist[0] : NULL, &finished.event, 
				&input->GetBuffer(), &output->GetBuffer(), clMedBuffer->GetBuffer() );
		else
			fftStatus = clfftEnqueueTransform( fftplan, Dir, 1, &Context->GetQueue(), eventwaitlist.size(),eventwaitlist.size() ? &eventwaitlist[0] : NULL, &finished.event, 
				&input->GetBuffer(), &output->GetBuffer(), NULL );
	
		finished.Set();

		if(output->isAuto)
			output->Update(finished);

		return finished;
	}
};

// Singleton to auto call clfftteardown on program termination
class AutoTeardownFFT
{
private:
	AutoTeardownFFT(){};
	AutoTeardownFFT(AutoTeardownFFT const& copy);
public:
	~AutoTeardownFFT(){ clfftTeardown(); }
	static AutoTeardownFFT& GetInstance() { static AutoTeardownFFT instance; return instance; }
};

#endif

