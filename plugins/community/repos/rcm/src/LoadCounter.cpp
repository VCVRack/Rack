#include "rcm.h"
#include "GVerbWidget.hpp"
#include <algorithm>

namespace rack_plugin_rcm {

struct LoadCounterModule : Module {
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

	LoadCounterModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}
	void step() override;

  int counter = 0;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void LoadCounterModule::step() {
  int n = 8;
  for (int i = 1; i <= n; ++i)
  {
    std::vector<int> x;
    x.resize(50);
    int m = 500;
    for (auto &v : x) 
    {
      v = 500 - m++; //rand();
    }

    std::sort(x.begin(), x.end());
  }
}

struct CKSSWhite : SVGSwitch, ToggleSwitch {
	CKSSWhite() {
		addFrame(SVG::load(assetPlugin(plugin, "res/CKSS_0_White.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CKSS_1_White.svg")));
	}
};

struct LoadCounterModuleWidget : ModuleWidget {
    TextField *textField;

	LoadCounterModuleWidget(LoadCounterModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/CVTgl.svg")));
	}
};

} // namespace rack_plugin_rcm

using namespace rack_plugin_rcm;

RACK_PLUGIN_MODEL_INIT(rcm, LoadCounterModule) {
   Model *modelLoadCounterModule = Model::create<LoadCounterModule, LoadCounterModuleWidget>("rcm", "rcm-counter", "Load Counter", ENVELOPE_FOLLOWER_TAG);
   return modelLoadCounterModule;
}
