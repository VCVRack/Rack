#include "Fundamental.hpp"
#include "dsp/vumeter.hpp"

namespace rack_plugin_Fundamental {

struct Unity : Module {
	enum ParamIds {
		AVG1_PARAM,
		AVG2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN1_INPUT,
		IN2_INPUT = IN1_INPUT + 6,
		NUM_INPUTS = IN2_INPUT + 6
	};
	enum OutputIds {
		MIX1_OUTPUT,
		INV1_OUTPUT,
		MIX2_OUTPUT,
		INV2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		VU1_LIGHT,
		VU2_LIGHT = VU1_LIGHT + 5,
		NUM_LIGHTS = VU2_LIGHT + 5
	};

	bool merge = false;

	Unity() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	void onReset() override {
		merge = false;
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();
		// merge
		json_object_set_new(rootJ, "merge", json_boolean(merge));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// merge
		json_t *mergeJ = json_object_get(rootJ, "merge");
		if (mergeJ)
			merge = json_boolean_value(mergeJ);
	}
};

void Unity::step() {
	float mix[2] = {};
	int count[2] = {};

	for (int i = 0; i < 2; i++) {
		// Inputs
		for (int j = 0; j < 6; j++) {
			mix[i] += inputs[IN1_INPUT + 6*i + j].value;
			if (inputs[IN1_INPUT + 6*i + j].active)
				count[i]++;
		}
	}

	// Combine
	if (merge) {
		mix[0] += mix[1];
		mix[1] = mix[0];
		count[0] += count[1];
		count[1] = count[0];
	}

	for (int i = 0; i < 2; i++) {
		// Params
		if ((int) params[AVG1_PARAM + i].value == 1 && count[i] > 0)
			mix[i] /= count[i];

		// Outputs
		outputs[MIX1_OUTPUT + 2*i].value = mix[i];
		outputs[INV1_OUTPUT + 2*i].value = -mix[i];
		// Lights
		VUMeter vuMeter;
		vuMeter.dBInterval = 6.0f;
		vuMeter.setValue(mix[i] / 10.0f);
		for (int j = 0; j < 5; j++) {
			lights[VU1_LIGHT + 5*i + j].setBrightnessSmooth(vuMeter.getBrightness(j));
		}
	}
}


struct UnityWidget : ModuleWidget {
	UnityWidget(Unity *module);
	void appendContextMenu(Menu *menu) override;
};

UnityWidget::UnityWidget(Unity *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/Unity.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

	addParam(ParamWidget::create<CKSS>(mm2px(Vec(12.867, 52.961)), module, Unity::AVG1_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<CKSS>(mm2px(Vec(12.867, 107.006)), module, Unity::AVG2_PARAM, 0.0f, 1.0f, 0.0f));

	addInput(Port::create<PJ301MPort>(mm2px(Vec(2.361, 17.144)), Port::INPUT, module, Unity::IN1_INPUT + 0));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(19.907, 17.144)), Port::INPUT, module, Unity::IN1_INPUT + 1));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(2.361, 28.145)), Port::INPUT, module, Unity::IN1_INPUT + 2));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(19.907, 28.145)), Port::INPUT, module, Unity::IN1_INPUT + 3));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(2.361, 39.145)), Port::INPUT, module, Unity::IN1_INPUT + 4));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(19.907, 39.145)), Port::INPUT, module, Unity::IN1_INPUT + 5));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(2.361, 71.145)), Port::INPUT, module, Unity::IN2_INPUT + 0));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(19.907, 71.145)), Port::INPUT, module, Unity::IN2_INPUT + 1));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(2.361, 82.145)), Port::INPUT, module, Unity::IN2_INPUT + 2));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(19.907, 82.145)), Port::INPUT, module, Unity::IN2_INPUT + 3));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(2.361, 93.144)), Port::INPUT, module, Unity::IN2_INPUT + 4));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(19.907, 93.144)), Port::INPUT, module, Unity::IN2_INPUT + 5));

	addOutput(Port::create<PJ301MPort>(mm2px(Vec(2.361, 54.15)), Port::OUTPUT, module, Unity::MIX1_OUTPUT));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(19.907, 54.15)), Port::OUTPUT, module, Unity::INV1_OUTPUT));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(2.361, 108.144)), Port::OUTPUT, module, Unity::MIX2_OUTPUT));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(19.907, 108.144)), Port::OUTPUT, module, Unity::INV2_OUTPUT));

	addChild(ModuleLightWidget::create<MediumLight<RedLight>>(mm2px(Vec(13.652, 19.663)), module, Unity::VU1_LIGHT + 0));
	addChild(ModuleLightWidget::create<MediumLight<YellowLight>>(mm2px(Vec(13.652, 25.163)), module, Unity::VU1_LIGHT + 1));
	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(mm2px(Vec(13.652, 30.663)), module, Unity::VU1_LIGHT + 2));
	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(mm2px(Vec(13.652, 36.162)), module, Unity::VU1_LIGHT + 3));
	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(mm2px(Vec(13.652, 41.662)), module, Unity::VU1_LIGHT + 4));
	addChild(ModuleLightWidget::create<MediumLight<RedLight>>(mm2px(Vec(13.652, 73.663)), module, Unity::VU2_LIGHT + 0));
	addChild(ModuleLightWidget::create<MediumLight<YellowLight>>(mm2px(Vec(13.652, 79.163)), module, Unity::VU2_LIGHT + 1));
	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(mm2px(Vec(13.652, 84.663)), module, Unity::VU2_LIGHT + 2));
	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(mm2px(Vec(13.652, 90.162)), module, Unity::VU2_LIGHT + 3));
	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(mm2px(Vec(13.652, 95.662)), module, Unity::VU2_LIGHT + 4));
}


struct UnityMergeItem : MenuItem {
	Unity *unity;
	void onAction(EventAction &e) override {
		unity->merge ^= true;
	}
	void step() override {
		rightText = CHECKMARK(unity->merge);
	}
};

void UnityWidget::appendContextMenu(Menu *menu) {
	menu->addChild(MenuEntry::create());

	Unity *unity = dynamic_cast<Unity*>(module);
	assert(unity);

	UnityMergeItem *mergeItem = MenuItem::create<UnityMergeItem>("Merge channels 1 & 2");
	mergeItem->unity = unity;
	menu->addChild(mergeItem);
}

} // namespace rack_plugin_Fundamental

using namespace rack_plugin_Fundamental;

RACK_PLUGIN_MODEL_INIT(Fundamental, Unity) {
   Model *modelUnity = Model::create<Unity, UnityWidget>("Fundamental", "Unity", "Unity", MIXER_TAG, UTILITY_TAG, DUAL_TAG);
   return modelUnity;
}
