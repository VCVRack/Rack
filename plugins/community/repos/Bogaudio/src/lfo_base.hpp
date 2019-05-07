
#pragma once

#include "bogaudio.hpp"
#include "dsp/oscillator.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

namespace bogaudio {

struct PitchModeListener {
	virtual void pitchModeChanged() = 0;
};

struct LFOBase : Module {
	enum PitchMode {
		UNKNOWN_PITCH_MODE,
		CLASSIC_PITCH_MODE,
		COMPLIANT_PITCH_MODE
	};

	PitchMode _pitchMode = UNKNOWN_PITCH_MODE;
	PitchModeListener* _pitchModeListener = NULL;

	LFOBase(int np, int ni, int no, int nl) : Module(np, ni, no, nl) {}

	json_t* toJson() override;
	void fromJson(json_t* root) override;
	bool isCompliantPitchMode() { return _pitchMode == COMPLIANT_PITCH_MODE; }
	void setPitchMode(PitchMode mode);
	void setPitchModeListener(PitchModeListener* listener) { _pitchModeListener = listener; }
	void setFrequency(bool slow, Param& frequency, Input& pitch, Phasor& phasor);
};

struct PitchModeMenuItem : MenuItem {
	LFOBase* _module;
	bool _compliant;

	PitchModeMenuItem(LFOBase* module, const char* label, bool compliant)
	: _module(module)
	, _compliant(compliant)
	{
		this->text = label;
	}

	void onAction(EventAction &e) override {
		_module->setPitchMode(_compliant ? LFOBase::COMPLIANT_PITCH_MODE : LFOBase::CLASSIC_PITCH_MODE);
	}

	void step() override {
		bool compliantMode = _module->isCompliantPitchMode();
		bool on = _compliant ? compliantMode : !compliantMode;
		rightText = on ? "✔" : "";
	}
};

struct LFOBaseWidget : ModuleWidget, PitchModeListener {
	LFOBase* _module;
	SVGPanel* _panel;
	std::shared_ptr<SVG> _classicSVG;
	std::shared_ptr<SVG> _compliantSVG;
	SVGKnob* _frequencyKnob = NULL;

	LFOBaseWidget(
		LFOBase* module,
		SVGPanel* panel,
		std::shared_ptr<SVG> classicSVG,
		std::shared_ptr<SVG> compliantSVG
	)
	: ModuleWidget(module)
	, _module(module)
	, _panel(panel)
	, _classicSVG(classicSVG)
	, _compliantSVG(compliantSVG)
	{
		setSVG();
		_module->setPitchModeListener(this);
	}

	void setSVG() {
		if (_module->isCompliantPitchMode()) {
			_panel->setBackground(_compliantSVG);
			if (_frequencyKnob) {
				_frequencyKnob->minValue = -5.0f;
				_frequencyKnob->maxValue = 8.0f;
				_frequencyKnob->dirty = true;
			}
		}
		else {
			_panel->setBackground(_classicSVG);
			if (_frequencyKnob) {
				_frequencyKnob->minValue = -8.0f;
				_frequencyKnob->maxValue = 5.0f;
				_frequencyKnob->dirty = true;
			}
		}
		_panel->dirty = true;
	}

	void pitchModeChanged() override {
		setSVG();
	}

	void appendContextMenu(Menu* menu) override {
		LFOBase* lfo = dynamic_cast<LFOBase*>(module);
		assert(lfo);
		menu->addChild(new MenuLabel());
		menu->addChild(new PitchModeMenuItem(lfo, "Classic pitch mode: 0V = C0 = 16.35HZ", false));
		menu->addChild(new PitchModeMenuItem(lfo, "Standard pitch mode: 0V = C-3 = 2.04HZ", true));
	}
};

} // namespace bogaudio
