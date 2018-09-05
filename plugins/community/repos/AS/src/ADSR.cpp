//**************************************************************************************
//ADSR module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//Code taken from the Fundamentals plugins by Andrew Belt http://www.vcvrack.com
//**************************************************************************************
#include "AS.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_AS {

struct ADSR : Module {
	enum ParamIds {
		ATTACK_PARAM,
		DECAY_PARAM,
		SUSTAIN_PARAM,
		RELEASE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ATTACK_INPUT,
		DECAY_INPUT,
		SUSTAIN_INPUT,
		RELEASE_INPUT,
		GATE_INPUT,
		TRIG_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENVELOPE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ATTACK_LIGHT,
		DECAY_LIGHT,
		SUSTAIN_LIGHT,
		RELEASE_LIGHT,
		NUM_LIGHTS
	};

	bool decaying = false;
	float env = 0.0f;
	SchmittTrigger trigger;

	ADSR() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

	}
	void step() override;
};


void ADSR::step() {
	float attack = clamp(params[ATTACK_PARAM].value + inputs[ATTACK_INPUT].value / 10.0f, 0.0f, 1.0f);
	float decay = clamp(params[DECAY_PARAM].value + inputs[DECAY_INPUT].value / 10.0f, 0.0f, 1.0f);
	float sustain = clamp(params[SUSTAIN_PARAM].value + inputs[SUSTAIN_INPUT].value / 10.0f, 0.0f, 1.0f);
	float release = clamp(params[RELEASE_PARAM].value + inputs[RELEASE_INPUT].value / 10.0f, 0.0f, 1.0f);
	// Gate and trigger
	bool gated = inputs[GATE_INPUT].value >= 1.0f;
	if (trigger.process(inputs[TRIG_INPUT].value))
		decaying = false;

	const float base = 20000.0f;
	const float maxTime = 10.0f;
	if (gated) {
		if (decaying) {
			// Decay
			if (decay < 1e-4) {
				env = sustain;
			}
			else {
				env += powf(base, 1 - decay) / maxTime * (sustain - env) / engineGetSampleRate();
			}
		}
		else {
			// Attack
			// Skip ahead if attack is all the way down (infinitely fast)
			if (attack < 1e-4) {
				env = 1.0f;
			}
			else {
				env += powf(base, 1 - attack) / maxTime * (1.01f - env) / engineGetSampleRate();
			}
			if (env >= 1.0f) {
				env = 1.0f;
				decaying = true;
			}
		}
	}
	else {
		// Release
		if (release < 1e-4) {
			env = 0.0f;
		}
		else {
			env += powf(base, 1 - release) / maxTime * (0.0f - env) / engineGetSampleRate();
		}
		decaying = false;
	}

	bool sustaining = isNear(env, sustain, 1e-3);
	bool resting = isNear(env, 0.0, 1e-3);

	outputs[ENVELOPE_OUTPUT].value = 10.0f * env;

	// Lights
	lights[ATTACK_LIGHT].value = (gated && !decaying) ? 1.0f : 0.0f;
	lights[DECAY_LIGHT].value = (gated && decaying && !sustaining) ? 1.0f : 0.0f;
	lights[SUSTAIN_LIGHT].value = (gated && decaying && sustaining) ? 1.0f : 0.0f;
	lights[RELEASE_LIGHT].value = (!gated && !resting) ? 1.0f : 0.0f;
}



struct ADSRWidget : ModuleWidget 
{ 
    ADSRWidget(ADSR *module);
};


ADSRWidget::ADSRWidget(ADSR *module) : ModuleWidget(module) {
	box.size = Vec(RACK_GRID_WIDTH*8, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/ADSR.svg")));
		addChild(panel);
	}

	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	
	static const float posX[4] = {13.0f,39.0f,65.0f,91.0f};
	addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(posX[0]+6, 74), module, ADSR::ATTACK_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(posX[1]+6, 74), module, ADSR::DECAY_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(posX[2]+6, 74), module, ADSR::SUSTAIN_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(posX[3]+6, 74), module, ADSR::RELEASE_LIGHT));

	addParam(ParamWidget::create<as_SlidePot>(Vec(posX[0]-3, 90), module, ADSR::ATTACK_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<as_SlidePot>(Vec(posX[1]-3, 90), module, ADSR::DECAY_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<as_SlidePot>(Vec(posX[2]-3, 90), module, ADSR::SUSTAIN_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<as_SlidePot>(Vec(posX[3]-3, 90), module, ADSR::RELEASE_PARAM, 0.0f, 1.0f, 0.5f));

	addInput(Port::create<as_PJ301MPort>(Vec(posX[0]-4, 217), Port::INPUT, module, ADSR::ATTACK_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(posX[1]-4, 217), Port::INPUT, module, ADSR::DECAY_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(posX[2]-4, 217), Port::INPUT, module, ADSR::SUSTAIN_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(posX[3]-4, 217), Port::INPUT, module, ADSR::RELEASE_INPUT));

	addInput(Port::create<as_PJ301MPort>(Vec(posX[0]-4, 310), Port::INPUT, module, ADSR::GATE_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(48, 310), Port::INPUT, module, ADSR::TRIG_INPUT));

	addOutput(Port::create<as_PJ301MPort>(Vec(posX[3]-4, 310), Port::OUTPUT, module, ADSR::ENVELOPE_OUTPUT));
}

} // namespace rack_plugin_AS

using namespace rack_plugin_AS;

RACK_PLUGIN_MODEL_INIT(AS, ADSR) {
   Model *modelADSR = Model::create<ADSR, ADSRWidget>("AS", "ADSR", "ADSR", ENVELOPE_GENERATOR_TAG);
   return modelADSR;
}
