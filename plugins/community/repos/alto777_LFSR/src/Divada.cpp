#include "LFSR.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_alto777_LFSR {

struct Divada : Module {
	enum ParamIds {
		ENUMS(DIVIDE_BY_PARAM, 5),
		RESET_PARAM,

		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(CLOCK_IN_INPUT, 5),
		RESET_INPUT,
		
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(CLOCK_OUT_OUTPUT, 5),

		NUM_OUTPUTS
	};
	enum LightIds {
		RESET_LAMP,

		NUM_LIGHTS
	};
	
	SchmittTrigger clockTrigger[5];
	const int factor[12] = {2, 3, 4, 5, 7, 8, 11, 13, 16, 17, 19, 23,};
	
	int divCount[5] = {0, 0, 0, 0, 0};		/* [] */

	SchmittTrigger resetTrigger;
	float resetLight = 0.0;
	
	Divada() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

struct mySmallSnapKnob : RoundSmallBlackKnob {
	mySmallSnapKnob() {
		snap = true;
		smooth = false;
	}
};

void Divada::step(/**/) {
	const float lightLambda = 0.075;

	if (resetTrigger.process(params[RESET_PARAM].value + inputs[RESET_INPUT].value)) {
		for (int ii = 0; ii < 5; ii++)
			divCount[ii] = 0;
		resetLight = 1.0f;
	}

	lights[RESET_LAMP].value = resetLight;
	resetLight -= resetLight / lightLambda / engineGetSampleRate();

	for (int ii = 0; ii < 5; ii++) {
		int divBy = int(params[DIVIDE_BY_PARAM + ii].value + 0.1);
		divBy = factor[divBy];

		if (clockTrigger[ii].process(inputs[CLOCK_IN_INPUT + ii].value))	
			if (++divCount[ii] >= divBy) divCount[ii] = 0;
		outputs[CLOCK_OUT_OUTPUT + ii].value = (divCount[ii] < (divBy / 2)) ? 10.0f : 0.0f;
	}
}

struct DivadaWidget : ModuleWidget {
  DivadaWidget(Divada *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Divada.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


	float xmarginR = 1.0f; float xmarginL = 21.125;
	float ymargin = 16.0f;
	float inup = 5.0f;
	float unitSpace = 19.0f;

	for (int ii = 0; ii < 5; ii++) {
		addInput(Port::create<PJ301MPort>(mm2px(Vec(xmarginR, ymargin + ii * unitSpace - inup)), Port::INPUT, module, Divada::CLOCK_IN_INPUT + ii));
		addParam(ParamWidget::create<mySmallSnapKnob>(mm2px(Vec(11.24, ymargin + 0.1775f + ii * unitSpace)), module, Divada::DIVIDE_BY_PARAM + ii, 0.0, 11.0, 0.0));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(xmarginL, ymargin + ii * unitSpace + inup)), Port::OUTPUT, module, Divada::CLOCK_OUT_OUTPUT + ii));
	}


	addInput(Port::create<PJ301MPort>(mm2px(Vec(xmarginL - 5.0f, ymargin + 5.0f + 5 * unitSpace)), Port::INPUT, module, Divada::RESET_INPUT));
	addParam(ParamWidget::create<LEDButton>(mm2px(Vec(7.0f - 3.0f + 0.135f, ymargin + 5.0f + 5 * unitSpace + 1.0f)), module, Divada::RESET_PARAM, 0.0, 1.0, 0.0));
	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(mm2px(Vec(7.0f - 3.0f + 1.49f + 0.135f, ymargin + 5.0f + 5 * unitSpace + 1.49f + 1.0f)), module, Divada::RESET_LAMP));

  }
};

} // namespace rack_plugin_alto777_LFSR

using namespace rack_plugin_alto777_LFSR;

RACK_PLUGIN_MODEL_INIT(alto777_LFSR, Divada) {
   Model *modelDivada = Model::create<Divada, DivadaWidget>("alto777_LFSR", "Divada", "Divada VIS", OSCILLATOR_TAG);
   return modelDivada;
}


