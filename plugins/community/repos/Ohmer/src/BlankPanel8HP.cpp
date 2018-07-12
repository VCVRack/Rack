////////////////////////////////////////////////////////////////////////////////////////////////////
////// Blank Panel 8 HP module /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

namespace rack_plugin_Ohmer {

struct OhmerBlank8 : Module {
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
	OhmerBlank8() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void OhmerBlank8::step() {
}

struct OhmerBlank8Widget : ModuleWidget {
	OhmerBlank8Widget(OhmerBlank8 *module);
};

OhmerBlank8Widget::OhmerBlank8Widget(OhmerBlank8 *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/Blank_8HP.svg")));
	// Four screws for 8 HP module.
	addChild(Widget::create<Torx_Silver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<Torx_Silver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<Torx_Silver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<Torx_Silver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

} // namespace rack_plugin_Ohmer

using namespace rack_plugin_Ohmer;

RACK_PLUGIN_MODEL_INIT(Ohmer, BlankPanel8) {
   Model *modelBlankPanel8 = Model::create<OhmerBlank8, OhmerBlank8Widget>("Ohmer Modules", "OhmerBlank8", "8 HP Blank Panel", BLANK_TAG);
   return modelBlankPanel8;
}
