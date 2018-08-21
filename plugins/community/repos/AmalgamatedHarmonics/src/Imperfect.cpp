#include "dsp/digital.hpp"

#include "AH.hpp"
#include "Core.hpp"
#include "UI.hpp"

#include <iostream>

namespace rack_plugin_AmalgamatedHarmonics {

struct Imperfect : AHModule {

	enum ParamIds {
		DELAY_PARAM,
		DELAYSPREAD_PARAM = DELAY_PARAM + 8,
		LENGTH_PARAM = DELAYSPREAD_PARAM + 8,
		LENGTHSPREAD_PARAM = LENGTH_PARAM + 8,
		DIVISION_PARAM = LENGTHSPREAD_PARAM + 8,
		NUM_PARAMS = DIVISION_PARAM + 8
	};
	enum InputIds {
		TRIG_INPUT,
		NUM_INPUTS = TRIG_INPUT + 8
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS = OUT_OUTPUT + 8
	};
	enum LightIds {
		OUT_LIGHT,
		NUM_LIGHTS = OUT_LIGHT + 16
	};

	Imperfect() : AHModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		reset();
	}
	
	void step() override;
	
	void reset() override {
		for (int i = 0; i < 8; i++) {
			delayState[i] = false;
			gateState[i] = false;
			delayTime[i] = 0.0;
			gateTime[i] = 0.0;
		}
	}
	
	Core core;

	bool delayState[8];
	bool gateState[8];
	float delayTime[8];
	float gateTime[8];
	PulseGenerator delayPhase[8];
	PulseGenerator gatePhase[8];
	SchmittTrigger inTrigger[8];

	int counter[8];
	
};

void Imperfect::step() {
	
	stepX++;
	
	float dlyLen;
	float dlySpr;
	float gateLen;
	float gateSpr;
	
	int lastValidInput = -1;
	
	for (int i = 0; i < 8; i++) {
		
		bool generateSignal = false;
		
		bool inputActive = inputs[TRIG_INPUT + i].active;
		bool haveTrigger = inTrigger[i].process(inputs[TRIG_INPUT + i].value);
		bool outputActive = outputs[OUT_OUTPUT + i].active;
		
		// If we have an active input, we should forget about previous valid inputs
		if (inputActive) {
			
			lastValidInput = -1;
	
			if (haveTrigger) {
				if (debugEnabled()) { std::cout << stepX << " " << i << " has active input and has received trigger" << std::endl; }
				generateSignal = true;
				lastValidInput = i;
			}
			
		} else {
			// We have an output and previously seen a trigger
			if (outputActive && lastValidInput > -1) {
				if (debugEnabled()) { std::cout << stepX << " " << i << " has active out and has seen trigger on " << lastValidInput << std::endl; }
				generateSignal = true;
			}
		}
				
		if (generateSignal) {

			counter[i]++;
			int target = core.ipow(2,params[DIVISION_PARAM + i].value);
		
			if (debugEnabled()) { 
				std::cout << stepX << " Div: " << i << ": Target: " << target << " Cnt: " << counter[lastValidInput] << " Exp: " << counter[lastValidInput] % target << std::endl; 
			}

			if (counter[lastValidInput] % target == 0) { 

				dlyLen  = log2(params[DELAY_PARAM + i].value);
				dlySpr  = log2(params[DELAYSPREAD_PARAM + i].value);
				gateLen = log2(params[LENGTH_PARAM + i].value);
				gateSpr = log2(params[LENGTHSPREAD_PARAM + i].value);

				// Determine delay and gate times for all active outputs
				double rndD = clamp(core.gaussrand(), -2.0f, 2.0f);
				delayTime[i] = clamp(dlyLen + dlySpr * rndD, 0.0f, 100.0f);
			
				// The modified gate time cannot be earlier than the start of the delay
				double rndG = clamp(core.gaussrand(), -2.0f, 2.0f);
				gateTime[i] = clamp(gateLen + gateSpr * rndG, 0.02f, 100.0f);

				if (debugEnabled()) { 
					std::cout << stepX << " Delay: " << i << ": Len: " << dlyLen << " Spr: " << dlySpr << " r: " << rndD << " = " << delayTime[i] << std::endl; 
					std::cout << stepX << " Gate: " << i << ": Len: " << gateLen << ", Spr: " << gateSpr << " r: " << rndG << " = " << gateTime[i] << std::endl; 
				}

				// Trigger the respective delay pulse generators
				delayState[i] = true;
				delayPhase[i].trigger(delayTime[i]);
			
			}
		}
	}
	
	for (int i = 0; i < 8; i++) {
	
		if (delayState[i] && !delayPhase[i].process(delta)) {
			gatePhase[i].trigger(gateTime[i]);
			gateState[i] = true;
			delayState[i] = false;
		}

		lights[OUT_LIGHT + i * 2].value = 0.0;
		lights[OUT_LIGHT + i * 2 + 1].value = 0.0;
		
		if (gatePhase[i].process(delta)) {
			outputs[OUT_OUTPUT + i].value = 10.0;

			lights[OUT_LIGHT + i * 2].value = 1.0;
			lights[OUT_LIGHT + i * 2 + 1].value = 0.0;

		} else {
			outputs[OUT_OUTPUT + i].value = 0.0;
			gateState[i] = false;

			if (delayState[i]) {
				lights[OUT_LIGHT + i * 2].value = 0.0;
				lights[OUT_LIGHT + i * 2 + 1].value = 1.0;
			} 
			
		}
			
	}
	
}

struct ImperfectWidget : ModuleWidget {
	ImperfectWidget(Imperfect *module);
};

ImperfectWidget::ImperfectWidget(Imperfect *module) : ModuleWidget(module) {
	
	UI ui;
	
	box.size = Vec(315, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Imperfect.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

	for (int i = 0; i < 8; i++) {
		addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 0, i + 1, true, true), Port::INPUT, module, Imperfect::TRIG_INPUT + i));
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 1, i + 1, true, true), module, Imperfect::DELAY_PARAM + i, 1.0, 2.0, 1.0));
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 2, i + 1, true, true), module, Imperfect::DELAYSPREAD_PARAM + i, 1.0, 2.0, 1.0));
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 3, i + 1, true, true), module, Imperfect::LENGTH_PARAM + i, 1.001, 2.0, 1.001)); // Always produce gate
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 4, i + 1, true, true), module, Imperfect::LENGTHSPREAD_PARAM + i, 1.0, 2.0, 1.0)); 
		addParam(ParamWidget::create<AHKnobSnap>(ui.getPosition(UI::KNOB, 5, i + 1, true, true), module, Imperfect::DIVISION_PARAM + i, 0, 8, 0));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(ui.getPosition(UI::LIGHT, 6, i + 1, true, true), module, Imperfect::OUT_LIGHT + i * 2));
		addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 7, i + 1, true, true), Port::OUTPUT, module, Imperfect::OUT_OUTPUT + i));
	}
	
}

} // namespace rack_plugin_AmalgamatedHarmonics

using namespace rack_plugin_AmalgamatedHarmonics;

RACK_PLUGIN_MODEL_INIT(AmalgamatedHarmonics, Imperfect) {
   Model *modelImperfect = Model::create<Imperfect, ImperfectWidget>( "Amalgamated Harmonics", "Imperfect", "Imperfect (deprecated)", UTILITY_TAG);
   return modelImperfect;
}

