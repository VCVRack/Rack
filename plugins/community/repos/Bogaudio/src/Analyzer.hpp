#pragma once

#include "bogaudio.hpp"
#include "dsp/analyzer.hpp"

using namespace bogaudio::dsp;

extern Model* modelAnalyzer;

namespace bogaudio {

struct ChannelAnalyzer;

struct Analyzer : Module {
	enum ParamsIds {
		RANGE_PARAM,
		SMOOTH_PARAM,
		QUALITY_PARAM,
		POWER_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		SIGNALA_INPUT,
		SIGNALB_INPUT,
		SIGNALC_INPUT,
		SIGNALD_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		SIGNALA_OUTPUT,
		SIGNALB_OUTPUT,
		SIGNALC_OUTPUT,
		SIGNALD_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		QUALITY_HIGH_LIGHT,
		QUALITY_GOOD_LIGHT,
		POWER_ON_LIGHT,
		NUM_LIGHTS
	};

	enum Quality {
		QUALITY_HIGH,
		QUALITY_GOOD
	};

	bool _running = false;
	int _averageN;
	ChannelAnalyzer* _channelA = NULL;
	ChannelAnalyzer* _channelB = NULL;
	ChannelAnalyzer* _channelC = NULL;
	ChannelAnalyzer* _channelD = NULL;
	float _range = 0.0;
	float _smooth = 0.0;
	Quality _quality = QUALITY_GOOD;
	const SpectrumAnalyzer::Overlap _overlap = SpectrumAnalyzer::OVERLAP_2;
	const int _binAverageN = 2;

	Analyzer() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}
	virtual ~Analyzer() {
		onReset();
	}

	void onReset() override;
	void onSampleRateChange() override;
	void resetChannels();
	SpectrumAnalyzer::Size size();
	void step() override;
	void stepChannel(ChannelAnalyzer*& channelPointer, bool running, Input& input, Output& output);
};

} // namespace bogaudio
