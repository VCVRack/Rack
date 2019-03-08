#pragma once

#include "bogaudio.hpp"

extern Model* modelTest;

// #define LPF 1
// #define LPFNOISE 1
// #define SINE 1
// #define SQUARE 1
// #define SAW 1
// #define SATSAW 1
// #define TRIANGLE 1
// #define SAMPLED_TRIANGLE 1
// #define SINEBANK 1
// #define OVERSAMPLING 1
// #define OVERSAMPLED_BL 1
// #define ANTIALIASING 1
// #define DECIMATORS 1
#define INTERPOLATOR 1
// #define FM 1
// #define PM 1
// #define FEEDBACK_PM 1
// #define EG 1
// #define TABLES 1
// #define SLEW 1
// #define RMS 1
// #define RAVG 1
// #define SATURATOR 1

#include "pitch.hpp"
#ifdef LPF
#include "dsp/filter.hpp"
#elif LPFNOISE
#include "dsp/filter.hpp"
#include "dsp/noise.hpp"
#elif SINE
#include "dsp/oscillator.hpp"
#elif SQUARE
#include "dsp/oscillator.hpp"
#elif SAW
#include "dsp/oscillator.hpp"
#elif SATSAW
#include "dsp/oscillator.hpp"
#elif TRIANGLE
#include "dsp/oscillator.hpp"
#elif SAMPLED_TRIANGLE
#include "dsp/oscillator.hpp"
#elif SINEBANK
#include "dsp/oscillator.hpp"
#elif OVERSAMPLING
#include "dsp/oscillator.hpp"
#include "dsp/decimator.hpp" // rack
#include "dsp/filter.hpp"
#define OVERSAMPLEN 16
#elif OVERSAMPLED_BL
#include "dsp/oscillator.hpp"
#include "dsp/filter.hpp"
#elif ANTIALIASING
#include "dsp/oscillator.hpp"
#include "dsp/decimator.hpp" // rack
#include "dsp/filter.hpp"
#elif DECIMATORS
#include "dsp/oscillator.hpp"
#include "dsp/filter.hpp"
#include "dsp/decimator.hpp" // rack
#elif INTERPOLATOR
#include "dsp/oscillator.hpp"
#include "dsp/filter.hpp"
#elif FM
#include "dsp/oscillator.hpp"
#elif PM
#include "dsp/oscillator.hpp"
#elif FEEDBACK_PM
#include "dsp/oscillator.hpp"
#elif EG
#include "dsp/envelope.hpp"
#elif TABLES
#include "dsp/oscillator.hpp"
#elif SLEW
#include "dsp/signal.hpp"
#elif RMS
#include "dsp/signal.hpp"
#elif RAVG
#include "dsp/signal.hpp"
#elif SATURATOR
#include "dsp/oscillator.hpp"
#include "dsp/signal.hpp"
#else
#error what
#endif

using namespace bogaudio::dsp;

namespace bogaudio {

struct Test : Module {
	enum ParamsIds {
		PARAM1_PARAM,
		PARAM2_PARAM,
		PARAM3_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		CV1_INPUT,
		CV2_INPUT,
		CV3_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		OUT_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

#ifdef LPF
	LowPassFilter _lpf;
#elif LPFNOISE
	WhiteNoiseGenerator _noise;
	LowPassFilter _lpf;
#elif SINE
	SineOscillator _sine;
	SineTable _table;
	TablePhasor _sine2;
#elif SQUARE
	SquareOscillator _square;
	BandLimitedSquareOscillator _square2;
#elif SAW
	SawOscillator _saw;
	BandLimitedSawOscillator _saw2;
#elif SATSAW
	SaturatingSawOscillator _saw;
	BandLimitedSawOscillator _saw2;
#elif TRIANGLE
	TriangleOscillator _triangle;
#elif SAMPLED_TRIANGLE
	TriangleOscillator _triangle;
	TriangleOscillator _triangle2;
	int _sampleSteps = 1 << 20;
	int _sampleStep = 0;
	float _sample = 0.0f;
#elif SINEBANK
	SineBankOscillator _sineBank;
#elif OVERSAMPLING
	SawOscillator _saw1;
	SawOscillator _saw2;
	LowPassFilter _lpf;
	LowPassFilter _lpf2;
	rack::Decimator<OVERSAMPLEN, OVERSAMPLEN> _rackDecimator;
#elif OVERSAMPLED_BL
	BandLimitedSawOscillator _saw1;
	BandLimitedSawOscillator _saw2;
	LowPassFilter _lpf;
#elif ANTIALIASING
	#define OVERSAMPLEN 8
	Phasor _phasor;
	Phasor _oversampledPhasor;
	BandLimitedSawOscillator _saw;
	BandLimitedSquareOscillator _square;
	bogaudio::dsp::LPFDecimator _sawDecimator;
	bogaudio::dsp::LPFDecimator _squareDecimator;
	rack::Decimator<OVERSAMPLEN, OVERSAMPLEN> _sawRackDecimator;
	rack::Decimator<OVERSAMPLEN, OVERSAMPLEN> _squareRackDecimator;
#elif DECIMATORS
	#define OVERSAMPLEN 8
	#define STAGES 4
	BandLimitedSawOscillator _saw;
	bogaudio::dsp::CICDecimator _cicDecimator;
	bogaudio::dsp::LPFDecimator _lpfDecimator;
	rack::Decimator<OVERSAMPLEN, OVERSAMPLEN> _rackDecimator;
#elif INTERPOLATOR
	#define FACTOR 8
	#define STAGES 4
	BandLimitedSawOscillator _saw;
	bogaudio::dsp::CICDecimator _decimator;
	bogaudio::dsp::CICInterpolator _interpolator;
	int _steps;
	float _rawSamples[FACTOR] {};
	float _processedSamples[FACTOR] {};
#elif FM
	float _baseHz = 0.0f;
	float _ratio = 0.0f;
	float _index = 0.0f;
	float _sampleRate = 0.0f;
	SineTableOscillator _modulator;
	SineTableOscillator _carrier;
	SineTableOscillator _modulator2;
	SineTableOscillator _carrier2;
#elif PM
	SineTableOscillator _modulator;
	SineTableOscillator _carrier;
#elif FEEDBACK_PM
	SineTableOscillator _carrier;
	float _feedbackSample = 0.0f;
#elif EG
	ADSR _envelope;
#elif TABLES
	SineTableOscillator _sine;
	TablePhasor _table;
#elif SLEW
	SlewLimiter _slew;
	ShapedSlewLimiter _slew2;
#elif RMS
	RootMeanSquare _rms;
	PucketteEnvelopeFollower _pef;
#elif RAVG
	RunningAverage _average;
	Trigger _reset;
#elif SATURATOR
	Saturator _saturator;
#endif

