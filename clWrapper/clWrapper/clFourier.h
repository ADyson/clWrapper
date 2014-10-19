#ifndef CL_FOURIER_H
#define CL_FOURIER_H

#include "clWrapper.h"
#include "clFFT.h"

class clContext;
template <class T, template <class> class AutoPolicy> class clMemory;
template <class T> class Manual;

class clFourier
{
	friend class clContext;
	clFourier(clContext* _Context): Context(_Context){};
	clFourier(const clFourier &RHS): Context(RHS.Context){};
	~clFourier(void);

	clContext* Context;

	clfftStatus fftStatus;
	clfftSetupData fftSetupData;
	clfftPlanHandle fftplan;
	
	//intermediate buffer	
	boost::shared_ptr<clMemory<char,Manual>> clMedBuffer;
	cl_int medstatus;
	size_t buffersize;

	void Setup(int width, int height);

public:
	clEvent operator()(cl_mem &input, cl_mem &output, clfftDirection Dir);
	template <class T, template <class> class AutoPolicy, template <class> class AutoPolicy2> 
	clEvent operator()(boost::shared_ptr<clMemory<T,AutoPolicy2>>& input, boost::shared_ptr<clMemory<T,AutoPolicy>>& output, clfftDirection Dir);
};

#endif