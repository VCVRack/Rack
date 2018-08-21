#include "AH.hpp"
#include "Core.hpp"
#include "UI.hpp"
#include "dsp/digital.hpp"

#include <iostream>

namespace rack_plugin_AmalgamatedHarmonics {

struct Circle : AHModule {

	const static int NUM_PITCHES = 6;
	
	enum ParamIds {
		KEY_PARAM,
		MODE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ROTL_INPUT,
		ROTR_INPUT,
		KEY_INPUT,
		MODE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		KEY_OUTPUT,
		MODE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(MODE_LIGHT,7),
		ENUMS(BKEY_LIGHT,12),
		ENUMS(CKEY_LIGHT,12),
		NUM_LIGHTS
	};
	enum Scaling {
		CHROMATIC,
		FIFTHS
	};
	Scaling voltScale = FIFTHS;
	
	Circle() : AHModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
	
	json_t *toJson() override {
		json_t *rootJ = json_object();

		// gateMode
		json_t *scaleModeJ = json_integer((int) voltScale);
		json_object_set_new(rootJ, "scale", scaleModeJ);

		return rootJ;
	}
	
	void fromJson(json_t *rootJ) override {
		// gateMode
		json_t *scaleModeJ = json_object_get(rootJ, "scale");
		
		if (scaleModeJ) {
			voltScale = (Scaling)json_integer_value(scaleModeJ);
		}
	}
	
	SchmittTrigger rotLTrigger;
	SchmittTrigger rotRTrigger;
	
	PulseGenerator stepPulse;
	
	float outVolts[NUM_PITCHES];
	
	int baseKeyIndex = 0;
	int curKeyIndex = 0;
	
	int curMode = 0;
		
	int poll = 50000;
		
};

void Circle::step() {
	
	AHModule::step();

	// Get inputs from Rack
	float rotLInput		= inputs[ROTL_INPUT].value;
	float rotRInput		= inputs[ROTR_INPUT].value;
	
	int newKeyIndex = 0;
	int deg;
	if (inputs[KEY_INPUT].active) {
		float fRoot = inputs[KEY_INPUT].value;
		if (voltScale == FIFTHS) {
			newKeyIndex = CoreUtil().getKeyFromVolts(fRoot);
		} else {
			CoreUtil().getPitchFromVolts(fRoot, Core::NOTE_C, Core::SCALE_CHROMATIC, &newKeyIndex, &deg);
		}
	} else {
		newKeyIndex = params[KEY_PARAM].value;
	}

	int newMode = 0;
	if (inputs[MODE_INPUT].active) {
		float fMode = inputs[MODE_INPUT].value;
		newMode = round(rescale(fabs(fMode), 0.0f, 10.0f, 0.0f, 6.0f)); 
	} else {
		newMode = params[MODE_PARAM].value;
	}

	curMode = newMode;
		
	// Process inputs
	bool rotLStatus		= rotLTrigger.process(rotLInput);
	bool rotRStatus		= rotRTrigger.process(rotRInput);
		
	if (rotLStatus) {
		if (debugEnabled()) { std::cout << stepX << " Rotate left: " << curKeyIndex; }
		if (voltScale == FIFTHS) {
			if (curKeyIndex == 0) {
				curKeyIndex = 11;
			} else {
				curKeyIndex--;
			}
		} else {
			curKeyIndex = curKeyIndex + 5;
			if (curKeyIndex > 11) {
				curKeyIndex = curKeyIndex - 12;
			}
		}
		
		if (debugEnabled()) { std::cout << " -> " << curKeyIndex << std::endl;	}
	} 
	
	if (rotRStatus) {
		if (debugEnabled()) { std::cout << stepX << " Rotate right: " << curKeyIndex; }
		if (voltScale == FIFTHS) {
			if (curKeyIndex == 11) {
				curKeyIndex = 0;
			} else {
				curKeyIndex++;
			}
		} else {
			curKeyIndex = curKeyIndex - 5;
			if (curKeyIndex < 0) {
				curKeyIndex = curKeyIndex + 12;
			}
		}
		if (debugEnabled()) { std::cout << " -> " << curKeyIndex << std::endl;	}
	} 
	
	if (rotLStatus && rotRStatus) {
		if (debugEnabled()) { std::cout << stepX << " Reset " << curKeyIndex << std::endl;	}
		curKeyIndex = baseKeyIndex;
	}
	
	
	if (newKeyIndex != baseKeyIndex) {
		if (debugEnabled()) { std::cout << stepX << " New base: " << newKeyIndex << std::endl;}
		baseKeyIndex = newKeyIndex;
		curKeyIndex = newKeyIndex;
	}
	
	int curKey;
	int baseKey;

	if (voltScale == FIFTHS) {
		curKey = CoreUtil().CIRCLE_FIFTHS[curKeyIndex];
		baseKey = CoreUtil().CIRCLE_FIFTHS[baseKeyIndex];
	} else {
		curKey = curKeyIndex;
		baseKey = baseKeyIndex;		
	}


	float keyVolts = CoreUtil().getVoltsFromKey(curKey);
	float modeVolts = CoreUtil().getVoltsFromMode(curMode);
	
	for (int i = 0; i < Core::NUM_NOTES; i++) {
		lights[CKEY_LIGHT + i].value = 0.0;
		lights[BKEY_LIGHT + i].value = 0.0;
	}

	lights[CKEY_LIGHT + curKey].value = 1.0;
	lights[BKEY_LIGHT + baseKey].value = 1.0;
	
	for (int i = 0; i < Core::NUM_MODES; i++) {
		lights[MODE_LIGHT + i].value = 0.0;
	}
	lights[MODE_LIGHT + curMode].value = 1.0;
		
	outputs[KEY_OUTPUT].value = keyVolts;
	outputs[MODE_OUTPUT].value = modeVolts;
	
}

struct CircleWidget : ModuleWidget {
	CircleWidget(Circle *module);
	Menu *createContextMenu() override;
};

CircleWidget::CircleWidget(Circle *module) : ModuleWidget(module) {
	
	UI ui;
	
	box.size = Vec(240, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Circle.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

	addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 0, 0, true, false), Port::INPUT, module, Circle::ROTL_INPUT));
	addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 5, 0, true, false), Port::INPUT, module, Circle::ROTR_INPUT));
	addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 0, 5, true, false), Port::INPUT, module, Circle::KEY_INPUT));
	addParam(ParamWidget::create<AHKnobSnap>(ui.getPosition(UI::KNOB, 1, 5, true, false), module, Circle::KEY_PARAM, 0.0, 11.0, 0.0)); 
	addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 2, 5, true, false), Port::INPUT, module, Circle::MODE_INPUT));
	addParam(ParamWidget::create<AHKnobSnap>(ui.getPosition(UI::KNOB, 3, 5, true, false), module, Circle::MODE_PARAM, 0.0, 6.0, 0.0)); 

	float div = (M_PI * 2) / 12.0;

	for (int i = 0; i < 12; i++) {

		float cosDiv = cos(div * i);
		float sinDiv = sin(div * i);
	
		float xPos  = sinDiv * 52.5;
		float yPos  = cosDiv * 52.5;
		float xxPos = sinDiv * 60.0;
		float yyPos = cosDiv * 60.0;

//		ui.calculateKeyboard(i, xSpace, xOffset, 230.0, &xPos, &yPos, &scale);
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(xxPos + 116.5, 149.5 - yyPos), module, Circle::CKEY_LIGHT + CoreUtil().CIRCLE_FIFTHS[i]));

