#include "ML_modules.hpp"

#include "dsp/digital.hpp"

namespace rack_plugin_ML_modules {

struct SH8 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		IN1_INPUT,
		IN2_INPUT,
		IN3_INPUT,
		IN4_INPUT,
		IN5_INPUT,
		IN6_INPUT,
		IN7_INPUT,
		IN8_INPUT,
		TRIG1_INPUT,
		TRIG2_INPUT,
		TRIG3_INPUT,
		TRIG4_INPUT,
		TRIG5_INPUT,
		TRIG6_INPUT,
		TRIG7_INPUT,
		TRIG8_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT3_OUTPUT,
		OUT4_OUTPUT,
		OUT5_OUTPUT,
		OUT6_OUTPUT,
		OUT7_OUTPUT,
		OUT8_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	SchmittTrigger trigger[8];
	float out[8];


	SH8() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) { reset(); };

	void step() override;

	void reset() override {
		for(int i=0; i<8; i++) out[i] = 0.0;
	};

};



void SH8::step() {

	float in[8], trig[8];

	float random = 0;

	trig[0] = inputs[TRIG1_INPUT].normalize(0.0);
	for(int i=1; i<8; i++) trig[i] = inputs[TRIG1_INPUT+i].normalize(trig[i-1]);



	in[0] = inputs[IN1_INPUT].normalize(random);

	for(int i=1; i<8; i++) in[i] = inputs[IN1_INPUT+i].normalize(in[i-1]);

	for(int i=0; i<8; i++) {

		if( trigger[i].process(trig[i]) ) out[i] = in[i];

	}

	for(int i=0; i<8; i++) outputs[OUT1_OUTPUT+i].value = out[i];

};



struct SH8Widget : ModuleWidget {
	SH8Widget(SH8 *module);
};

SH8Widget::SH8Widget(SH8 *module) : ModuleWidget(module) {

	box.size = Vec(15*8, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/SH8.svg")));

		addChild(panel);
	}

	addChild(Widget::create<MLScrew>(Vec(15, 0)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<MLScrew>(Vec(15, 365)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 365)));




	const float offset_y = 60, delta_y = 32, row1=15, row2 = 48, row3 = 80;

	for( int i=0; i<8; i++) {
		addInput(Port::create<MLPort>(Vec(row1, offset_y + i*delta_y  ), Port::INPUT, module, SH8::IN1_INPUT+i));
		addInput(Port::create<MLPort>(Vec(row2, offset_y + i*delta_y  ), Port::INPUT, module, SH8::TRIG1_INPUT+i));
		addOutput(Port::create<MLPort>(Vec(row3, offset_y + i*delta_y ), Port::OUTPUT, module, SH8::OUT1_OUTPUT+i));
	};


}

} // namespace rack_plugin_ML_modules

using namespace rack_plugin_ML_modules;

RACK_PLUGIN_MODEL_INIT(ML_modules, SH8) {
   Model *modelSH8 = Model::create<SH8, SH8Widget>("ML modules", "SH8", "S&H 8", SAMPLE_AND_HOLD_TAG);
   return modelSH8;
}
