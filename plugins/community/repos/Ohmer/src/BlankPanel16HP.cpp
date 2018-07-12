////////////////////////////////////////////////////////////////////////////////////////////////////
////// Blank Panel 16 HP module ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

namespace rack_plugin_Ohmer {

struct OhmerBlank16 : Module {
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
	OhmerBlank16() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void OhmerBlank16::step() {
}

struct OhmerBlank16Widget : ModuleWidget {
	OhmerBlank16Widget(OhmerBlank16 *module);
};

OhmerBlank16Widget::OhmerBlank16Widget(OhmerBlank16 *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/Blank_16HP.svg")));
	// Four screws for 16 HP module.
	addChild(Widget::create<Torx_Silver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<Torx_Silver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<Torx_Silver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<Torx_Silver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

} // namespace rack_plugin_Ohmer

using namespace rack_plugin_Ohmer;

RACK_PLUGIN_MODEL_INIT(Ohmer, BlankPanel16) {
   Model *modelBlankPanel16 = Model::create<OhmerBlank16, OhmerBlank16Widget>("Ohmer Modules", "OhmerBlank16", "16 HP Blank Panel", BLANK_TAG);
   return modelBlankPanel16;
}
