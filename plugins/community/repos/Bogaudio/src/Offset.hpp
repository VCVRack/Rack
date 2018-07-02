#pragma once

#include "bogaudio.hpp"
#include "disable_output_limit.hpp"

extern Model* modelOffset;

namespace bogaudio {

struct Offset : DisableOutputLimitModule {
	enum ParamIds {
		OFFSET_PARAM,
		SCALE_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		OFFSET_INPUT,
		SCALE_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};

	Offset() : DisableOutputLimitModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}

	void step() override;

	float knobValue(const Param& knob, const Input& cv) const;
};

} // namespace bogaudio
