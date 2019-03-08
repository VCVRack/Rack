#include "QuantalAudio.hpp"

namespace rack_plugin_QuantalAudio {

struct Blank1Widget : ModuleWidget {
	Blank1Widget(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/blank-1.svg")));
	}
};

} // namespace rack_plugin_QuantalAudio

using namespace rack_plugin_QuantalAudio;

RACK_PLUGIN_MODEL_INIT(QuantalAudio, Blank1) {
   Model *modelBlank1 = Model::create<Module, Blank1Widget>("QuantalAudio", "Blank1", "Blank Panel | 1HP", BLANK_TAG);
   return modelBlank1;
}
