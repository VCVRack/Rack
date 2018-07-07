#include <string.h>
#include <math.h>
#include <float.h>
#include "LOGinstruments.hpp"
#include "LOGgui.hpp"

namespace rack_plugin_LOGinstruments {
/*
 * 3x3 MATRIX MOD
 */
struct Britix : Module {
	enum ParamIds {
		MAT1_A_A_PIN,
		MAT1_A_B_PIN,
		MAT1_A_C_PIN,
		MAT1_B_A_PIN,
		MAT1_B_B_PIN,
		MAT1_B_C_PIN,
		MAT1_C_A_PIN,
		MAT1_C_B_PIN,
		MAT1_C_C_PIN,
		MAT2_A_A_PIN,
		MAT2_A_B_PIN,
		MAT2_A_C_PIN,
		MAT2_B_A_PIN,
		MAT2_B_B_PIN,
		MAT2_B_C_PIN,
		MAT2_C_A_PIN,
		MAT2_C_B_PIN,
		MAT2_C_C_PIN,
		NUM_PARAMS_1 = MAT1_C_C_PIN+1,
		NUM_PARAMS_2 = NUM_PARAMS_1,
		NUM_PARAMS_TOT = NUM_PARAMS_1 + NUM_PARAMS_2,
	};
	enum InputIds {
		INPUT1_A,
		INPUT1_B,
		INPUT1_C,
		INPUT2_A,
		INPUT2_B,
		INPUT2_C,
		NUM_INPUTS_1 = INPUT1_C +1,
		NUM_INPUTS_2 = NUM_INPUTS_1,
		NUM_INPUTS_TOT = NUM_INPUTS_1 + NUM_INPUTS_2
	};
	enum OutputIds {
		OUTPUT1_A,
		OUTPUT1_B,
		OUTPUT1_C,
		OUTPUT1_SUM,
		OUTPUT2_A,
		OUTPUT2_B,
		OUTPUT2_C,
		OUTPUT2_SUM,
		NUM_OUTPUTS_1 = OUTPUT1_SUM +1,
		NUM_OUTPUTS_2 = NUM_OUTPUTS_1,
		NUM_OUTPUTS_TOT = NUM_OUTPUTS_1 + NUM_OUTPUTS_2
	};

	enum LightIds {
		LIGHTS1_1,
		LIGHTS1_2,
		LIGHTS1_3,
		LIGHTS2_1,
		LIGHTS2_2,
		LIGHTS2_3,
		NUM_LIGHTS,
	};

	float lights1[3] = {};
	float lights2[3] = {};

	Britix() : Module(NUM_PARAMS_TOT, NUM_INPUTS_TOT, NUM_OUTPUTS_TOT, NUM_LIGHTS) {}
	void step() override ;

