#include "AH.hpp"
#include "Core.hpp"
#include "UI.hpp"

#include <iostream>

namespace rack_plugin_AmalgamatedHarmonics {

struct ScaleQuantizer : AHModule {

	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		IN_INPUT,
		KEY_INPUT,
		SCALE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		TRIG_OUTPUT,
		GATE_OUTPUT,
		NUM_OUTPUTS = GATE_OUTPUT + 12
	};
	enum LightIds {
		NOTE_LIGHT,
		KEY_LIGHT = NOTE_LIGHT + 12,
		SCALE_LIGHT = KEY_LIGHT + 12,
		DEGREE_LIGHT = SCALE_LIGHT + 12,
		NUM_LIGHTS = DEGREE_LIGHT + 12
	};

	ScaleQuantizer() : AHModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	bool firstStep = true;
	int lastScale = 0;
	int lastRoot = 0;
	float lastPitch = 0.0;
	
	int currScale = 0;
	int currRoot = 0;
	int currNote = 0;
	int currDegree = 0;
	float currPitch = 0.0;

};

void ScaleQuantizer::step() {
	
	stepX++;
	
	lastScale = currScale;
	lastRoot = currRoot;
	lastPitch = currPitch;

	// Get the input pitch
	float volts = inputs[IN_INPUT].value;
	float root =  inputs[KEY_INPUT].value;
	float scale = inputs[SCALE_INPUT].value;

	// Calculate output pitch from raw voltage
	currPitch = CoreUtil().getPitchFromVolts(volts, root, scale, &currRoot, &currScale, &currNote, &currDegree);

	// Set the value
	outputs[OUT_OUTPUT].value = currPitch;

	// update tone lights
	for (int i = 0; i < Core::NUM_NOTES; i++) {
		lights[NOTE_LIGHT + i].value = 0.0;
	}
	lights[NOTE_LIGHT + currNote].value = 1.0;

	// update degree lights
	for (int i = 0; i < Core::NUM_NOTES; i++) {
		lights[DEGREE_LIGHT + i].value = 0.0;
		outputs[GATE_OUTPUT + i].value = 0.0;
	}
	lights[DEGREE_LIGHT + currDegree].value = 1.0;
	outputs[GATE_OUTPUT + currDegree].value = 10.0;

	if (lastScale != currScale || firstStep) {
		for (int i = 0; i < Core::NUM_NOTES; i++) {
			lights[SCALE_LIGHT + i].value = 0.0;
		}
		lights[SCALE_LIGHT + currScale].value = 1.0;
	} 

	if (lastRoot != currRoot || firstStep) {
		for (int i = 0; i < Core::NUM_NOTES; i++) {
			lights[KEY_LIGHT + i].value = 0.0;
		}
		lights[KEY_LIGHT + currRoot].value = 1.0;
	} 

	if (lastPitch != currPitch || firstStep) {
		outputs[TRIG_OUTPUT].value = 10.0;
	} else {
		outputs[TRIG_OUTPUT].value = 0.0;		
	}

	firstStep = false;

}
struct ScaleQuantizerWidget : ModuleWidget {
	ScaleQuantizerWidget(ScaleQuantizer *module);
};

ScaleQuantizerWidget::ScaleQuantizerWidget(ScaleQuantizer *module) : ModuleWidget(module) {
	
	UI ui;
	
	box.size = Vec(240, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/ScaleQuantizer.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

	addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 0, 5, false, false), Port::INPUT, module, ScaleQuantizer::IN_INPUT));
	addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 1, 5, false, false), Port::INPUT, module, ScaleQuantizer::KEY_INPUT));
	addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 2, 5, false, false), Port::INPUT, module, ScaleQuantizer::SCALE_INPUT));
	addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 3, 5, false, false), Port::OUTPUT, module, ScaleQuantizer::TRIG_OUTPUT));
	addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 4, 5, false, false), Port::OUTPUT, module, ScaleQuantizer::OUT_OUTPUT));

	float xOffset = 18.0;
	float xSpace = 21.0;
	float xPos = 0.0;
	float yPos = 0.0;
	int scale = 0;

	for (int i = 0; i < 12; i++) {
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(xOffset + i * 18.0, 280.0), module, ScaleQuantizer::SCALE_LIGHT + i));

		ui.calculateKeyboard(i, xSpace, xOffset, 230.0, &xPos, &yPos, &scale);
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(xPos, yPos), module, ScaleQuantizer::KEY_LIGHT + scale));

		ui.calculateKeyboard(i, xSpace, xOffset + 72.0, 165.0, &xPos, &yPos, &scale);
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(xPos, yPos), module, ScaleQuantizer::NOTE_LIGHT + scale));

		ui.calculateKeyboard(i, 30.0, xOffset + 9.5, 110.0, &xPos, &yPos, &scale);
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(xPos, yPos), module, ScaleQuantizer::DEGREE_LIGHT + scale));

		ui.calculateKeyboard(i, 30.0, xOffset, 85.0, &xPos, &yPos, &scale);

		addOutput(Port::create<PJ301MPort>(Vec(xPos, yPos), Port::OUTPUT, module, ScaleQuantizer::GATE_OUTPUT + scale));
	}

}

} // namespace rack_plugin_AmalgamatedHarmonics

using namespace rack_plugin_AmalgamatedHarmonics;

RACK_PLUGIN_MODEL_INIT(AmalgamatedHarmonics, ScaleQuantizer) {
   Model *modelScaleQuantizer = Model::create<ScaleQuantizer, ScaleQuantizerWidget>( "Amalgamated Harmonics", "ScaleQuantizer", "Scale Quantizer (deprecated)", QUANTIZER_TAG);
   return modelScaleQuantizer;
}

