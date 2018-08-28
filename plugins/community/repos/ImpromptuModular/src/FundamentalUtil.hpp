//***********************************************************************************************
//Impromptu Modular: Modules for VCV Rack by Marc Boulé
//
//This is code from the Fundamental plugin by Andrew Belt 
//See ./LICENSE.txt for all licenses and see below for the filter code license
//***********************************************************************************************

#include "rack.hpp"
#include "dsp/functions.hpp"
#include "dsp/resampler.hpp"
#include "dsp/ode.hpp"
#include "dsp/filter.hpp"
#include "dsp/digital.hpp"


using namespace rack;

namespace rack_plugin_ImpromptuModular {

extern float sawTable[2048];// see end of file
extern float triTable[2048];// see end of file


// From Fundamental VCF
struct LadderFilter {
	float omega0;
	float resonance = 1.0f;
	float state[4];
	float input;
	float lowpass;
	float highpass;
	
	LadderFilter() {
		reset();
		setCutoff(0.f);
	}	
	void reset() {
		for (int i = 0; i < 4; i++) {
			state[i] = 0.f;
		}
	}
	void setCutoff(float cutoff) {
		omega0 = 2.f*M_PI * cutoff;
	}
	void process(float input, float dt);
};


// From Fundamental VCO.cpp
//template <int OVERSAMPLE, int QUALITY>
static const int OVERSAMPLE = 8;
static const int QUALITY = 8;
struct VoltageControlledOscillator {
	bool analog = false;
	bool soft = false;
	float lastSyncValue = 0.0f;
	float phase = 0.0f;
	float freq;
	float pw = 0.5f;
	float pitch;
	bool syncEnabled = false;
	bool syncDirection = false;

	Decimator<OVERSAMPLE, QUALITY> sinDecimator;
	Decimator<OVERSAMPLE, QUALITY> triDecimator;
	Decimator<OVERSAMPLE, QUALITY> sawDecimator;
	Decimator<OVERSAMPLE, QUALITY> sqrDecimator;
	RCFilter sqrFilter;

	// For analog detuning effect
	float pitchSlew = 0.0f;
	int pitchSlewIndex = 0;

	float sinBuffer[OVERSAMPLE] = {};
	float triBuffer[OVERSAMPLE] = {};
	float sawBuffer[OVERSAMPLE] = {};
	float sqrBuffer[OVERSAMPLE] = {};

	void setPitch(float pitchKnob, float pitchCv);
	void setPulseWidth(float pulseWidth);
	void process(float deltaTime, float syncValue);

	float sin() {
		return sinDecimator.process(sinBuffer);
	}
	float tri() {
		return triDecimator.process(triBuffer);
	}
	float saw() {
		return sawDecimator.process(sawBuffer);
	}
	float sqr() {
		return sqrDecimator.process(sqrBuffer);
	}
	float light() {
		return sinf(2*M_PI * phase);
	}
};



// From Fundamental LFO.cpp
struct LowFrequencyOscillator {
	float phase = 0.0f;
	float pw = 0.5f;
	float freq = 1.0f;
	bool offset = false;
	bool invert = false;
	SchmittTrigger resetTrigger;

	LowFrequencyOscillator() {}
	void setPitch(float pitch) {
		pitch = fminf(pitch, 8.0f);
		freq = powf(2.0f, pitch);
	}
	void setPulseWidth(float pw_) {
		const float pwMin = 0.01f;
		pw = clamp(pw_, pwMin, 1.0f - pwMin);
	}
	void setReset(float reset) {
		if (resetTrigger.process(reset / 0.01f)) {
			phase = 0.0f;
		}
	}
	void step(float dt) {
		float deltaPhase = fminf(freq * dt, 0.5f);
		phase += deltaPhase;
		if (phase >= 1.0f)
			phase -= 1.0f;
	}
	float sin() {
		if (offset)
			return 1.0f - cosf(2*M_PI * phase) * (invert ? -1.0f : 1.0f);
		else
			return sinf(2*M_PI * phase) * (invert ? -1.0f : 1.0f);
	}
	float tri(float x) {
		return 4.0f * fabsf(x - roundf(x));
	}
	float tri() {
		if (offset)
			return tri(invert ? phase - 0.5f : phase);
		else
			return -1.0f + tri(invert ? phase - 0.25f : phase - 0.75f);
	}
	float saw(float x) {
		return 2.0f * (x - roundf(x));
	}
	float saw() {
		if (offset)
			return invert ? 2.0f * (1.0f - phase) : 2.0f * phase;
		else
			return saw(phase) * (invert ? -1.0f : 1.0f);
	}
	float sqr() {
		float sqr = (phase < pw) ^ invert ? 1.0f : -1.0f;
		return offset ? sqr + 1.0f : sqr;
	}
	float light() {
		return sinf(2*M_PI * phase);
	}
};

} // namespace rack_plugin_ImpromptuModular