	void reset() override {
	}
};

inline float op(float op1, float op2, char operation) {
	if (operation == '+')
		return op1 + op2;
	if (operation == '-')
		return op1 - op2;
	if (operation == '*')
		return op1 * op2;
	return 0.0; // unkown operation
}

void Britix::step() {

	// TOP
	float in1[NUM_INPUTS_1] = {0.0};
	float out1[NUM_OUTPUTS_1] = {0.0};
	char ops1[NUM_INPUTS_1*NUM_INPUTS_1]; // operations

	// acquire pin state
	// pins values: none 0.0, red 1.0, blue 2.0 and 3.0 black
	for (int i = 0; i < NUM_INPUTS_1*NUM_INPUTS_1; i++) {
		if (params[i].value == 1.0) {
			ops1[i] = '+';
		} else if (params[i].value == 2.0) {
			ops1[i] = '-';
		} else if (params[i].value == 3.0) {
			ops1[i] = '*';
		} else
			ops1[i] = '\0'; // unknown
	}

	// acquire inputs
	for (int i = 0; i < NUM_INPUTS_1; i++) {
		if (inputs[i].active) {
			in1[i] = lights[LIGHTS1_1+i].value = (inputs[i].value);
		} else {
			lights[LIGHTS1_1+i].value = 0.0;
		}
	}

	// process (i,j) = (rows, columns)
	for (int i = 0; i < NUM_INPUTS_1; i++) {
		for (int j = 0; j < NUM_INPUTS_1; j++) {
			out1[i] += op(in1[i], in1[j], ops1[i*NUM_INPUTS_1+j]);
		}
		out1[i] = out1[i] / NUM_INPUTS_2; // better way?
	}

	// compute sigma output (sum of all)
	for (int i = 0; i < NUM_INPUTS_1; i++)
		out1[OUTPUT1_SUM] += out1[i];
	outputs[OUTPUT1_SUM].value = out1[OUTPUT1_SUM];

	// outputs
	for (int i = 0; i < NUM_OUTPUTS_1; i++) {
		if (outputs[i].active) {
			outputs[i].value = out1[i];
		}
	}

	// BOTTOM
	float in2[NUM_INPUTS_2] = {0.0};
	float out2[NUM_OUTPUTS_2] = {0.0};
	float matstat[NUM_INPUTS_2*NUM_INPUTS_2]; // operations

	// acquire pin state
	// pins values: none 0.0, white 1.0
	for (int i = 0; i < NUM_INPUTS_2*NUM_INPUTS_2; i++) {
		if (params[i+MAT2_A_A_PIN].value == 1.0) {
			matstat[i] = 1.0;
		} else
			matstat[i] = 0.0;
	}

	// acquire inputs
	for (int i = 0; i < NUM_INPUTS_2; i++) {
		if (inputs[i+INPUT2_A].active) {
			in2[i] = lights[LIGHTS2_1+i].value = (inputs[i+INPUT2_A].value);
		} else {
			lights[LIGHTS2_1+i].value = 0.0;
		}
	}

	// process (i,j) = (rows, columns)
	for (int i = 0; i < NUM_INPUTS_2; i++) {
		for (int j = 0; j < NUM_INPUTS_2; j++) {
			out2[i] += matstat[i*NUM_INPUTS_2+j] * in2[j];
		}
		out2[i] = out2[i] / NUM_INPUTS_2; // better way?
	}


	// compute sigma output (sum of all)
	for (int i = 0; i < NUM_INPUTS_2; i++)
		out2[OUTPUT2_SUM-OUTPUT1_SUM-1] += out2[i];
	outputs[OUTPUT2_SUM].value = out2[OUTPUT2_SUM-OUTPUT1_SUM-1];

	// outputs
	for (int i = 0; i < NUM_OUTPUTS_2; i++) {
		if (outputs[i+OUTPUT2_A].active) {
			outputs[i+OUTPUT2_A].value = out2[i];
		}
	}

}

struct BritixWidget : ModuleWidget {
	BritixWidget(Britix *module);
};

BritixWidget::BritixWidget(Britix *module) : ModuleWidget(module) {


	box.size = Vec(15*20, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Britix_nofonts.svg")));
		addChild(panel);
	}

	// TOP
	addParam(ParamWidget::create<VCSPin4State>(Vec(68,56), module, Britix::MAT1_A_A_PIN, 0.0, 3.0, 0.0));
	addParam(ParamWidget::create<VCSPin4State>(Vec(98,56), module, Britix::MAT1_A_B_PIN, 0.0, 3.0, 0.0));
	addParam(ParamWidget::create<VCSPin4State>(Vec(128,56), module, Britix::MAT1_A_C_PIN, 0.0, 3.0, 0.0));
	addParam(ParamWidget::create<VCSPin4State>(Vec(68,86), module, Britix::MAT1_B_A_PIN, 0.0, 3.0, 0.0));
	addParam(ParamWidget::create<VCSPin4State>(Vec(98,86), module, Britix::MAT1_B_B_PIN, 0.0, 3.0, 0.0));
	addParam(ParamWidget::create<VCSPin4State>(Vec(128,86), module, Britix::MAT1_B_C_PIN, 0.0, 3.0, 0.0));
	addParam(ParamWidget::create<VCSPin4State>(Vec(68,116), module, Britix::MAT1_C_A_PIN, 0.0, 3.0, 0.0));
	addParam(ParamWidget::create<VCSPin4State>(Vec(98,116), module, Britix::MAT1_C_B_PIN, 0.0, 3.0, 0.0));
	addParam(ParamWidget::create<VCSPin4State>(Vec(128,116), module, Britix::MAT1_C_C_PIN, 0.0, 3.0, 0.0));

	addInput(Port::create<PJ301MPort>(Vec(9, 58), Port::INPUT, module, Britix::INPUT1_A));
	addInput(Port::create<PJ301MPort>(Vec(9, 88), Port::INPUT, module, Britix::INPUT1_B));
	addInput(Port::create<PJ301MPort>(Vec(9, 118), Port::INPUT, module, Britix::INPUT1_C));

	addOutput(Port::create<PJ3410Port>(Vec(230, 58), Port::OUTPUT, module, Britix::OUTPUT1_A));
	addOutput(Port::create<PJ3410Port>(Vec(230, 88), Port::OUTPUT, module, Britix::OUTPUT1_B));
	addOutput(Port::create<PJ3410Port>(Vec(230, 118), Port::OUTPUT, module, Britix::OUTPUT1_C));
	addOutput(Port::create<PJ3410Port>(Vec(230, 148), Port::OUTPUT, module, Britix::OUTPUT1_SUM));

	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(81, 167), module, Britix::LIGHTS1_1));
	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(111, 167), module, Britix::LIGHTS1_2));
	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(141, 167), module, Britix::LIGHTS1_3));

	//BOTTOM
