#include "SubmarineFree.hpp"

namespace rack_plugin_SubmarineFree {

template <int x>
struct BP1 : ModuleWidget {
	BP1(Module *module) : ModuleWidget(module) {
		setPanel(SubHelper::LoadPanel(plugin, "BP-1", x));
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP101) {
   Model *modelBP101 = Model::create<Module, BP1<1>>("Submarine (Free)", "BP-101", "BP-101 Blanking Plate", BLANK_TAG);
   return modelBP101;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP102) {
   Model *modelBP102 = Model::create<Module, BP1<2>>("Submarine (Free)", "BP-102", "BP-102 Blanking Plate", BLANK_TAG);
   return modelBP102;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP104) {
   Model *modelBP104 = Model::create<Module, BP1<4>>("Submarine (Free)", "BP-104", "BP-104 Blanking Plate", BLANK_TAG);
   return modelBP104;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP108) {
   Model *modelBP108 = Model::create<Module, BP1<8>>("Submarine (Free)", "BP-108", "BP-108 Blanking Plate", BLANK_TAG);
   return modelBP108;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP110) {
   Model *modelBP110 = Model::create<Module, BP1<10>>("Submarine (Free)", "BP-110", "BP-110 Blanking Plate", BLANK_TAG);
   return modelBP110;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP112) {
   Model *modelBP112 = Model::create<Module, BP1<12>>("Submarine (Free)", "BP-112", "BP-112 Blanking Plate", BLANK_TAG);
   return modelBP112;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP116) {
   Model *modelBP116 = Model::create<Module, BP1<16>>("Submarine (Free)", "BP-116", "BP-116 Blanking Plate", BLANK_TAG);
   return modelBP116;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP120) {
   Model *modelBP120 = Model::create<Module, BP1<20>>("Submarine (Free)", "BP-120", "BP-120 Blanking Plate", BLANK_TAG);
   return modelBP120;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP124) {
   Model *modelBP124 = Model::create<Module, BP1<24>>("Submarine (Free)", "BP-124", "BP-124 Blanking Plate", BLANK_TAG);
   return modelBP124;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP132) {
   Model *modelBP132 = Model::create<Module, BP1<32>>("Submarine (Free)", "BP-132", "BP-132 Blanking Plate", BLANK_TAG);
   return modelBP132;
}
