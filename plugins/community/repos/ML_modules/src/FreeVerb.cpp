#include "ML_modules.hpp"
#include "../freeverb/revmodel.hpp"

#include "dsp/digital.hpp"

namespace rack_plugin_ML_modules {

struct FreeVerb : Module {
	enum ParamIds {
		ROOMSIZE_PARAM,
		DAMP_PARAM,
		FREEZE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_INPUT,
		ROOMSIZE_INPUT,
		DAMP_INPUT,
		FREEZE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};

	enum LighIds {
		FREEZE_LIGHT,
		NUM_LIGHTS
	};
	revmodel reverb;

	float roomsize, damp; 


	bool freeze=false;

	SchmittTrigger buttonTrigger;

	FreeVerb() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) {

	float gSampleRate = engineGetSampleRate();

		reverb.init(gSampleRate);

	};

	void step() override;

	void onSampleRateChange() override;


};


void FreeVerb::onSampleRateChange() {

	float gSampleRate = engineGetSampleRate();

	reverb.init(gSampleRate);

	reverb.setdamp(damp);
	reverb.setroomsize(roomsize);

};

void FreeVerb::step() {

	float out1, out2;

	out1 = out2 = 0.0;

	float old_roomsize = roomsize;
	float old_damp = damp;
	bool  old_freeze = freeze;


	float input = clamp(inputs[IN_INPUT].value,-10.0f,10.0f);

	if(inputs[ROOMSIZE_INPUT].active) {
		roomsize = clamp(inputs[ROOMSIZE_INPUT].value/8.0f, 0.0f, 1.0f);
	} else {
		roomsize = params[ROOMSIZE_PARAM].value;
	};

	if(inputs[DAMP_INPUT].active) {
		damp     = clamp(inputs[DAMP_INPUT].value/8.0f, 0.0f, 1.0f);
	} else {
		damp     = params[DAMP_PARAM].value;
	};

	if(inputs[FREEZE_INPUT].active) {
		freeze = inputs[FREEZE_INPUT].value > 1.0;
	} else {
		if(buttonTrigger.process(params[FREEZE_PARAM].value)) freeze = !freeze;
	};


	if( old_damp != damp ) reverb.setdamp(damp);
	if( old_roomsize != roomsize) reverb.setroomsize(roomsize);

	lights[FREEZE_LIGHT].value = freeze?10.0:0.0;


	if(freeze != old_freeze) reverb.setmode(freeze?1.0:0.0);

	reverb.process(input, out1, out2);


	outputs[OUT1_OUTPUT].value = out1;
	outputs[OUT2_OUTPUT].value = out2;
};



struct FreeVerbWidget : ModuleWidget {
	FreeVerbWidget(FreeVerb *module);
};

FreeVerbWidget::FreeVerbWidget(FreeVerb *module) : ModuleWidget(module) {

	box.size = Vec(15*6, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/FreeVerb.svg")));
		addChild(panel);
	}

	addChild(Widget::create<MLScrew>(Vec(15, 0)));
	addChild(Widget::create<MLScrew>(Vec(15, 365)));

	addInput(Port::create<MLPort>(Vec(33, 50), Port::INPUT, module, FreeVerb::IN_INPUT));

	addInput(Port::create<MLPort>(Vec(53, 120), Port::INPUT, module, FreeVerb::ROOMSIZE_INPUT));
	addInput(Port::create<MLPort>(Vec(53, 183), Port::INPUT, module, FreeVerb::DAMP_INPUT));
	addInput(Port::create<MLPort>(Vec(53, 246), Port::INPUT, module, FreeVerb::FREEZE_INPUT));

    addParam(ParamWidget::create<SmallBlueMLKnob>(Vec(10, 122), module, FreeVerb::ROOMSIZE_PARAM, 0.0, 1.0, 0.5));
    addParam(ParamWidget::create<SmallBlueMLKnob>(Vec(10, 186), module, FreeVerb::DAMP_PARAM, 0.0, 1.0, 0.5));
    addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(14, 250), module, FreeVerb::FREEZE_PARAM, 0.0, 10.0, 0.0));
    addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>(Vec(18,254), module, FreeVerb::FREEZE_LIGHT));

	addOutput(Port::create<MLPort>(Vec(11, 313), Port::OUTPUT, module, FreeVerb::OUT1_OUTPUT));
	addOutput(Port::create<MLPort>(Vec(55, 313), Port::OUTPUT, module, FreeVerb::OUT2_OUTPUT));
}

} // namespace rack_plugin_ML_modules

using namespace rack_plugin_ML_modules;

RACK_PLUGIN_MODEL_INIT(ML_modules, FreeVerb) {
   Model *modelFreeVerb = Model::create<FreeVerb, FreeVerbWidget>("ML modules", "FreeVerb", "FreeVerb", REVERB_TAG);
   return modelFreeVerb;
}
