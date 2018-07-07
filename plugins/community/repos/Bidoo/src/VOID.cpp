#include "Bidoo.hpp"
#include "BidooComponents.hpp"

using namespace std;

namespace rack_plugin_Bidoo {

struct VOID : Module {
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

	VOID() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {	}

	void step() override;
};


void VOID::step() {

}

struct VOIDWidget : ModuleWidget {
	VOIDWidget(VOID *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/VOID.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	}
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, VOID) {
   Model *modelVOID = Model::create<VOID, VOIDWidget>("Bidoo", "vOId", "vOId machine", BLANK_TAG);
   return modelVOID;
}
