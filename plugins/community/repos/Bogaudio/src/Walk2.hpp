#pragma once

#include "bogaudio.hpp"
#include "dsp/buffer.hpp"
#include "dsp/noise.hpp"

using namespace bogaudio::dsp;

extern Model* modelWalk2;

namespace bogaudio {

struct Walk2 : Module {
	enum ParamsIds {
		RATE_X_PARAM,
		RATE_Y_PARAM,
		OFFSET_X_PARAM,
		OFFSET_Y_PARAM,
		SCALE_X_PARAM,
		SCALE_Y_PARAM,
		TRACK_X_PARAM,
		TRACK_Y_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		HOLD_X_INPUT,
		RATE_X_INPUT,
		HOLD_Y_INPUT,
		RATE_Y_INPUT,
		JUMP_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		HOLD_X_OUTPUT,
		OUT_X_OUTPUT,
		HOLD_Y_OUTPUT,
		OUT_Y_OUTPUT,
		DISTANCE_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		TRACK_X_LIGHT,
		TRACK_Y_LIGHT,
		NUM_LIGHTS
	};

	const int modulationSteps = 100;
	int _modulationStep = 0;

	const float historySeconds = 1.0f;
	const int historyPoints = 100;
	int _historySteps;
	int _historyStep = 0;

	Trigger _triggerX, _triggerY;
	RandomWalk _walkX, _walkY;
	Trigger _jumpTrigger;
	float _holdX = 0.0f, _holdY = 0.0f;
	HistoryBuffer<float> _outsX, _outsY, _holdsX, _holdsY;

	Walk2()
	: Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	, _outsX(historyPoints, 0.0f)
	, _outsY(historyPoints, 0.0f)
	, _holdsX(historyPoints, 0.0f)
	, _holdsY(historyPoints, 0.0f)
	{
		onReset();
		onSampleRateChange();
	}

	void onReset() override;
	void onSampleRateChange() override;
	void step() override;
};

} // namespace bogaudio
