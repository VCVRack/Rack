#include "ML_modules.hpp"

namespace rack_plugin_ML_modules {

struct Sum8mk2 : Module {
	enum ParamIds {
		POLARITY_PARAM,
		NUM_PARAMS = POLARITY_PARAM + 8
	};
	enum InputIds {
		IN_INPUT,
		NUM_INPUTS = IN_INPUT + 8
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};


	Sum8mk2() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) {};


	void step() override;

};



void Sum8mk2::step() {

	float out=0.0;


	for(int i=0; i<8; i++) out += inputs[IN_INPUT+i].normalize(0.0) * (2*params[POLARITY_PARAM+i].value - 1.0);

	outputs[OUT_OUTPUT].value = out;

};



struct Sum8mk2Widget : ModuleWidget {
	Sum8mk2Widget(Sum8mk2 *module);
};

Sum8mk2Widget::Sum8mk2Widget(Sum8mk2 *module) : ModuleWidget(module) {

	box.size = Vec(15*5, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/Sum8mk2.svg")));

		addChild(panel);
	}

	addChild(Widget::create<MLScrew>(Vec(15, 0)));
	addChild(Widget::create<MLScrew>(Vec(15, 365)));




	const float offset_y = 70, delta_y = 26.5, offset_x=9.5;

	for( int i=0; i<8; i++) {
		addInput(Port::create<MLPort>(Vec(offset_x, offset_y + i*delta_y  ), Port::INPUT, module, Sum8mk2::IN_INPUT+i));
        addParam(ParamWidget::create<POLSWITCH>( Vec(offset_x + 37, offset_y + i*delta_y + 2 ), module, Sum8mk2::POLARITY_PARAM + i, 0.0, 1.0, 1.0));
	}


	addOutput(Port::create<MLPort>(Vec(offset_x, 320), Port::OUTPUT, module, Sum8mk2::OUT_OUTPUT));


}

} // namespace rack_plugin_ML_modules

using namespace rack_plugin_ML_modules;

RACK_PLUGIN_MODEL_INIT(ML_modules, Sum8mk2) {
   Model *modelSum8mk2 = Model::create<Sum8mk2, Sum8mk2Widget>("ML modules", "Sum8mk2", "Sum8 MkII", UTILITY_TAG, MIXER_TAG);
   return modelSum8mk2;
}
