// Reverb model implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

// adapted for use in VCV Rack by Martin Lueders

#include "revmodel.hpp"

revmodel::revmodel()
{
};


void revmodel::init(const float sampleRate)
{

	conversion = sampleRate/44100.0;

	int ccombtuningL1 = round(conversion * combtuningL1);
	int ccombtuningR1 = round(conversion * combtuningR1);
	int ccombtuningL2 = round(conversion * combtuningL2);
	int ccombtuningR2 = round(conversion * combtuningR2);
	int ccombtuningL3 = round(conversion * combtuningL3);
	int ccombtuningR3 = round(conversion * combtuningR3);
	int ccombtuningL4 = round(conversion * combtuningL4);
	int ccombtuningR4 = round(conversion * combtuningR4);
	int ccombtuningL5 = round(conversion * combtuningL5);
	int ccombtuningR5 = round(conversion * combtuningR5);
	int ccombtuningL6 = round(conversion * combtuningL6);
	int ccombtuningR6 = round(conversion * combtuningR6);
	int ccombtuningL7 = round(conversion * combtuningL7);
	int ccombtuningR7 = round(conversion * combtuningR7);
	int ccombtuningL8 = round(conversion * combtuningL8);
	int ccombtuningR8 = round(conversion * combtuningR8);

	int callpasstuningL1 = round(conversion * allpasstuningL1);
	int callpasstuningR1 = round(conversion * allpasstuningR1);
	int callpasstuningL2 = round(conversion * allpasstuningL2);
	int callpasstuningR2 = round(conversion * allpasstuningR2);
	int callpasstuningL3 = round(conversion * allpasstuningL3);
	int callpasstuningR3 = round(conversion * allpasstuningR3);
	int callpasstuningL4 = round(conversion * allpasstuningL4);
	int callpasstuningR4 = round(conversion * allpasstuningR4);

	// Tie the components to their buffers
	combL[0].makebuffer(bufcombL1,ccombtuningL1);
	combR[0].makebuffer(bufcombR1,ccombtuningR1);
	combL[1].makebuffer(bufcombL2,ccombtuningL2);
	combR[1].makebuffer(bufcombR2,ccombtuningR2);
	combL[2].makebuffer(bufcombL3,ccombtuningL3);
	combR[2].makebuffer(bufcombR3,ccombtuningR3);
	combL[3].makebuffer(bufcombL4,ccombtuningL4);
	combR[3].makebuffer(bufcombR4,ccombtuningR4);
	combL[4].makebuffer(bufcombL5,ccombtuningL5);
	combR[4].makebuffer(bufcombR5,ccombtuningR5);
	combL[5].makebuffer(bufcombL6,ccombtuningL6);
	combR[5].makebuffer(bufcombR6,ccombtuningR6);
	combL[6].makebuffer(bufcombL7,ccombtuningL7);
	combR[6].makebuffer(bufcombR7,ccombtuningR7);
	combL[7].makebuffer(bufcombL8,ccombtuningL8);
	combR[7].makebuffer(bufcombR8,ccombtuningR8);

	allpassL[0].makebuffer(bufallpassL1,callpasstuningL1);
	allpassR[0].makebuffer(bufallpassR1,callpasstuningR1);
	allpassL[1].makebuffer(bufallpassL2,callpasstuningL2);
	allpassR[1].makebuffer(bufallpassR2,callpasstuningR2);
	allpassL[2].makebuffer(bufallpassL3,callpasstuningL3);
	allpassR[2].makebuffer(bufallpassR3,callpasstuningR3);
	allpassL[3].makebuffer(bufallpassL4,callpasstuningL4);
	allpassR[3].makebuffer(bufallpassR4,callpasstuningR4);

	feedback_allpass = 0.5;

	setwet(initialwet);
	setroomsize(initialroom);
	setdry(initialdry);
	setdamp(initialdamp);
	setwidth(initialwidth);
	setmode(initialmode);


	// Buffer will be full of rubbish - so we MUST mute them
	mute();
}


void revmodel::mute()
{
	if (getmode() >= freezemode)
		return;

	for (int i=0;i<numcombs;i++)
	{
		combL[i].mute();
		combR[i].mute();
	}
	for (int i=0;i<numallpasses;i++)
	{
		allpassL[i].mute();
		allpassR[i].mute();
	}
}

void revmodel::process(const float in, float &outputL, float &outputR)
{
	float outL,outR,input;

	{
		outL = outR = 0;
		input = in*gain*conversion;

		// Accumulate comb filters in parallel
		for(int i=0; i<numcombs; i++)
		{
			outL += combL[i].process(input, damp1, damp2, roomsize1);
			outR += combR[i].process(input, damp1, damp2, roomsize1);
		}

		// Feed through allpasses in series
		for(int i=0; i<numallpasses; i++)
		{
			outL = allpassL[i].process(outL, feedback_allpass);
			outR = allpassR[i].process(outR, feedback_allpass);
		}

		// Calculate output REPLACING anything already there
		outputL = outL; //*wet1 + outR*wet2;
		outputR = outR; //*wet1 + outL*wet2;

	}
}


void revmodel::update()
{
// Recalculate internal values after parameter change


	if (mode >= freezemode)
	{
		roomsize1 = 1;
		damp1 = 0;
		gain = muted;
	}
	else
	{
		roomsize1 = roomsize;
		damp1 = damp;
		gain = fixedgain;
	}

	damp2 = 1.0 - damp1;
}

// The following get/set functions are not inlined, because
// speed is never an issue when calling them, and also
// because as you develop the reverb model, you may
// wish to take dynamic action when they are called.

void revmodel::setroomsize(float value)
{
	roomsize = ((value*scaleroom) + offsetroom); // * conversion;
	update();
}

float revmodel::getroomsize()
{
//	return (roomsize/conversion-offsetroom)/scaleroom;
	return (roomsize-offsetroom)/scaleroom;
}

void revmodel::setdamp(float value)
{
	damp = value*scaledamp/conversion;
	damp = value*scaledamp * sqrt(conversion) ;
	update();
}

float revmodel::getdamp()
{
//	return conversion * damp/scaledamp;
	return damp/scaledamp;
}

void revmodel::setwet(float value)
{
	wet = value*scalewet;
	update();
}

float revmodel::getwet()
{
	return wet/scalewet;
}

void revmodel::setdry(float value)
{
	dry = value*scaledry;
}

float revmodel::getdry()
{
	return dry/scaledry;
}

void revmodel::setwidth(float value)
{
	width = value;
	update();
}

float revmodel::getwidth()
{
	return width;
}

void revmodel::setmode(float value)
{
	mode = value;
	update();
}

float revmodel::getmode()
{
	if (mode >= freezemode)
		return 1;
	else
		return 0;
}

//ends
