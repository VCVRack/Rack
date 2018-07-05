#include "ML_modules.hpp"

namespace rack_plugin_ML_modules {

struct Quant : Module {
	enum ParamIds {
		AMOUNT1_PARAM,
		AMOUNT2_PARAM,
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
	enum LighIds {
		NUM_LIGHTS
	};

	Quant() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {};

	void step() override;
};


static void stepChannel(Input &in, Param &p_amount, Output &out) {



	float v = in.normalize(0.0);

	float amount = p_amount.value;

	int octave = round(v);

	float rest = v - 1.0*octave;

	int semi   = round( rest*12.0 );

	float quantized = 1.0*octave + semi/12.0;

	out.value = quantized + amount*(v-quantized);

}

void Quant::step() {
	if (outputs[OUT1_OUTPUT].active) stepChannel(inputs[IN1_INPUT], params[AMOUNT1_PARAM], outputs[OUT1_OUTPUT]);
	if (outputs[OUT2_OUTPUT].active) stepChannel(inputs[IN2_INPUT], params[AMOUNT2_PARAM], outputs[OUT2_OUTPUT]);
}


struct QuantizerWidget : ModuleWidget {
	QuantizerWidget(Quant *module);
};

QuantizerWidget::QuantizerWidget(Quant *module) : ModuleWidget(module) {
	box.size = Vec(15*3, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/Quantizer.svg")));
		addChild(panel);
	}

	addChild(Widget::create<MLScrew>(Vec(15, 0)));
	addChild(Widget::create<MLScrew>(Vec(15, 365)));

	addParam(ParamWidget::create<SmallBlueMLKnob>(Vec(9,  60), module, Quant::AMOUNT1_PARAM, -1.0, 1.0, 0.0));
	addInput( Port::create<MLPort>(Vec(9, 104), Port::INPUT, module, Quant::IN1_INPUT));
	addOutput(Port::create<MLPort>(Vec(9, 150), Port::OUTPUT, module, Quant::OUT1_OUTPUT));

	addParam(ParamWidget::create<SmallBlueMLKnob>(Vec(9, 203), module, Quant::AMOUNT2_PARAM, -1.0, 1.0, 0.0));
	addInput( Port::create<MLPort>(Vec(9, 246), Port::INPUT, module, Quant::IN2_INPUT));
	addOutput(Port::create<MLPort>(Vec(9, 292), Port::OUTPUT, module, Quant::OUT2_OUTPUT));
}

} // namespace rack_plugin_ML_modules

using namespace rack_plugin_ML_modules;

RACK_PLUGIN_MODEL_INIT(ML_modules, Quantizer) {
   Model *modelQuantizer = Model::create<Quant, QuantizerWidget>("ML modules", "Quantizer", "Quantizer (h-bar)", QUANTIZER_TAG);
   return modelQuantizer;
}
