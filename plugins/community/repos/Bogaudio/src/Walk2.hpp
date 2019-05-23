#pragma once

#include <atomic>

#include "bogaudio.hpp"
#include "dsp/buffer.hpp"
#include "dsp/noise.hpp"
#include "dsp/signal.hpp"

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
		NUM_PARAMS
	};

	enum InputsIds {
		OFFSET_X_INPUT,
		SCALE_X_INPUT,
		RATE_X_INPUT,
		OFFSET_Y_INPUT,
		SCALE_Y_INPUT,
		RATE_Y_INPUT,
		JUMP_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		OUT_X_OUTPUT,
		OUT_Y_OUTPUT,
		DISTANCE_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	const int modulationSteps = 100;
	int _modulationStep = 0;

	const float historySeconds = 1.0f;
	const int historyPoints = 100;
	int _historySteps;
	int _historyStep = 0;

	float _offsetX = 0.0f, _offsetY = 0.0f;
	float _scaleX = 0.0f, _scaleY = 0.0f;
	RandomWalk _walkX, _walkY;
	SlewLimiter _slewX, _slewY;
	Trigger _jumpTrigger;
	HistoryBuffer<float> _outsX, _outsY;
	std::atomic<Vec*> _jumpTo;

	enum TraceColor {
		GREEN_TRACE_COLOR,
		ORANGE_TRACE_COLOR,
		RED_TRACE_COLOR,
		BLUE_TRACE_COLOR
	};
	bool _zoomOut = false;
	bool _drawGrid = true;
	TraceColor _traceColor = GREEN_TRACE_COLOR;

	Walk2()
	: Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	, _outsX(historyPoints, 0.0f)
	, _outsY(historyPoints, 0.0f)
	, _jumpTo(NULL)
	{
		onReset();
		onSampleRateChange();
	}

	void onReset() override;
	void onSampleRateChange() override;
	json_t* toJson() override;
	void fromJson(json_t* root) override;
	void step() override;
};

} // namespace bogaudio
