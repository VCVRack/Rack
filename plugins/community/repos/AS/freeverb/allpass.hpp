// Allpass filter declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

// adapted for use in VCV Rack by Martin Lueders

#ifndef _allpass_
#define _allpass_
#include "denormals.h"

class allpass
{
public:
	allpass()
	{
		bufidx = 0;
		buffer = 0;
	};

	~allpass()
	{
		if (buffer) delete buffer;
	};

        void    makebuffer(float *buf, int size)
        {
		if (buffer) delete buffer;
                buffer = new float[size];
                bufsize = size;
		bufidx = 0;
        }

        void    deletebuffer()
        {
                if(buffer) delete buffer;
                bufsize = 0;
        };

	void	setbuffer(float *buf, int size)
	{
		buffer = buf;
		bufsize = size;
	};

	inline  float	process(float inp, float feedback);
	void	mute()
	{
		for (int i=0; i<bufsize; i++) buffer[i] = 0.0;
	};

// private:
	float	*buffer;
	int	bufsize;
	int	bufidx;
};


// Big to inline - but crucial for speed

inline float allpass::process(float input, float feedback)
{
	float output;
	float bufout;
	
	bufout = buffer[bufidx];
//	undenormalise(bufout);
	
	output = -input + bufout;
	buffer[bufidx] = input + (bufout*feedback);

	if(++bufidx>=bufsize) bufidx = 0;

	return output;
}

#endif//_allpass

//ends
