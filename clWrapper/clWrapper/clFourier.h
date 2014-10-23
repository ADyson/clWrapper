#ifndef CL_FOURIER_H
#define CL_FOURIER_H

#include "clWrapper.h"
#include "clFFT.h"

class clContext;
class AutoTeardownFFT;
template <class T, template <class> class AutoPolicy> class clMemory;
template <class T> class Manual;

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
	//clFourier(const clFourier &RHS): Context(RHS.Context), fftSetupData(RHS.fftSetupData), fftplan(RHS.fftplan), clMedBuffer(RHS.clMedBuffer), buffersize(RHS.buffersize){};
	~clFourier(void);
	template <class T, template <class> class AutoPolicy, template <class> class AutoPolicy2> 
	clEvent operator()(boost::shared_ptr<clMemory<T,AutoPolicy2>>& input, boost::shared_ptr<clMemory<T,AutoPolicy>>& output, clfftDirection Dir)
	{
		clEvent finished;

		if(buffersize)
			fftStatus = clfftEnqueueTransform( fftplan, Dir, 1, &Context->GetQueue(), 0, NULL, &finished.event, 
				&input->GetBuffer(), &output->GetBuffer(), clMedBuffer->GetBuffer() );
		else
			fftStatus = clfftEnqueueTransform( fftplan, Dir, 1, &Context->GetQueue(), 0, NULL, &finished.event, 
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

