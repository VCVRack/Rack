#include "DS.hpp"

namespace rack_plugin_SubmarineFree {

struct LD_106 : DS_Module {
	static const int deviceCount = 6;
	enum ParamIds {
		PARAM_CUTOFF_1,
		PARAM_CUTOFF_2,
		PARAM_CUTOFF_3,
		PARAM_CUTOFF_4,
		PARAM_CUTOFF_5,
		PARAM_CUTOFF_6,
		PARAM_WIDTH_1,
		PARAM_WIDTH_2,
		PARAM_WIDTH_3,
		PARAM_WIDTH_4,
		PARAM_WIDTH_5,
		PARAM_WIDTH_6,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_1,
		INPUT_2,
		INPUT_3,
		INPUT_4,
		INPUT_5,
		INPUT_6,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		OUTPUT_4,
		OUTPUT_5,
		OUTPUT_6,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	DS_Schmitt schmittState[deviceCount];

	LD_106() : DS_Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void LD_106::step() {
	for (int i = 0; i < deviceCount; i++) {
		outputs[OUTPUT_1 + i].value = output(schmittState[i].state(params[PARAM_CUTOFF_1 + i].value - params[PARAM_WIDTH_1 + i].value, params[PARAM_CUTOFF_1 + i].value + params[PARAM_WIDTH_1 + i].value, inputs[INPUT_1 + i].value));
	}
}

struct LD106 : ModuleWidget {
	ParamWidget *cutoffWidgets[LD_106::deviceCount];
	ParamWidget *widthWidgets[LD_106::deviceCount];
	LD106(LD_106 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/LD-106.svg")));

		for (int i = 0; i < LD_106::deviceCount; i++) {
			int offset = 58 * i;
			addInput(Port::create<sub_port>(Vec(4,19 + offset), Port::INPUT, module, LD_106::INPUT_1 + i));

			addOutput(Port::create<sub_port_blue>(Vec(62,19 + offset), Port::OUTPUT, module, LD_106::OUTPUT_1 + i));

			cutoffWidgets[i] = ParamWidget::create<sub_knob_small>(Vec(4, 47 + offset), module, LD_106::PARAM_CUTOFF_1 + i, -10.0f, 10.0f, 5.0f);
			addParam(cutoffWidgets[i]);
			widthWidgets[i] = ParamWidget::create<sub_knob_small>(Vec(62, 47 + offset), module, LD_106::PARAM_WIDTH_1 + i, 0.0f, 5.0f, 1.0f);
			addParam(widthWidgets[i]);
		}
	}
	void appendContextMenu(Menu *menu) override;
};

struct LDMenuItem: MenuItem {
	LD106 *ld106;
	float cutoff;
	float width;
	void onAction(EventAction &e) override {
		for (int i = 0; i < LD_106::deviceCount; i++) {
			ld106->cutoffWidgets[i]->setValue(cutoff);
			ld106->widthWidgets[i]->setValue(width);
		}
	}
};

void LD106::appendContextMenu(Menu *menu) {
	menu->addChild(MenuEntry::create());
	LD106 *ld106 = dynamic_cast<LD106*>(this);
	assert(ld106);
	LDMenuItem *menuItem = MenuItem::create<LDMenuItem>("Cutoff 5V");
	menuItem->ld106 = ld106;
	menuItem->cutoff = 5.0f;
	menuItem->width = 1.0f;
	menu->addChild(menuItem);
	menuItem = MenuItem::create<LDMenuItem>("Cutoff 0V");
	menuItem->ld106 = ld106;
	menuItem->cutoff = 0.0f;
	menuItem->width = 0.0f;
	menu->addChild(menuItem);
	menuItem = MenuItem::create<LDMenuItem>("Cutoff 2.5V");
	menuItem->ld106 = ld106;
	menuItem->cutoff = 2.5f;
	menuItem->width = 0.5f;
	menu->addChild(menuItem);
	menuItem = MenuItem::create<LDMenuItem>("TTL Levels");
	menuItem->ld106 = ld106;
	menuItem->cutoff = 1.4f;
	menuItem->width = 0.6f;
	menu->addChild(menuItem);
	((LD_106 *)module)->appendContextMenu(menu);
}

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, LD106) {
   Model *modelLD106 = Model::create<LD_106, LD106>("SubmarineFree", "LD-106", "LD-106 Schmitt Trigger Line Drivers", LOGIC_TAG, MULTIPLE_TAG);
   return modelLD106;
}
