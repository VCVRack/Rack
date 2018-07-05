#include "ML_modules.hpp"

namespace rack_plugin_ML_modules {

struct Sum8 : Module {
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
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};


	Sum8() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) {};


	void step() override;

};



void Sum8::step() {

	float out=0.0;



	for(int i=0; i<8; i++) out += inputs[IN1_INPUT+i].normalize(0.0);

	outputs[OUT_OUTPUT].value = out;

};



struct Sum8Widget : ModuleWidget {
	Sum8Widget(Sum8 *module);
};

Sum8Widget::Sum8Widget(Sum8 *module) : ModuleWidget(module) {

	box.size = Vec(15*3, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/Sum8.svg")));

		addChild(panel);
	}

	addChild(Widget::create<MLScrew>(Vec(15, 0)));
	addChild(Widget::create<MLScrew>(Vec(15, 365)));




	const float offset_y = 70, delta_y = 26.5, offset_x=9.5;

	for( int i=0; i<8; i++) addInput(Port::create<MLPort>(Vec(offset_x, offset_y + i*delta_y  ), Port::INPUT, module, Sum8::IN1_INPUT+i));


	addOutput(Port::create<MLPort>(Vec(offset_x, 320), Port::OUTPUT, module, Sum8::OUT_OUTPUT));


}

} // namespace rack_plugin_ML_modules

using namespace rack_plugin_ML_modules;

RACK_PLUGIN_MODEL_INIT(ML_modules, Sum8) {
   Model *modelSum8 = Model::create<Sum8, Sum8Widget>("ML modules", "Sum8", "Sum8", UTILITY_TAG, MIXER_TAG);
   return modelSum8;
}
