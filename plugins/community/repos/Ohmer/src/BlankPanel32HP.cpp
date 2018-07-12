////////////////////////////////////////////////////////////////////////////////////////////////////
////// Blank Panel 32 HP module ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

namespace rack_plugin_Ohmer {

struct OhmerBlank32 : Module {
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
	OhmerBlank32() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void OhmerBlank32::step() {
}

struct OhmerBlank32Widget : ModuleWidget {
	OhmerBlank32Widget(OhmerBlank32 *module);
};

OhmerBlank32Widget::OhmerBlank32Widget(OhmerBlank32 *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/Blank_32HP.svg")));
	// Four screws for 32 HP module.
	addChild(Widget::create<Torx_Silver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<Torx_Silver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<Torx_Silver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<Torx_Silver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

} // namespace rack_plugin_Ohmer

using namespace rack_plugin_Ohmer;

RACK_PLUGIN_MODEL_INIT(Ohmer, BlankPanel32) {
   Model *modelBlankPanel32 = Model::create<OhmerBlank32, OhmerBlank32Widget>("Ohmer Modules", "OhmerBlank32", "32 HP Blank Panel", BLANK_TAG);
   return modelBlankPanel32;
}
