#include "XF.hpp"

namespace rack_plugin_SubmarineFree {

struct XF_102 : XF {
	static const int deviceCount = 2;
	enum ParamIds {
		PARAM_CV_1, PARAM_CV_2,
		PARAM_MODE_1, PARAM_MODE_2,
		PARAM_FADE_1, PARAM_FADE_2,
		PARAM_LINK_1,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_A_1, INPUT_A_2,
		INPUT_B_1, INPUT_B_2,
		INPUT_CV_1, INPUT_CV_2,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1, OUTPUT_2,
		NUM_OUTPUTS
	};
	enum LightIds {
		LIGHT_LIN_1, LIGHT_LIN_2,
		LIGHT_LOG_1, LIGHT_LOG_2,
		LIGHT_AUTO_1, LIGHT_INV_1, LIGHT_AUTO_2, LIGHT_INV_2,
		NUM_LIGHTS
	};
	XF_Correlator correlators[deviceCount];
	XF_Controls controls[(int)(deviceCount * 1.5f)];

	XF_102() : XF(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		for (int i = 0; i < deviceCount; i++) {
			controls[i].a = INPUT_A_1 + i;
			controls[i].ar = 0;
			controls[i].b = INPUT_B_1 + i;
			controls[i].br = 0;
			controls[i].fader = PARAM_FADE_1 + i;
			controls[i].cv = INPUT_CV_1 + i;
			controls[i].out = OUTPUT_1 + i;
			controls[i].outr = 0;
			controls[i].polar = PARAM_CV_1 + i;
			controls[i].mode = PARAM_MODE_1 + i;
			controls[i].light1 = LIGHT_LIN_1 + i;
			controls[i].light2 = LIGHT_LOG_1 + i;
			controls[i].light3 = LIGHT_AUTO_1 + i * 2;
			controls[i].correlator = &correlators[i];
		}
		for (int i = 0; i < deviceCount / 2; i++) {
			int x = i * 2;
			controls[i + deviceCount].a = INPUT_A_1 + x;
			controls[i + deviceCount].ar = INPUT_A_2 + x;
			controls[i + deviceCount].b = INPUT_B_1 + x;
			controls[i + deviceCount].br = INPUT_B_2 + x;
			controls[i + deviceCount].fader = PARAM_FADE_1 + x;
			controls[i + deviceCount].cv = INPUT_CV_1 + x;
			controls[i + deviceCount].out = OUTPUT_1 + x;
			controls[i + deviceCount].outr = OUTPUT_2 + x;
			controls[i + deviceCount].polar = PARAM_CV_1 + x;
			controls[i + deviceCount].mode = PARAM_MODE_1 + x;
			controls[i + deviceCount].light1 = LIGHT_LIN_1 + x;
			controls[i + deviceCount].light2 = LIGHT_LOG_1 + x;
			controls[i + deviceCount].light3 = LIGHT_AUTO_1 + x * 2;
			controls[i + deviceCount].correlator = &correlators[x];
		}
	}
	void step() override;
};

void XF_102::step() {
	if (params[PARAM_LINK_1].value > 0.5f) {
		crossFade(&controls[2]);
	}
	else {
		crossFade(&controls[0]);
		crossFade(&controls[1]);
	}
}

struct XF102 : ModuleWidget {
	XF102(XF_102 *module) : ModuleWidget(module) {
		XF_LightKnob *fader;
		setPanel(SVG::load(assetPlugin(plugin, "res/XF-102.svg")));
		for (int i = 0; i < XF_102::deviceCount; i++) {
			int offset = 88 * i;
			addInput(Port::create<SilverPort>(Vec(27.5,18 + offset), Port::INPUT, module, XF_102::INPUT_A_1 + i));
			addInput(Port::create<SilverPort>(Vec(127.5,18 + offset), Port::INPUT, module, XF_102::INPUT_B_1 + i));
			addInput(Port::create<SilverPort>(Vec(27.5,74 + offset), Port::INPUT, module, XF_102::INPUT_CV_1 + i));

			addOutput(Port::create<SilverPort>(Vec(127.5,74 + offset), Port::OUTPUT, module, XF_102::OUTPUT_1 + i));

			addParam(ParamWidget::create<sub_sw_2>(Vec(41, 46 + offset), module, XF_102::PARAM_CV_1 + i, 0.0f, 1.0f, 0.0f));
			addParam(ParamWidget::create<sub_sw_3>(Vec(125, 43.5 + offset), module, XF_102::PARAM_MODE_1 + i, 0.0f, 2.0f, 0.0f));
			fader = ParamWidget::create<XF_LightKnob>(Vec(63, 31 + offset), module, XF_102::PARAM_FADE_1 + i, 0.0f, 10.0f, 5.0f);
			fader->cv = XF_102::INPUT_CV_1 + i;
			fader->link = i?XF_102::PARAM_LINK_1:0;
			addParam(fader);

			addChild(ModuleLightWidget::create<TinyLight<BlueLight>>(Vec(141, 47 + offset), module, XF_102::LIGHT_LIN_1 + i));
			addChild(ModuleLightWidget::create<TinyLight<BlueLight>>(Vec(141, 57 + offset), module, XF_102::LIGHT_LOG_1 + i));
			addChild(ModuleLightWidget::create<TinyLight<BlueRedLight>>(Vec(141, 67 + offset), module, XF_102::LIGHT_AUTO_1 + i * 2));
		}

		addParam(ParamWidget::create<sub_btn>(Vec(90, 94.5), module, XF_102::PARAM_LINK_1, 0.0f, 1.0f, 0.0f));
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, XF102) {
   Model *modelXF102 = Model::create<XF_102, XF102>("Submarine (Free)", "XF-102", "XF-102 Dual Mono Cross Fader", MIXER_TAG, DUAL_TAG);
   return modelXF102;
}
