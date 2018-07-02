#include "SubmarineFree.hpp"

namespace rack_plugin_SubmarineFree {

struct BP : Module {
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

	BP() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void BP::step() {
}

struct BP101 : ModuleWidget {
	BP101(BP *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/BP-101.svg")));
	}
};

struct BP102 : ModuleWidget {
	BP102(BP *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/BP-102.svg")));
	}
};

struct BP104 : ModuleWidget {
	BP104(BP *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/BP-104.svg")));
	}
};

struct BP108 : ModuleWidget {
	BP108(BP *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/BP-108.svg")));
	}
};

struct BP110 : ModuleWidget {
	BP110(BP *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/BP-110.svg")));
	}
};

struct BP112 : ModuleWidget {
	BP112(BP *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/BP-112.svg")));
	}
};

struct BP116 : ModuleWidget {
	BP116(BP *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/BP-116.svg")));
	}
};

struct BP120 : ModuleWidget {
	BP120(BP *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/BP-120.svg")));
	}
};

struct BP124 : ModuleWidget {
	BP124(BP *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/BP-124.svg")));
	}
};

struct BP132 : ModuleWidget {
	BP132(BP *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/BP-132.svg")));
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP101) {
   Model *modelBP101 = Model::create<BP, BP101>("SubmarineFree", "BP-101", "BP-101 Blanking Plate", BLANK_TAG);
   return modelBP101;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP102) {
   Model *modelBP102 = Model::create<BP, BP102>("SubmarineFree", "BP-102", "BP-102 Blanking Plate", BLANK_TAG);
   return modelBP102;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP104) {
   Model *modelBP104 = Model::create<BP, BP104>("SubmarineFree", "BP-104", "BP-104 Blanking Plate", BLANK_TAG);
   return modelBP104;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP108) {
   Model *modelBP108 = Model::create<BP, BP108>("SubmarineFree", "BP-108", "BP-108 Blanking Plate", BLANK_TAG);
   return modelBP108;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP110) {
   Model *modelBP110 = Model::create<BP, BP110>("SubmarineFree", "BP-110", "BP-110 Blanking Plate", BLANK_TAG);
   return modelBP110;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP112) {
   Model *modelBP112 = Model::create<BP, BP112>("SubmarineFree", "BP-112", "BP-112 Blanking Plate", BLANK_TAG);
   return modelBP112;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP116) {
   Model *modelBP116 = Model::create<BP, BP116>("SubmarineFree", "BP-116", "BP-116 Blanking Plate", BLANK_TAG);
   return modelBP116;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP120) {
   Model *modelBP120 = Model::create<BP, BP120>("SubmarineFree", "BP-120", "BP-120 Blanking Plate", BLANK_TAG);
   return modelBP120;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP124) {
   Model *modelBP124 = Model::create<BP, BP124>("SubmarineFree", "BP-124", "BP-124 Blanking Plate", BLANK_TAG);
   return modelBP124;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BP132) {
   Model *modelBP132 = Model::create<BP, BP132>("SubmarineFree", "BP-132", "BP-132 Blanking Plate", BLANK_TAG);
   return modelBP132;
}
