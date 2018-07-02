#pragma once

#include "bogaudio.hpp"
#include "mixer.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelMix8;

namespace bogaudio {

struct Mix8 : Module {
	enum ParamsIds {
		LEVEL1_PARAM,
		MUTE1_PARAM,
		PAN1_PARAM,
		LEVEL2_PARAM,
		MUTE2_PARAM,
		PAN2_PARAM,
		LEVEL3_PARAM,
		MUTE3_PARAM,
		PAN3_PARAM,
		LEVEL4_PARAM,
		MUTE4_PARAM,
		PAN4_PARAM,
		LEVEL5_PARAM,
		MUTE5_PARAM,
		PAN5_PARAM,
		LEVEL6_PARAM,
		MUTE6_PARAM,
		PAN6_PARAM,
		LEVEL7_PARAM,
		MUTE7_PARAM,
		PAN7_PARAM,
		LEVEL8_PARAM,
		MUTE8_PARAM,
		PAN8_PARAM,
		MIX_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		CV1_INPUT,
		PAN1_INPUT,
		IN1_INPUT,
		CV2_INPUT,
		PAN2_INPUT,
		IN2_INPUT,
		CV3_INPUT,
		PAN3_INPUT,
		IN3_INPUT,
		CV4_INPUT,
		PAN4_INPUT,
		IN4_INPUT,
		CV5_INPUT,
		PAN5_INPUT,
		IN5_INPUT,
		CV6_INPUT,
		PAN6_INPUT,
		IN6_INPUT,
		CV7_INPUT,
		PAN7_INPUT,
		IN7_INPUT,
		CV8_INPUT,
		PAN8_INPUT,
		IN8_INPUT,
		MIX_CV_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		L_OUTPUT,
		R_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	MixerChannel _channel1;
	MixerChannel _channel2;
	MixerChannel _channel3;
	MixerChannel _channel4;
	MixerChannel _channel5;
	MixerChannel _channel6;
	MixerChannel _channel7;
	MixerChannel _channel8;
	Amplifier _amplifier;
	SlewLimiter _slewLimiter;
	Saturator _saturator;
	RootMeanSquare _rms;
	float _rmsLevel = 0.0f;

	Mix8()
	: Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	, _channel1(params[LEVEL1_PARAM], params[PAN1_PARAM], params[MUTE1_PARAM], inputs[IN1_INPUT], inputs[CV1_INPUT], inputs[PAN1_INPUT])
	, _channel2(params[LEVEL2_PARAM], params[PAN2_PARAM], params[MUTE2_PARAM], inputs[IN2_INPUT], inputs[CV2_INPUT], inputs[PAN2_INPUT])
	, _channel3(params[LEVEL3_PARAM], params[PAN3_PARAM], params[MUTE3_PARAM], inputs[IN3_INPUT], inputs[CV3_INPUT], inputs[PAN3_INPUT])
	, _channel4(params[LEVEL4_PARAM], params[PAN4_PARAM], params[MUTE4_PARAM], inputs[IN4_INPUT], inputs[CV4_INPUT], inputs[PAN4_INPUT])
	, _channel5(params[LEVEL5_PARAM], params[PAN5_PARAM], params[MUTE5_PARAM], inputs[IN5_INPUT], inputs[CV4_INPUT], inputs[PAN5_INPUT])
	, _channel6(params[LEVEL6_PARAM], params[PAN6_PARAM], params[MUTE6_PARAM], inputs[IN6_INPUT], inputs[CV4_INPUT], inputs[PAN6_INPUT])
	, _channel7(params[LEVEL7_PARAM], params[PAN7_PARAM], params[MUTE7_PARAM], inputs[IN7_INPUT], inputs[CV4_INPUT], inputs[PAN7_INPUT])
	, _channel8(params[LEVEL8_PARAM], params[PAN8_PARAM], params[MUTE8_PARAM], inputs[IN8_INPUT], inputs[CV4_INPUT], inputs[PAN8_INPUT])
	{
		onSampleRateChange();
		_rms.setSensitivity(0.05f);
	}

	void onSampleRateChange() override;
	void step() override;
};

} // namespace bogaudio
