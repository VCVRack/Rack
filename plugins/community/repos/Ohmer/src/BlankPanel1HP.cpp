////////////////////////////////////////////////////////////////////////////////////////////////////
////// Blank Panel 1 HP module /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

namespace rack_plugin_Ohmer {

struct OhmerBlank1 : Module {
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
	OhmerBlank1() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void OhmerBlank1::step() {
}

struct OhmerBlank1Widget : ModuleWidget {
	OhmerBlank1Widget(OhmerBlank1 *module);
};

OhmerBlank1Widget::OhmerBlank1Widget(OhmerBlank1 *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/Blank_1HP.svg")));
	// Two screws for 1 HP module.
	addChild(Widget::create<Torx_Silver>(Vec(0, 0)));
	addChild(Widget::create<Torx_Silver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

} // namespace rack_plugin_Ohmer

using namespace rack_plugin_Ohmer;

RACK_PLUGIN_MODEL_INIT(Ohmer, BlankPanel1) {
   Model *modelBlankPanel1 = Model::create<OhmerBlank1, OhmerBlank1Widget>("Ohmer Modules", "OhmerBlank1", "1 HP Blank Panel", BLANK_TAG);
   return modelBlankPanel1;
}
