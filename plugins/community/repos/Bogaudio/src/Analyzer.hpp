#pragma once

#include "bogaudio.hpp"
#include "analyzer_base.hpp"

extern Model* modelAnalyzer;

namespace bogaudio {

struct Analyzer : AnalyzerBase {
	enum ParamsIds {
		RANGE_PARAM, // no longer used
		SMOOTH_PARAM,
		QUALITY_PARAM,
		POWER_PARAM,  // no longer used
		WINDOW_PARAM,
		RANGE2_PARAM,
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
		QUALITY_ULTRA_LIGHT,
		WINDOW_NONE_LIGHT,
		WINDOW_HAMMING_LIGHT,
		WINDOW_KAISER_LIGHT,
		NUM_LIGHTS
	};

	const int modulationSteps = 100;
	int _modulationStep = 0;

	Analyzer() : AnalyzerBase(4, NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}
	virtual ~Analyzer() {
		onReset();
	}

	void onReset() override;
	void onSampleRateChange() override;
	json_t* toJson() override;
	void fromJson(json_t* root) override;
	void step() override;
};

} // namespace bogaudio
