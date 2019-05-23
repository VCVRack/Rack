#pragma once

#include "bogaudio.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

namespace bogaudio {

struct MixerChannel {
	static const float maxDecibels;
	static const float minDecibels;
	static const float levelSlewTimeMS;
	static const float panSlewTimeMS;

	Amplifier _amplifier;
	Panner _panner;
	SlewLimiter _levelSL;
	SlewLimiter _panSL;
	RootMeanSquare _rms;

	Param& _levelParam;
	Param& _panParam;
	Param& _muteParam;
	Input& _inInput;
	Input& _levelInput;
	Input& _panInput;
	Input* _muteInput;

	float out = 0.0f;
	float left = 0.0f;
	float right = 0.0f;
	float rms = 0.0f;

	MixerChannel(
		Param& level,
		Param& pan,
		Param& mute,
		Input& in,
		Input& levelCv,
		Input& panCv,
		float sampleRate = 1000.0f,
		Input* muteCv = NULL
	)
	: _levelParam(level)
	, _panParam(pan)
	, _muteParam(mute)
	, _inInput(in)
	, _levelInput(levelCv)
	, _panInput(panCv)
	, _muteInput(muteCv)
	{
		setSampleRate(sampleRate);
		_rms.setSensitivity(0.05f);
	}

	void setSampleRate(float sampleRate);
	void next(bool stereo, bool solo); // input from _in; outputs on out, left, right, rms.
};

struct MuteButton : ToggleButton {
	MuteButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/button_18px_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/button_18px_1_orange.svg")));
	}
};

struct SoloMuteButton : ParamWidget, FramebufferWidget {
	std::vector<std::shared_ptr<SVG>> _frames;
	SVGWidget* _svgWidget; // deleted elsewhere.
	CircularShadow* shadow = NULL;

	SoloMuteButton();
	void step() override;
	void onMouseDown(EventMouseDown& e) override;
	void onChange(EventChange &e) override;
};

} // namespace bogaudio
