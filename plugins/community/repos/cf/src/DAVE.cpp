


#include "cf.hpp"

namespace rack_plugin_cf {

struct DAVE : Module {
	enum ParamIds {
		
		NUM_PARAMS
	};
	enum InputIds {

		NUM_INPUTS
	};
	enum OutputIds {

		NUM_OUTPUTS
	};


	DAVE() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;

};


void DAVE::step() {

}
struct DAVEWidget : ModuleWidget {
	DAVEWidget(DAVE *module);
};

DAVEWidget::DAVEWidget(DAVE *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/DAVE.svg")));
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, DAVE) {
   Model *modelDAVE = Model::create<DAVE, DAVEWidget>("cf", "DAVE", "Dave", BLANK_TAG);
   return modelDAVE;
}
