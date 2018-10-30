#include "XF.hpp"

namespace rack_plugin_SubmarineFree {

struct XF_202 : XF {
	static const int deviceCount = 2;
	enum ParamIds {
		PARAM_CV_1, PARAM_CV_2,
		PARAM_MODE_1, PARAM_MODE_2,
		PARAM_FADE_1, PARAM_FADE_2,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_A_1, INPUT_A_2,
		INPUT_AR_1, INPUT_AR_2,
		INPUT_B_1, INPUT_B_2,
		INPUT_BR_1, INPUT_BR_2,
		INPUT_CV_1, INPUT_CV_2,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1, OUTPUT_2,
		OUTPUTR_1, OUTPUTR_2,
		NUM_OUTPUTS
	};
	enum LightIds {
		LIGHT_LIN_1, LIGHT_LIN_2,
		LIGHT_LOG_1, LIGHT_LOG_2,
		LIGHT_AUTO_1, LIGHT_INV_1, LIGHT_AUTO_2, LIGHT_INV_2,
		NUM_LIGHTS
	};
	XF_Correlator correlators[deviceCount];
	XF_Controls controls[deviceCount];

	XF_202() : XF(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		for (int i = 0; i < deviceCount; i++) {
			controls[i].a = INPUT_A_1 + i;
			controls[i].ar = INPUT_AR_1 + i;
			controls[i].b = INPUT_B_1 + i;
			controls[i].br = INPUT_BR_1 + i;
			controls[i].fader = PARAM_FADE_1 + i;
			controls[i].cv = INPUT_CV_1 + i;
			controls[i].out = OUTPUT_1 + i;
			controls[i].outr = OUTPUTR_1 + i;
			controls[i].polar = PARAM_CV_1 + i;
			controls[i].mode = PARAM_MODE_1 + i;
			controls[i].light1 = LIGHT_LIN_1 + i;
			controls[i].light2 = LIGHT_LOG_1 + i;
			controls[i].light3 = LIGHT_AUTO_1 + i * 2;
			controls[i].correlator = &correlators[i];
		}
	}
	void step() override;
};

void XF_202::step() {
	crossFade(&controls[0]);
	crossFade(&controls[1]);
}

struct XF202 : ModuleWidget {
	XF202(XF_202 *module) : ModuleWidget(module) {
		XF_LightKnob *fader;
		setPanel(SVG::load(assetPlugin(plugin, "res/XF-202.svg")));
		for (int i = 0; i < XF_202::deviceCount; i++) {
			int offset = 176 * i;
			addInput(Port::create<SilverPort>(Vec(3,18 + offset), Port::INPUT, module, XF_202::INPUT_A_1 + i));
			addInput(Port::create<RedPort>(Vec(3,45 + offset), Port::INPUT, module, XF_202::INPUT_AR_1 + i));
			addInput(Port::create<SilverPort>(Vec(92,18 + offset), Port::INPUT, module, XF_202::INPUT_B_1 + i));
			addInput(Port::create<RedPort>(Vec(92,45 + offset), Port::INPUT, module, XF_202::INPUT_BR_1 + i));
			addInput(Port::create<SilverPort>(Vec(3,120 + offset), Port::INPUT, module, XF_202::INPUT_CV_1 + i));

			addOutput(Port::create<SilverPort>(Vec(92,93 + offset), Port::OUTPUT, module, XF_202::OUTPUT_1 + i));
			addOutput(Port::create<RedPort>(Vec(92,120 + offset), Port::OUTPUT, module, XF_202::OUTPUTR_1 + i));

			addParam(ParamWidget::create<sub_sw_2>(Vec(28, 154.5 + offset), module, XF_202::PARAM_CV_1 + i, 0.0f, 1.0f, 0.0f));
			addParam(ParamWidget::create<sub_sw_3>(Vec(65, 152 + offset), module, XF_202::PARAM_MODE_1 + i, 0.0f, 2.0f, 0.0f));
			fader = ParamWidget::create<XF_LightKnob>(Vec(33, 51 + offset), module, XF_202::PARAM_FADE_1 + i, 0.0f, 10.0f, 5.0f);
			fader->cv = XF_202::INPUT_CV_1 + i;
			fader->link = 0;
			addParam(fader);

			addChild(ModuleLightWidget::create<TinyLight<BlueLight>>(Vec(81, 156 + offset), module, XF_202::LIGHT_LIN_1 + i));
			addChild(ModuleLightWidget::create<TinyLight<BlueLight>>(Vec(81, 166 + offset), module, XF_202::LIGHT_LOG_1 + i));
			addChild(ModuleLightWidget::create<TinyLight<BlueRedLight>>(Vec(81, 176 + offset), module, XF_202::LIGHT_AUTO_1 + i * 2));
		}

	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, XF202) {
   Model *modelXF202 = Model::create<XF_202, XF202>("Submarine (Free)", "XF-202", "XF-202 Dual Stereo Cross Fader", MIXER_TAG, DUAL_TAG);
   return modelXF202;
}
