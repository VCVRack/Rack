#include "AH.hpp"
#include "Core.hpp"
#include "UI.hpp"
#include "VCO.hpp"
#include "dsp/digital.hpp"
#include "dsp/resampler.hpp"
#include "dsp/filter.hpp"

#include <iostream>

namespace rack_plugin_AmalgamatedHarmonics {

struct Chord : AHModule {

	const static int NUM_PITCHES = 6;

	enum ParamIds {
		ENUMS(WAVE_PARAM,6),
		ENUMS(OCTAVE_PARAM,6),
		ENUMS(DETUNE_PARAM,6),
		ENUMS(PW_PARAM,6),
		ENUMS(PWM_PARAM,6),
		ENUMS(ATTN_PARAM,6),
		ENUMS(PAN_PARAM,6),
		SPREAD_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(PITCH_INPUT,6),
		ENUMS(PW_INPUT,6),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(OUT_OUTPUT,2),
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	
	Chord() : AHModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override;
		
	Core core;

	int poll = 50000;

	SchmittTrigger moveTrigger;
	PulseGenerator triggerPulse;

	EvenVCO oscillator[6];

};

void Chord::step() {
	
	AHModule::step();

	float out[2] = {0.0f, 0.0f};
	int nP[2] = {0, 0};

	float spread = params[SPREAD_PARAM].value;
	float SQRT2_2 = sqrt(2.0) / 2.0;

	for (int i = 0; i < NUM_PITCHES; i++) {

		int index = PITCH_INPUT + i;
		int side = i % 2;

		float pitchCv = inputs[index].value + params[OCTAVE_PARAM + i].value;
		float pitchFine = params[DETUNE_PARAM + i].value / 12.0; // +- 1V
		float attn = params[ATTN_PARAM + i].value;
		oscillator[i].pw = params[PW_PARAM + i].value + params[PWM_PARAM + i].value * inputs[PW_INPUT + i].value / 10.0f;
		oscillator[i].step(delta, pitchFine + pitchCv); // 1V/OCT

		if (inputs[index].active) {

			float amp = 0.0;
			nP[side]++;

			int wave = params[WAVE_PARAM + i].value;
			switch(wave) {
				case 0:		amp = oscillator[i].sine * attn;		break;
				case 1:		amp = oscillator[i].saw * attn;			break;
				case 2:		amp = oscillator[i].doubleSaw * attn;	break;
				case 3:		amp = oscillator[i].square * attn;		break;
				case 4:		amp = oscillator[i].even * attn;		break;
				default:	amp = oscillator[i].sine * attn;		break;
			};

			float angle = spread * params[PAN_PARAM + i].value;
			float left = SQRT2_2 * (cos(angle) - sin(angle));
    		float right = SQRT2_2 * (cos(angle) + sin(angle));

			out[0] += left * amp;
			out[1] += right * amp;

		}
	}
	
	if (nP[0] > 0) {
		out[0] = (out[0] * 5.0f) / (float)nP[0];
	} 

	if (nP[1] > 0) {
		out[1] = (out[1] * 5.0f) / (float)nP[1];
	} 

	// std::cout << nPitches << " " << out[0] << " " << out[1] << std::endl;

	if (outputs[OUT_OUTPUT].active && outputs[OUT_OUTPUT + 1].active) {
		outputs[OUT_OUTPUT].value 		= out[0];
		outputs[OUT_OUTPUT + 1].value 	= out[1];
	} else if (!outputs[OUT_OUTPUT].active && outputs[OUT_OUTPUT + 1].active) {
		outputs[OUT_OUTPUT].value 		= 0.0f;
		outputs[OUT_OUTPUT + 1].value 	= (out[0] + out[1]) / 2.0f;
	} else if (outputs[OUT_OUTPUT].active && !outputs[OUT_OUTPUT + 1].active) {
		outputs[OUT_OUTPUT].value 		= (out[0] + out[1]) / 2.0f;
		outputs[OUT_OUTPUT + 1].value 	= 0.0f;
	}

}

struct ChordWidget : ModuleWidget {

	ChordWidget(Chord *module) : ModuleWidget(module) {
		
		UI ui;
		
		float PI_180 = M_PI / 180.0f;
		float posMax = 90.0 * 0.5 * PI_180;
		float voicePosDeg[6] = {-90.0f, 90.0f, -54.0f, 54.0f, -18.0f, 18.0f};
		float voicePosRad[6];	
		for(int i = 0; i < 6; i++) {
			voicePosRad[i] = voicePosDeg[i] * 0.5 * PI_180;
		}

		box.size = Vec(270, 380);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/Chord.svg")));
			addChild(panel);
		}

		for (int n = 0; n < 6; n++) {
			addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, n, 0, true, true), Port::INPUT, module, Chord::PITCH_INPUT + n));
			addParam(ParamWidget::create<AHKnobSnap>(ui.getPosition(UI::KNOB, n, 1, true, true), module, Chord::WAVE_PARAM + n, 0.0f, 4.0f, 0.0f));
			addParam(ParamWidget::create<AHKnobSnap>(ui.getPosition(UI::KNOB, n, 2, true, true), module, Chord::OCTAVE_PARAM + n, -3.0f, 3.0f, 0.0f));
			addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, n, 3, true, true), module, Chord::DETUNE_PARAM + n, -1.0f, 1.0f, 0.0f));
			addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, n, 4, true, true), module, Chord::PW_PARAM + n, -1.0f, 1.0f, 0.0f));
			addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, n, 5, true, true), Port::INPUT, module, Chord::PW_INPUT + n));
			addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, n, 6, true, true), module, Chord::PWM_PARAM + n, 0.0f, 1.0f, 0.0f));
			addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, n, 7, true, true), module, Chord::ATTN_PARAM + n, 0.0f, 1.0f, 1.0f));
			addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, n, 8, true, true), module, Chord::PAN_PARAM + n, -posMax, posMax, voicePosRad[n]));
		}

		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 0, 9, true, true), module, Chord::SPREAD_PARAM, 0.0f, 1.0f, 1.0f));

		addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 4, 9, true, true), Port::OUTPUT, module, Chord::OUT_OUTPUT));
		addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 5, 9, true, true), Port::OUTPUT, module, Chord::OUT_OUTPUT + 1));

	}
};

} // namespace rack_plugin_AmalgamatedHarmonics

using namespace rack_plugin_AmalgamatedHarmonics;

RACK_PLUGIN_MODEL_INIT(AmalgamatedHarmonics, Chord) {
   Model *modelChord = Model::create<Chord, ChordWidget>( "Amalgamated Harmonics", "Chord", "D'acchord", OSCILLATOR_TAG);
   return modelChord;
}

// ♯♭
