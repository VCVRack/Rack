#include "QuantalAudio.hpp"

namespace rack_plugin_QuantalAudio {

struct Blank5Widget : ModuleWidget {
	Blank5Widget(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/blank-5.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	}
};

} // namespace rack_plugin_QuantalAudio

using namespace rack_plugin_QuantalAudio;

RACK_PLUGIN_MODEL_INIT(QuantalAudio, Blank5) {
   Model *modelBlank5 = Model::create<Module, Blank5Widget>("QuantalAudio", "Blank5", "Blank Panel | 5HP", BLANK_TAG);
   return modelBlank5;
}
