#include "CatroModulo.hpp"


//Catro-Module CM-6: 1hp blank

struct CM6Module : Module {

	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	CM6Module() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}
	void step() override;
};

void CM6Module::step() {
}

struct CM6ModuleWidget : ModuleWidget {

	CM6ModuleWidget(CM6Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/CM-6.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(1, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(1, 365)));

	}
};

// Model *modelCM6Module = Model::create<CM6Module, CM6ModuleWidget>("CatroModulo", "CatroModulo_CM-6", "C/M6 : 1hp blank", BLANK_TAG);

RACK_PLUGIN_MODEL_INIT(CatroModulo, CM6Module) {
   Model *model = Model::create<CM6Module, CM6ModuleWidget>("CatroModulo", "CatroModulo_CM6", "C/M6 : 1hp blank", BLANK_TAG);
   return model;
}