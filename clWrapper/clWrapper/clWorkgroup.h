#ifndef CL_WORKGROUP_H
#define CL_WORKGROUP_H

// Specifiy number of threads to launch
class clWorkGroup
{
	public:
	clWorkGroup(unsigned int x, unsigned int y, unsigned int z)
	{
		worksize[0] = x;
		worksize[1] = y;
		worksize[2] = z;
	};

	clWorkGroup(size_t* workgroupsize)
	{
		worksize[0] = workgroupsize[0];
		worksize[1] = workgroupsize[1];
		worksize[2] = workgroupsize[2];
	};

	size_t worksize[3];
};

#endif