#include "SubmarineFree.hpp"

namespace rack_plugin_SubmarineFree {

struct SS_112 : Module {
	static constexpr int deviceCount = 12;
	SS_112() : Module(0, deviceCount, 0, 0) {}
};

struct SS112 : ModuleWidget {
	SS112(SS_112 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/SS-112.svg")));
		for (int i = 0; i < SS_112::deviceCount; i++) {
			addInput(Port::create<SilverPort>(Vec(2.5,19 + i * 29), Port::INPUT, module, i));
		}
	}
};

struct SS_208 : Module {
	static constexpr int deviceCount = 8;
	SS_208() : Module(0, 0, deviceCount, 0) {
		outputs[0].value = M_PI;
		outputs[1].value = 2 * M_PI;
		outputs[2].value = M_E;
		outputs[3].value = M_SQRT1_2;
		outputs[4].value = M_SQRT2;
		outputs[5].value = powf(3.0f, 0.5f);
		outputs[6].value = powf(5.0f, 0.5f);
		outputs[7].value = powf(7.0f, 0.5f);
	}
};

struct SS208 : ModuleWidget {
	SS208(SS_208 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/SS-208.svg")));
		for (int i = 0; i < SS_208::deviceCount; i++) {
			addOutput(Port::create<SilverPort>(Vec(2.5,19 + 43 * i), Port::OUTPUT, module, i));
		}
	}
};

struct SS_212 : Module {
	static constexpr int deviceCount = 12;
	int v = 0;
	void setValues() {
		for (int i = 0; i < deviceCount; i++) {
			outputs[i].value = v + 1.0f * i / 12.0f;
		}
	}

	SS_212() : Module(0, 0, deviceCount, 0) {
		setValues();
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "octave", json_integer(v));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *intJ = json_object_get(rootJ, "octave");
		if (intJ)
			v = json_integer_value(intJ);
		setValues();
	}
};

struct SS212 : ModuleWidget {
	SS212(SS_212 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/SS-212.svg")));
		for (int i = 0; i < SS_212::deviceCount; i++) {
			addOutput(Port::create<SilverPort>(Vec(2.5,19 + i * 29), Port::OUTPUT, module, i));
		}
	}

	void appendContextMenu(Menu *menu) override;
};

struct SSMenuItem : MenuItem {
	SS_212 *ss_212;
	int v;
	void onAction(EventAction &e) override {
		ss_212->v = v;
		ss_212->setValues();
	}
	void step() override {
		rightText = CHECKMARK(ss_212->v == v);
	}
};

void SS212::appendContextMenu(Menu *menu) {
	char label[20];
	menu->addChild(MenuEntry::create());
	SS_212 *ss_212 = dynamic_cast<SS_212*>(this->module);
        assert(ss_212);
	for (int i = -5; i < 5; i++) {
		sprintf(label, "Octave %d", i);
		SSMenuItem *menuItem = MenuItem::create<SSMenuItem>(label);
		menuItem->ss_212 = ss_212;
		menuItem->v = i;
		menu->addChild(menuItem);
	}
}

struct SS_221 : Module {
	static constexpr int deviceCount = 21;
	SS_221() : Module(0, 0, deviceCount, 0) {
		for (int i = 0; i < deviceCount; i++) {
			outputs[i].value = 10.0f - i;
		}
	}
};

struct SS221 : ModuleWidget {
	SS221(SS_221 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/SS-221.svg")));
		for (int i = 0; i < SS_221::deviceCount; i++) {
			addOutput(Port::create<SilverPort>(Vec(2.5 + 45 * (i % 2),19 + i * 16), Port::OUTPUT, module, i));
		}
	}
};

struct SS_220 : Module {
	static constexpr int deviceCount = 12;
	static constexpr int deviceSetCount = 10;
	SS_220() : Module(0, 0, deviceCount * deviceSetCount, 0) {
		for (int j = 0; j < deviceSetCount; j++) {
			for (int i = 0; i < deviceCount; i++) {
				outputs[j * deviceCount + i].value = (j - 5.0f) + 1.0f * i / 12.0f;
			}
		}
	}
};

struct SS220 : ModuleWidget {
	SS220(SS_220 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/SS-220.svg")));
		for (int j = 0; j < SS_220::deviceSetCount; j++) {
			for (int i = 0; i < SS_220::deviceCount; i++) {
				addOutput(Port::create<SilverPort>(Vec(2.5 + 30 * j, 19 + i * 29), Port::OUTPUT, module, j * SS_220::deviceCount + i));
			}
		}
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, SS112) {
   Model *modelSS112 = Model::create<SS_112, SS112>("Submarine (Free)", "SS-112", "SS-112 12 Input Sinks", UTILITY_TAG);
   return modelSS112;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, SS208) {
   Model *modelSS208 = Model::create<SS_208, SS208>("Submarine (Free)", "SS-208", "SS-208 8 Irrational Output Voltage Sources", UTILITY_TAG);
   return modelSS208;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, SS212) {
   Model *modelSS212 = Model::create<SS_212, SS212>("Submarine (Free)", "SS-212", "SS-212 12 Chromatic Output Voltage Sources", UTILITY_TAG);
   return modelSS212;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, SS220) {
   Model *modelSS220 = Model::create<SS_220, SS220>("Submarine (Free)", "SS-220", "SS-220 120 Chromatic Output Voltage Sources", UTILITY_TAG);
   return modelSS220;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, SS221) {
   Model *modelSS221 = Model::create<SS_221, SS221>("Submarine (Free)", "SS-221", "SS-221 21 Output Voltage Sources", UTILITY_TAG);
   return modelSS221;
}
