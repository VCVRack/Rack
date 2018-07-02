// Comb filter class declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

// adapted for use in VCV Rack by Martin Lueders

#ifndef _comb_
#define _comb_

#include "denormals.h"

class comb
{
public:
	comb() 
	{
		buffer = 0;
		filterstore = 0;
		bufidx = 0;
	};

	~comb()
	{
		if (buffer) delete buffer;
	};

	void    makebuffer(float *buf, int size) 
	{
		if (buffer) {delete buffer;}
		buffer = new float[size];
		bufsize = size;
		bufidx = 0;
	}

	void	deletebuffer()
	{
		if(buffer) delete buffer;
		bufsize = 0;
	};

	void	setbuffer(float *buf, int size)
	{
		buffer = buf;
		bufsize = size;
	};

	inline  float	process(float inp, float damp1, float damp2, float feedback);
	void	mute()
	{
		for( int i=0; i<bufsize; i++) buffer[i]=0.0;
	};

private:
	float	filterstore;
	float	*buffer;
	int	bufsize;
	int	bufidx;
};


// Big to inline - but crucial for speed

inline float comb::process(float input, float damp1, float damp2, float feedback)
{
	float output;

	output = buffer[bufidx]; // y[n-K]
//	undenormalise(output);

	filterstore *= damp1;
	filterstore += (output*damp2);

//  filterstore = damp1*filterstore + damp2*output;
//  filterstore = damp1*filterstore + (1.0-damp1) * output;
//	filterstore = output + damp1*(filterstore - output);



//	undenormalise(filterstore);

	buffer[bufidx] = input + (filterstore*feedback);

	if(++bufidx>=bufsize) bufidx = 0;

	return output;
}

#endif //_comb_

//ends
