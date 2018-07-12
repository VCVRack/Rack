////////////////////////////////////////////////////////////////////////////////////////////////////
////// Blank Panel 4 HP module /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

namespace rack_plugin_Ohmer {

struct OhmerBlank4 : Module {
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
	OhmerBlank4() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void OhmerBlank4::step() {
}

struct OhmerBlank4Widget : ModuleWidget {
	OhmerBlank4Widget(OhmerBlank4 *module);
};

OhmerBlank4Widget::OhmerBlank4Widget(OhmerBlank4 *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/Blank_4HP.svg")));
	// Four screws for 4 HP module.
	addChild(Widget::create<Torx_Silver>(Vec(0, 0)));
	addChild(Widget::create<Torx_Silver>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<Torx_Silver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<Torx_Silver>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

} // namespace rack_plugin_Ohmer

using namespace rack_plugin_Ohmer;

RACK_PLUGIN_MODEL_INIT(Ohmer, BlankPanel4) {
   Model *modelBlankPanel4 = Model::create<OhmerBlank4, OhmerBlank4Widget>("Ohmer Modules", "OhmerBlank4", "4 HP Blank Panel", BLANK_TAG);
   return modelBlankPanel4;
}
