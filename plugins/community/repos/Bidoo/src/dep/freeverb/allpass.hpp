// Allpass filter declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef _allpass_
#define _allpass_

namespace rack_plugin_Bidoo {

struct allpass
{
	allpass();
	~allpass();
	void	setbuffer(float *buf, int size);
	void	changebuffer(float *buf, int size);
	inline float process(float inp);
	void	mute();
	void	setfeedback(float val);
	float	getfeedback();
	float	feedback;
	float	*buffer;
	int		bufsize;
	int		bufidx;
};

inline float allpass::process(float input)
{
	float output;
	float bufout;

	bufout = buffer[bufidx];
	output = -input + bufout;
	buffer[bufidx] = input + (bufout*feedback);

	if(++bufidx>=bufsize) bufidx = 0;

	return output;
}

} // namespace rack_plugin_Bidoo

#endif
