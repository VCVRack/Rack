#include "ML_modules.hpp"

#include "dsp/digital.hpp"

namespace rack_plugin_ML_modules {

struct OctaFlop : Module {
	enum ParamIds {
		RESET_PARAM,
		TOGGLE_PARAM,
		NUM_PARAMS = TOGGLE_PARAM+8
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
		RESET_INPUT,
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
		STATE1_LIGHT,
		STATE2_LIGHT,
		STATE3_LIGHT,
		STATE4_LIGHT,
		STATE5_LIGHT,
		STATE6_LIGHT,
		STATE7_LIGHT,
		STATE8_LIGHT,
		NUM_LIGHTS
	};

	SchmittTrigger trigger[8], resetTrigger;
	float out[8] = {};
	bool state[8] = {};


	OctaFlop() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) { for(int i=0; i<8; i++) state[i]=false; };

	void step() override;

};



void OctaFlop::step() {

	float trig[8];



	trig[0] = inputs[TRIG1_INPUT].normalize(0.0) ;
	for(int i=1; i<8; i++) trig[i] = inputs[TRIG1_INPUT+i].normalize(10.0-out[i-1]) ;


	for(int i=0; i<8; i++) {

		if( trigger[i].process(trig[i]) ) {

			state[i] = !state[i];
			out[i]   = state[i]?10.0:0.0;
		};

	}

	if(resetTrigger.process(params[RESET_PARAM].value + inputs[RESET_INPUT].normalize(0.0))) {
		for(int i=0; i<8; i++) {
			state[i]=false;
			out[i] = 0.0;
			trigger[i].reset();
		};
	};


	for(int i=0; i<8; i++) { 
		outputs[OUT1_OUTPUT+i].value = out[i];
		lights[STATE1_LIGHT+i].value = out[i];
	}

};



struct OctaFlopWidget : ModuleWidget {
	OctaFlopWidget(OctaFlop *module);
};

OctaFlopWidget::OctaFlopWidget(OctaFlop *module) : ModuleWidget(module) {

	box.size = Vec(15*8, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/OctaFlop.svg")));

		addChild(panel);
	}

	addChild(Widget::create<MLScrew>(Vec(15, 0)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<MLScrew>(Vec(15, 365)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 365)));



	const float offset_y = 60, delta_y = 32, row1=15, row2 = 55, row3 = 80;

	for( int i=0; i<8; i++) {
		addInput(Port::create<MLPort>(Vec(row1, offset_y + i*delta_y  ), Port::INPUT, module, OctaFlop::TRIG1_INPUT+i));

		// addParam(ParamWidget::create<ML_SmallLEDButton>(Vec(row2 - 3, offset_y + 5 + i*delta_y), module, OctaFlop::TOGGLE_PARAM+i, 0.0, 10.0, 0.0));
        addChild(ModuleLightWidget::create<MLSmallLight<GreenLight>>(Vec(row2 +1, offset_y + 9 +   i*delta_y), module,  OctaFlop::STATE1_LIGHT+i));
		addOutput(Port::create<MLPort>(Vec(row3, offset_y + i*delta_y ), Port::OUTPUT, module, OctaFlop::OUT1_OUTPUT+i));
	};

    addParam(ParamWidget::create<MLButton>(Vec(row1+3, 320), module, OctaFlop::RESET_PARAM, 0.0, 10.0, 0.0));
	addInput(Port::create<MLPort>(Vec(row3, 320), Port::INPUT, module, OctaFlop::RESET_INPUT));

}

} // namespace rack_plugin_ML_modules

using namespace rack_plugin_ML_modules;

RACK_PLUGIN_MODEL_INIT(ML_modules, OctaFlop) {
   Model *modelOctaFlop = Model::create<OctaFlop, OctaFlopWidget>("ML modules", "OctaFlop", "OctaFlop", UTILITY_TAG, CLOCK_TAG);
   return modelOctaFlop;
}