//		ui.calculateKeyboard(i, xSpace, xOffset + 72.0, 165.0, &xPos, &yPos, &scale);
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(xPos + 116.5, 149.5 - yPos), module, Circle::BKEY_LIGHT + CoreUtil().CIRCLE_FIFTHS[i]));
	}
	
	float xOffset = 18.0;
	
	for (int i = 0; i < 7; i++) {
		float xPos = 2 * xOffset + i * 18.2;
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(xPos, 280.0), module, Circle::MODE_LIGHT + i));
	}

	addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 4, 5, true, false), Port::OUTPUT, module, Circle::KEY_OUTPUT));
	addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 5, 5, true, false), Port::OUTPUT, module, Circle::MODE_OUTPUT));

}


struct CircleScalingItem : MenuItem {
	Circle *circle;
	Circle::Scaling scalingMode;
	void onAction(EventAction &e) override {
		circle->voltScale = scalingMode;
	}
	void step() override {
		rightText = (circle->voltScale == scalingMode) ? "✔" : "";
	}
};

Menu *CircleWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	Circle *circle = dynamic_cast<Circle*>(module);
	assert(circle);

	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Root Volt Scaling";
	menu->addChild(modeLabel);

	CircleScalingItem *fifthsItem = new CircleScalingItem();
	fifthsItem->text = "Fifths";
	fifthsItem->circle = circle;
	fifthsItem->scalingMode = Circle::FIFTHS;
	menu->addChild(fifthsItem);

	CircleScalingItem *chromaticItem = new CircleScalingItem();
	chromaticItem->text = "Chromatic (V/OCT)";
	chromaticItem->circle = circle;
	chromaticItem->scalingMode = Circle::CHROMATIC;
	menu->addChild(chromaticItem);

	return menu;
}

} // namespace rack_plugin_AmalgamatedHarmonics

using namespace rack_plugin_AmalgamatedHarmonics;

RACK_PLUGIN_MODEL_INIT(AmalgamatedHarmonics, Circle) {
   Model *modelCircle = Model::create<Circle, CircleWidget>( "Amalgamated Harmonics", "Circle", "Fifths and Fourths", SEQUENCER_TAG);
   return modelCircle;
}

// ♯♭
