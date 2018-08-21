/*

PvCBlank

*/////////////////////////////////////////////////////////////////////////////


#include "pvc.hpp"

namespace rack_plugin_PvC {

struct PvCBlank : Module {
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
	PvCBlank() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	// void step() override;
};


struct PvCBlankWidget : ModuleWidget {
	PvCBlankWidget(PvCBlank *module);
};

PvCBlankWidget::PvCBlankWidget(PvCBlank *module) : ModuleWidget(module) {
setPanel(SVG::load(assetPlugin(plugin, "res/panels/PvC64.svg")));

}

} // namespace rack_plugin_PvC

using namespace rack_plugin_PvC;

RACK_PLUGIN_MODEL_INIT(PvC, PvCBlank) {
   Model *modelPvCBlank = Model::create<PvCBlank, PvCBlankWidget>(
      "PvC", "PvCBlank", "PvCBlank", BLANK_TAG);
   return modelPvCBlank;
}