#define BOTTOM_VDIST 175
	addParam(ParamWidget::create<VCSPin2State>(Vec(68,56+BOTTOM_VDIST), module, Britix::MAT2_A_A_PIN, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<VCSPin2State>(Vec(98,56+BOTTOM_VDIST), module, Britix::MAT2_A_B_PIN, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<VCSPin2State>(Vec(128,56+BOTTOM_VDIST), module, Britix::MAT2_A_C_PIN, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<VCSPin2State>(Vec(68,86+BOTTOM_VDIST), module, Britix::MAT2_B_A_PIN, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<VCSPin2State>(Vec(98,86+BOTTOM_VDIST), module, Britix::MAT2_B_B_PIN, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<VCSPin2State>(Vec(128,86+BOTTOM_VDIST), module, Britix::MAT2_B_C_PIN, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<VCSPin2State>(Vec(68,116+BOTTOM_VDIST), module, Britix::MAT2_C_A_PIN, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<VCSPin2State>(Vec(98,116+BOTTOM_VDIST), module, Britix::MAT2_C_B_PIN, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<VCSPin2State>(Vec(128,116+BOTTOM_VDIST), module, Britix::MAT2_C_C_PIN, 0.0, 1.0, 0.0));

	addInput(Port::create<PJ301MPort>(Vec(9, 58+BOTTOM_VDIST), Port::INPUT, module, Britix::INPUT2_A));
	addInput(Port::create<PJ301MPort>(Vec(9, 88+BOTTOM_VDIST), Port::INPUT, module, Britix::INPUT2_B));
	addInput(Port::create<PJ301MPort>(Vec(9, 118+BOTTOM_VDIST), Port::INPUT, module, Britix::INPUT2_C));

	addOutput(Port::create<PJ3410Port>(Vec(230, 58+BOTTOM_VDIST), Port::OUTPUT, module, Britix::OUTPUT2_A));
	addOutput(Port::create<PJ3410Port>(Vec(230, 88+BOTTOM_VDIST), Port::OUTPUT, module, Britix::OUTPUT2_B));
	addOutput(Port::create<PJ3410Port>(Vec(230, 118+BOTTOM_VDIST), Port::OUTPUT, module, Britix::OUTPUT2_C));
	addOutput(Port::create<PJ3410Port>(Vec(230, 148+BOTTOM_VDIST), Port::OUTPUT, module, Britix::OUTPUT2_SUM));

	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(81, 167+BOTTOM_VDIST), module, Britix::LIGHTS2_1));
	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(111, 167+BOTTOM_VDIST), module, Britix::LIGHTS2_2));
	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(141, 167+BOTTOM_VDIST), module, Britix::LIGHTS2_3));
}

} // namespace rack_plugin_LOGinstruments

using namespace rack_plugin_LOGinstruments;

RACK_PLUGIN_MODEL_INIT(LOGinstruments, Britix) {
   Model *modelBritix = Model::create<Britix, BritixWidget>("LOGinstruments", "Britix", "Matrix Modulator", LOGIC_TAG, MIXER_TAG, UTILITY_TAG);
   return modelBritix;
}
