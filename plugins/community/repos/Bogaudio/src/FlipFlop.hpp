#pragma once

#include "bogaudio.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelFlipFlop;

namespace bogaudio {

struct FlipFlop : Module {
	enum ParamsIds {
		NUM_PARAMS
	};

	enum InputsIds {
		IN1_INPUT,
		RESET1_INPUT,
		IN2_INPUT,
		RESET2_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		A1_OUTPUT,
		B1_OUTPUT,
		A2_OUTPUT,
		B2_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	bool _flipped1;
	bool _flipped2;
	PositiveZeroCrossing _trigger1;
	SchmittTrigger _resetTrigger1;
	PositiveZeroCrossing _trigger2;
	SchmittTrigger _resetTrigger2;

	FlipFlop() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void onReset() override;
	void step() override;
	void channelStep(
		Input& triggerInput,
		Input& resetInput,
		Output& aOutput,
		Output& bOutput,
		PositiveZeroCrossing& trigger,
		SchmittTrigger& resetTrigger,
		bool& flipped
	);
};

} // namespace bogaudio