	Test()
	: Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
#if SINE
	, _table(12)
	, _sine2(_table)
#elif DECIMATORS
	, _cicDecimator(STAGES)
#elif INTERPOLATOR
	, _decimator(STAGES)
	, _interpolator(STAGES)
#elif TABLES
  , _table(StaticBlepTable::table(), 44100.0, 1000.0)
#elif RAVG
	, _average(engineGetSampleRate(), 1.0f, 1000.0f)
#endif
	{
		onReset();

#ifdef SINE
		_table.generate();
		_sine2.setPhase(M_PI);

#elif SAW
		_saw2.setPhase(M_PI);

#elif SATSAW
		_saw2.setPhase(M_PI);

#elif SAMPLED_TRIANGLE
		_triangle2.setPhase(M_PI);

#elif SINEBANK
		const float baseAmplitude = 5.0;
		switch (5) {
			case 1: {
				// saw
				for (int i = 1, n = _sineBank.partialCount(); i <= n; ++i) {
					_sineBank.setPartial(i, i, baseAmplitude / (float)i);
				}
				_sineBank.syncToPhase(M_PI);
				break;
			}

			case 2: {
				// square
				for (int i = 1, n = _sineBank.partialCount(); i <= n; ++i) {
					_sineBank.setPartial(i, i, i % 2 == 1 ? baseAmplitude / (float)i : 0.0);
				}
				break;
			}

			case 3: {
				// triangle
				if (false) {
					for (int i = 1, n = _sineBank.partialCount(); i <= n; ++i) {
						_sineBank.setPartial(i, i, i % 2 == 1 ? baseAmplitude / (float)(i * i) : 0.0);
					}
					_sineBank.syncToPhase(M_PI / 2.0);
				}
				else {
					_sineBank.setPartial(1, 1.0f, baseAmplitude);
					for (int i = 2, n = _sineBank.partialCount(); i < n; ++i) {
						float k = 2*i - 1;
						_sineBank.setPartial(i, k, powf(-1.0f, k) * baseAmplitude/(k * k));
					}
				}
				break;
			}

			case 4: {
				// saw-square
				for (int i = 1, n = _sineBank.partialCount(); i <= n; ++i) {
					_sineBank.setPartial(i, i, i % 2 == 1 ? baseAmplitude / (float)i : baseAmplitude / (float)(2 * i));
				}
				break;
			}

			case 5: {
				// ?
				float factor = 0.717;
				float factor2 = factor;
				float multiple = 1.0;
				for (int i = 1, n = _sineBank.partialCount(); i <= n; ++i) {
					_sineBank.setPartial(i, multiple, baseAmplitude / multiple);
					multiple += i % 2 == 1 ? factor : factor2;
				}
				break;
			}
		}

#elif OVERSAMPLED_BL
		_saw2.setPhase(M_PI);
#endif
	}

	void onReset() override;
	void step() override;
	float oscillatorPitch(float max = 10000.0);
	float oscillatorPitch2(float max = 10000.0);
	float ratio2();
	float index3();
};

} // namespace bogaudio
