// Reverb model declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

// adapted for use in VCV Rack by Martin Lueders

#ifndef _revmodel_
#define _revmodel_

#include <math.h>

#include "comb.hpp"
#include "allpass.hpp"
#include "tuning.h"

class revmodel
{
public:
	revmodel();

	void	init(const float sampleRate);

	void	mute();

	void	process(const float input, float &outputL, float &outputR);

	void	setroomsize(float value);
	float	getroomsize();
	void	setdamp(float value);
	float	getdamp();
	void	setwet(float value);
	float	getwet();
	void	setdry(float value);
	float	getdry();
	void	setwidth(float value);
	float	getwidth();
	void	setmode(float value);
	float	getmode();
private:
	void	update();
private:
	float	gain;
	float	roomsize,roomsize1;
	float	damp,damp1, damp2;
	float   feedback_allpass;
	float	wet,wet1,wet2;
	float	dry;
	float	width;
	float	mode;

	float 	conversion;
	float math_e = 2.71828;

	// The following are all declared inline 
	// to remove the need for dynamic allocation
	// with its subsequent error-checking messiness

	// Comb filters
	comb	combL[numcombs];
	comb	combR[numcombs];

	// Allpass filters
	allpass	allpassL[numallpasses];
	allpass	allpassR[numallpasses];

	// Buffers for the combs
	float	*bufcombL1;
	float	*bufcombR1;
	float	*bufcombL2;
	float	*bufcombR2;
	float	*bufcombL3;
	float	*bufcombR3;
	float	*bufcombL4;
	float	*bufcombR4;
	float	*bufcombL5;
	float	*bufcombR5;
	float	*bufcombL6;
	float	*bufcombR6;
	float	*bufcombL7;
	float	*bufcombR7;
	float	*bufcombL8;
	float	*bufcombR8;

	// Buffers for the allpasses
	float	*bufallpassL1;
	float	*bufallpassR1;
	float	*bufallpassL2;
	float	*bufallpassR2;
	float	*bufallpassL3;
	float	*bufallpassR3;
	float	*bufallpassL4;
	float	*bufallpassR4;
};

#endif//_revmodel_

//ends
