// Reverb model implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#include "revmodel.hpp"
#include <math.h>

namespace rack_plugin_Bidoo {

revmodel::revmodel()
{
	// Tie the components to their buffers
	combL[0].setbuffer(bufcombL1,combtuningL1);
	combR[0].setbuffer(bufcombR1,combtuningR1);
	combL[1].setbuffer(bufcombL2,combtuningL2);
	combR[1].setbuffer(bufcombR2,combtuningR2);
	combL[2].setbuffer(bufcombL3,combtuningL3);
	combR[2].setbuffer(bufcombR3,combtuningR3);
	combL[3].setbuffer(bufcombL4,combtuningL4);
	combR[3].setbuffer(bufcombR4,combtuningR4);
	combL[4].setbuffer(bufcombL5,combtuningL5);
	combR[4].setbuffer(bufcombR5,combtuningR5);
	combL[5].setbuffer(bufcombL6,combtuningL6);
	combR[5].setbuffer(bufcombR6,combtuningR6);
	combL[6].setbuffer(bufcombL7,combtuningL7);
	combR[6].setbuffer(bufcombR7,combtuningR7);
	combL[7].setbuffer(bufcombL8,combtuningL8);
	combR[7].setbuffer(bufcombR8,combtuningR8);
	allpassL[0].setbuffer(bufallpassL1,allpasstuningL1);
	allpassR[0].setbuffer(bufallpassR1,allpasstuningR1);
	allpassL[1].setbuffer(bufallpassL2,allpasstuningL2);
	allpassR[1].setbuffer(bufallpassR2,allpasstuningR2);
	allpassL[2].setbuffer(bufallpassL3,allpasstuningL3);
	allpassR[2].setbuffer(bufallpassR3,allpasstuningR3);
	allpassL[3].setbuffer(bufallpassL4,allpasstuningL4);
	allpassR[3].setbuffer(bufallpassR4,allpasstuningR4);

	// Set default values
	allpassL[0].setfeedback(0.5f);
	allpassR[0].setfeedback(0.5f);
	allpassL[1].setfeedback(0.5f);
	allpassR[1].setfeedback(0.5f);
	allpassL[2].setfeedback(0.5f);
	allpassR[2].setfeedback(0.5f);
	allpassL[3].setfeedback(0.5f);
	allpassR[3].setfeedback(0.5f);
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

void revmodel::processreplace(float *inputL, float *inputR, float *outputL, float *outputR, long numsamples, int skip)
{
	float outL,outR,input;

	while(numsamples-- > 0)
	{
		outL = outR = 0;
		input = (*inputL + *inputR) * gain;

		// Accumulate comb filters in parallel
		for(int i=0; i<numcombs; i++)
		{
			outL += combL[i].process(input);
			outR += combR[i].process(input);
		}

		// Feed through allpasses in series
		for(int i=0; i<numallpasses; i++)
		{
			outL = allpassL[i].process(outL);
			outR = allpassR[i].process(outR);
		}

		// Calculate output REPLACING anything already there
		*outputL = outL*wet1 + outR*wet2 + *inputL*dry;
		*outputR = outR*wet1 + outL*wet2 + *inputR*dry;

		// Increment sample pointers, allowing for interleave (if any)
		inputL += skip;
		inputR += skip;
		outputL += skip;
		outputR += skip;
	}
}

void revmodel::processmix(float *inputL, float *inputR, float *outputL, float *outputR, long numsamples, int skip)
{
	float outL,outR,input;

	while(numsamples-- > 0)
	{
		outL = outR = 0;
		input = (*inputL + *inputR) * gain;

		// Accumulate comb filters in parallel
		for(int i=0; i<numcombs; i++)
		{
			outL += combL[i].process(input);
			outR += combR[i].process(input);
		}

		// Feed through allpasses in series
		for(int i=0; i<numallpasses; i++)
		{
			outL = allpassL[i].process(outL);
			outR = allpassR[i].process(outR);
		}

		// Calculate output MIXING with anything already there
		*outputL += outL*wet1 + outR*wet2 + *inputL*dry;
		*outputR += outR*wet1 + outL*wet2 + *inputR*dry;

		// Increment sample pointers, allowing for interleave (if any)
		inputL += skip;
		inputR += skip;
		outputL += skip;
		outputR += skip;
	}
}

void revmodel::process(const float inL, const float inR, const float fbInL, const float fbInR, float &outputL, float &outputR, float &wOutputL, float &wOutputR)
{
	float outL = 0.0f, outR = 0.0f;
	float input = (inL + inR) * gain;
	// Accumulate comb filters in parallel
	for(int i=0; i<numcombs; i++)
	{
		outL += combL[i].process(input);
		outR += combR[i].process(input);
	}

	// Feed through allpasses in series
	for(int i=0; i<numallpasses; i++)
	{
		outL = allpassL[i].process((i==0 ? fbInL : 0.0f) + outL);
		outR = allpassR[i].process((i==0 ? fbInR : 0.0f) + outR);
	}

	outputL = outL*wet1 + outR*wet2 + inL*dry;
	outputR = outR*wet1 + outL*wet2 + inR*dry;
	wOutputL = outL*wet1 + outR*wet2;
	wOutputR = outR*wet1 + outL*wet2;
}

void revmodel::update()
{
// Recalculate internal values after parameter change

	int i;

	wet1 = wet*(width/2 + 0.5f);
	wet2 = wet*((1-width)/2);

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

	for(i=0; i<numcombs; i++)
	{
		combL[i].setfeedback(roomsize1);
		combR[i].setfeedback(roomsize1);
	}

	for(i=0; i<numcombs; i++)
	{
		combL[i].setdamp(damp1);
		combR[i].setdamp(damp1);
	}
}

// The following get/set functions are not inlined, because
// speed is never an issue when calling them, and also
// because as you develop the reverb model, you may
// wish to take dynamic action when they are called.

void revmodel::setroomsize(float value)
{
	roomsize = (value*scaleroom) + offsetroom;
	update();
}

float revmodel::getroomsize()
{
	return (roomsize-offsetroom)/scaleroom;
}

void revmodel::setdamp(float value)
{
	damp = value * scaledamp;
	update();
}

float revmodel::getdamp()
{
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

void revmodel::setsamplerate(const float samplerate) {

	sampleRate = samplerate;
	float coeff = sampleRate/44100.0;

	int mctL1 = round(coeff * combtuningL1);
	int mctR1 = round(coeff * combtuningR1);
	int mctL2 = round(coeff * combtuningL2);
	int mctR2 = round(coeff * combtuningR2);
	int mctL3 = round(coeff * combtuningL3);
	int mctR3 = round(coeff * combtuningR3);
	int mctL4 = round(coeff * combtuningL4);
	int mctR4 = round(coeff * combtuningR4);
	int mctL5 = round(coeff * combtuningL5);
	int mctR5 = round(coeff * combtuningR5);
	int mctL6 = round(coeff * combtuningL6);
	int mctR6 = round(coeff * combtuningR6);
	int mctL7 = round(coeff * combtuningL7);
	int mctR7 = round(coeff * combtuningR7);
	int mctL8 = round(coeff * combtuningL8);
	int mctR8 = round(coeff * combtuningR8);

	int maptL1 = round(coeff * allpasstuningL1);
	int maptR1 = round(coeff * allpasstuningR1);
	int maptL2 = round(coeff * allpasstuningL2);
	int maptR2 = round(coeff * allpasstuningR2);
	int maptL3 = round(coeff * allpasstuningL3);
	int maptR3 = round(coeff * allpasstuningR3);
	int maptL4 = round(coeff * allpasstuningL4);
	int maptR4 = round(coeff * allpasstuningR4);

	mute();

	combL[0].changebuffer(bufcombL1,mctL1);
	combR[0].changebuffer(bufcombR1,mctR1);
	combL[1].changebuffer(bufcombL2,mctL2);
	combR[1].changebuffer(bufcombR2,mctR2);
	combL[2].changebuffer(bufcombL3,mctL3);
	combR[2].changebuffer(bufcombR3,mctR3);
	combL[3].changebuffer(bufcombL4,mctL4);
	combR[3].changebuffer(bufcombR4,mctR4);
	combL[4].changebuffer(bufcombL5,mctL5);
	combR[4].changebuffer(bufcombR5,mctR5);
	combL[5].changebuffer(bufcombL6,mctL6);
	combR[5].changebuffer(bufcombR6,mctR6);
	combL[6].changebuffer(bufcombL7,mctL7);
	combR[6].changebuffer(bufcombR7,mctR7);
	combL[7].changebuffer(bufcombL8,mctL8);
	combR[7].changebuffer(bufcombR8,mctR8);

	allpassL[0].changebuffer(bufallpassL1,maptL1);
	allpassR[0].changebuffer(bufallpassR1,maptR1);
	allpassL[1].changebuffer(bufallpassL2,maptL2);
	allpassR[1].changebuffer(bufallpassR2,maptR2);
	allpassL[2].changebuffer(bufallpassL3,maptL3);
	allpassR[2].changebuffer(bufallpassR3,maptR3);
	allpassL[3].changebuffer(bufallpassL4,maptL4);
	allpassR[3].changebuffer(bufallpassR4,maptR4);

	setwet(initialwet);
	setroomsize(initialroom);
	setdry(initialdry);
	setdamp(initialdamp);
	setwidth(initialwidth);
	setmode(initialmode);

}

} // namespace rack_plugin_Bidoo

//ends
