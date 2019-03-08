#pragma once

#include "bogaudio.hpp"
#include "analyzer_base.hpp"

extern Model* modelAnalyzerXL;

namespace bogaudio {

struct AnalyzerXL : AnalyzerBase {
	enum ParamsIds {
		NUM_PARAMS
	};

	enum InputsIds {
		SIGNALA_INPUT,
		SIGNALB_INPUT,
		SIGNALC_INPUT,
		SIGNALD_INPUT,
		SIGNALE_INPUT,
		SIGNALF_INPUT,
		SIGNALG_INPUT,
		SIGNALH_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	const int modulationSteps = 100;
	int _modulationStep = 0;
	float _range = 0.0f;
	float _smooth = 0.25f;
	AnalyzerCore::Quality _quality = AnalyzerCore::QUALITY_GOOD;
	AnalyzerCore::Window _window = AnalyzerCore::WINDOW_KAISER;

	AnalyzerXL() : AnalyzerBase(8, NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void onReset() override;
	void onSampleRateChange() override;
	void setCoreParams();
	json_t* toJson() override;
	void fromJson(json_t* root) override;
	void step() override;
};

} // namespace bogaudio
