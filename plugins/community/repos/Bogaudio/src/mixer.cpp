
#include "mixer.hpp"

const float MixerChannel::maxDecibels = 6.0f;
const float MixerChannel::minDecibels = Amplifier::minDecibels;
const float MixerChannel::levelSlewTimeMS = 5.0f;
const float MixerChannel::panSlewTimeMS = 10.0f;

void MixerChannel::setSampleRate(float sampleRate) {
	_levelSL.setParams(sampleRate, levelSlewTimeMS, maxDecibels - minDecibels);
	_panSL.setParams(sampleRate, panSlewTimeMS, 2.0f);
	_rms.setSampleRate(sampleRate);
}

void MixerChannel::next(bool stereo, bool solo) {
	if (!_inInput.active) {
		rms = out = left = right = 0.0f;
		return;
	}

	float mute = _muteParam.value;
	if (_muteInput) {
		mute += clamp(_muteInput->value, 0.0f, 10.0f);
	}
	bool muted = solo ? mute < 2.0f : mute > 0.5f;
	if (muted) {
		_amplifier.setLevel(_levelSL.next(minDecibels));
	}
	else {
		float level = clamp(_levelParam.value, 0.0f, 1.0f);
		if (_levelInput.active) {
			level *= clamp(_levelInput.value / 10.0f, 0.0f, 1.0f);
		}
		level *= maxDecibels - minDecibels;
		level += minDecibels;
		_amplifier.setLevel(_levelSL.next(level));
	}

	out = _amplifier.next(_inInput.value);
	rms = _rms.next(out / 5.0f);
	if (stereo) {
		float pan = clamp(_panParam.value, -1.0f, 1.0f);
		if (_panInput.active) {
			pan *= clamp(_panInput.value / 5.0f, -1.0f, 1.0f);
		}
		_panner.setPan(_panSL.next(pan));
		_panner.next(out, left, right);
	}
}


SoloMuteButton::SoloMuteButton() {
	shadow = new CircularShadow();
	addChild(shadow);

	_svgWidget = new SVGWidget();
	addChild(_svgWidget);

	auto svg = SVG::load(assetPlugin(plugin, "res/button_18px_0.svg"));
	_frames.push_back(svg);
	_frames.push_back(SVG::load(assetPlugin(plugin, "res/button_18px_1_orange.svg")));
	_frames.push_back(SVG::load(assetPlugin(plugin, "res/button_18px_1_green.svg")));
	_frames.push_back(SVG::load(assetPlugin(plugin, "res/button_18px_1_green.svg")));

	_svgWidget->setSVG(svg);
	box.size = _svgWidget->box.size;
	shadow->box.size = _svgWidget->box.size;
	shadow->blurRadius = 1.0;
	shadow->box.pos = Vec(0.0, 1.0);
}

void SoloMuteButton::step() {
	FramebufferWidget::step();
}

void SoloMuteButton::onMouseDown(EventMouseDown& e) {
	if (value >= 2.0f) {
		setValue(value - 2.0f);
	}
	else if (e.button == 1) { // right click
		setValue(value + 2.0f);
	}
	else {
		setValue(value > 0.5f ? 0.0f : 1.0f);
	}

	e.consumed = true;
	e.target = this;
}

void SoloMuteButton::onChange(EventChange &e) {
	assert(_frames.size() == 4);
	assert(value >= 0.0f && value <= 3.0f);
	_svgWidget->setSVG(_frames[(int)value]);
	dirty = true;
	ParamWidget::onChange(e);
}
