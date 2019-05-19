#include "MMFilter.h"

void MMFilter::setfreq(float freq)
{
    cutoff = freq;
    calculateFeedbackAmount();
}
void MMFilter::setreso(float reso)
{
    resonance = reso;
    if (resonance > 0.99)
    {
        resonance = 0.99;
    }
    else if (resonance < 0.0)
    {
        resonance = 0.0;
    }
    calculateFeedbackAmount();
}
void MMFilter::setinput(float input)
{
    inputValue = input;
}
void MMFilter::setmode(int fMode)
{
    if (fMode == 0)
    {
        mode = FILTER_MODE_LOWPASS;
    }
    if (fMode == 1)
    {
        mode = FILTER_MODE_BANDPASS;
    }
    if (fMode == 2)
    {
        mode = FILTER_MODE_HIGHPASS;
    }
    if (fMode == 3)
    {
        mode = FILTER_MODE_NOTCH;
    }
}
float  MMFilter::getoutput() {

	if (inputValue == 0.0) return inputValue;
	float calculatedCutoff = getCalculatedCutoff();;
	buf0 += calculatedCutoff * (inputValue - buf0 + feedbackAmount * (buf0 - buf1));
	buf1 += calculatedCutoff * (buf0 - buf1);
	buf2 += calculatedCutoff * (buf1 - buf2);
	buf3 += calculatedCutoff * (buf2 - buf3);
	switch (mode) {
	case FILTER_MODE_LOWPASS:
		return buf3;
	case FILTER_MODE_HIGHPASS:
		return inputValue - buf3;
	case FILTER_MODE_BANDPASS:
		return buf0 - buf3;
	case FILTER_MODE_NOTCH:
		return inputValue - (buf0 - buf3);
	default:
		return buf3;
	}
}
