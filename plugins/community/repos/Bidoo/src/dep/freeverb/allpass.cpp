// Allpass filter implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#include "allpass.hpp"

namespace rack_plugin_Bidoo {

allpass::allpass()
{
	bufidx = 0;
	buffer = 0;
};

allpass::~allpass()
{
	//if (buffer) delete buffer;
};

void allpass::setbuffer(float *buf, int size)
{
	buffer = buf;
	bufsize = size;
}

void allpass::changebuffer(float *buf, int size)
	{
		if (buffer) {delete buffer;}
		buffer = new float[size];
		bufsize = size;
		bufidx = 0;
}

void allpass::mute()
{
	for (int i=0; i<bufsize; i++)
		buffer[i]=0;
}

void allpass::setfeedback(float val)
{
	feedback = val;
}

float allpass::getfeedback()
{
	return feedback;
}

} // namespace rack_plugin_Bidoo

//ends
