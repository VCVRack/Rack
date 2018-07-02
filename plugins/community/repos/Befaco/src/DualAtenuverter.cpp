#include "Befaco.hpp"


struct DualAtenuverter : Module {
	enum ParamIds {
		ATEN1_PARAM,
		OFFSET1_PARAM,
		ATEN2_PARAM,
		OFFSET2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN1_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		OUT1_POS_LIGHT,
		OUT1_NEG_LIGHT,
		OUT2_POS_LIGHT,
		OUT2_NEG_LIGHT,
		NUM_LIGHTS
	};

	DualAtenuverter() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


void DualAtenuverter::step() {
	float out1 = inputs[IN1_INPUT].value * params[ATEN1_PARAM].value + params[OFFSET1_PARAM].value;
	float out2 = inputs[IN2_INPUT].value * params[ATEN2_PARAM].value + params[OFFSET2_PARAM].value;
	out1 = clamp(out1, -10.0f, 10.0f);
	out2 = clamp(out2, -10.0f, 10.0f);

	outputs[OUT1_OUTPUT].value = out1;
	outputs[OUT2_OUTPUT].value = out2;
	lights[OUT1_POS_LIGHT].value = fmaxf(0.0, out1 / 5.0);
	lights[OUT1_NEG_LIGHT].value = fmaxf(0.0, -out1 / 5.0);
	lights[OUT2_POS_LIGHT].value = fmaxf(0.0, out2 / 5.0);
	lights[OUT2_NEG_LIGHT].value = fmaxf(0.0, -out2 / 5.0);
}


struct DualAtenuverterWidget : ModuleWidget {
	DualAtenuverterWidget(DualAtenuverter *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/DualAtenuverter.svg")));

		addChild(Widget::create<Knurlie>(Vec(15, 0)));
		addChild(Widget::create<Knurlie>(Vec(15, 365)));

		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(20, 33), module, DualAtenuverter::ATEN1_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Davies1900hRedKnob>(Vec(20, 91), module, DualAtenuverter::OFFSET1_PARAM, -10.0, 10.0, 0.0));
		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(20, 201), module, DualAtenuverter::ATEN2_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Davies1900hRedKnob>(Vec(20, 260), module, DualAtenuverter::OFFSET2_PARAM, -10.0, 10.0, 0.0));

		addInput(Port::create<PJ301MPort>(Vec(7, 152), Port::INPUT, module, DualAtenuverter::IN1_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(43, 152), Port::OUTPUT, module, DualAtenuverter::OUT1_OUTPUT));

		addInput(Port::create<PJ301MPort>(Vec(7, 319), Port::INPUT, module, DualAtenuverter::IN2_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(43, 319), Port::OUTPUT, module, DualAtenuverter::OUT2_OUTPUT));

		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(33, 143), module, DualAtenuverter::OUT1_POS_LIGHT));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(33, 311), module, DualAtenuverter::OUT2_POS_LIGHT));
	}
};


RACK_PLUGIN_MODEL_INIT(Befaco, DualAtenuverter) {
   Model *modelDualAtenuverter = Model::create<DualAtenuverter, DualAtenuverterWidget>("Befaco", "DualAtenuverter", "Dual Atenuverter", ATTENUATOR_TAG, DUAL_TAG);
   return modelDualAtenuverter;
}
