//**************************************************************************************
//VCA module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//Code taken from the Fundamentals plugins by Andrew Belt http://www.vcvrack.com
//**************************************************************************************

#include "AS.hpp"

namespace rack_plugin_AS {

struct VCA : Module {
	enum ParamIds {
		LEVEL1_PARAM,
		LEVEL2_PARAM,
        MODE1_PARAM,
        MODE2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ENV1_INPUT,
		IN1_INPUT,
		ENV2_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};

	float v1= 0.0f;
	float v2= 0.0f;
	const float expBase = 50.0f;

	VCA() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;
};

void VCA::step() {
	//VCA 1
	v1 = inputs[IN1_INPUT].value * params[LEVEL1_PARAM].value;
	if(inputs[ENV1_INPUT].active){
		if(params[MODE1_PARAM].value==1){
			v1 *= clamp(inputs[ENV1_INPUT].value / 10.0f, 0.0f, 1.0f);
		}else{
			v1 *= rescale(powf(expBase, clamp(inputs[ENV1_INPUT].value / 10.0f, 0.0f, 1.0f)), 1.0f, expBase, 0.0f, 1.0f);
		}
	}
	outputs[OUT1_OUTPUT].value = v1;
	//VCA 2
	v2 = inputs[IN2_INPUT].value * params[LEVEL2_PARAM].value;
	if(inputs[ENV2_INPUT].active){
		if(params[MODE2_PARAM].value){
			v2 *= clamp(inputs[ENV2_INPUT].value / 10.0f, 0.0f, 1.0f);
		}else{
			v2 *= rescale(powf(expBase, clamp(inputs[ENV2_INPUT].value / 10.0f, 0.0f, 1.0f)), 1.0f, expBase, 0.0f, 1.0f);
		}
	}
	outputs[OUT2_OUTPUT].value = v2;
}


struct VCAWidget : ModuleWidget 
{ 
    VCAWidget(VCA *module);
};

VCAWidget::VCAWidget(VCA *module) : ModuleWidget(module) {

   setPanel(SVG::load(assetPlugin(plugin, "res/VCA.svg")));
   
	//SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    //SLIDERS
	addParam(ParamWidget::create<as_SlidePot>(Vec(10, 70), module, VCA::LEVEL1_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<as_SlidePot>(Vec(55, 70), module, VCA::LEVEL2_PARAM, 0.0f, 1.0f, 0.5f));
    //MODE SWITCHES
    addParam(ParamWidget::create<as_CKSS>(Vec(14, 190), module, VCA::MODE1_PARAM, 0.0f, 1.0f, 1.0f));
	addParam(ParamWidget::create<as_CKSS>(Vec(59, 190), module, VCA::MODE2_PARAM, 0.0f, 1.0f, 1.0f));
	//PORTS
	addInput(Port::create<as_PJ301MPort>(Vec(10, 217), Port::INPUT, module, VCA::ENV1_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(55, 217), Port::INPUT, module, VCA::ENV2_INPUT));

	addInput(Port::create<as_PJ301MPort>(Vec(10, 260), Port::INPUT, module, VCA::IN1_INPUT));
    addInput(Port::create<as_PJ301MPort>(Vec(55, 260), Port::INPUT, module, VCA::IN2_INPUT));
	
	addOutput(Port::create<as_PJ301MPort>(Vec(10, 310), Port::OUTPUT, module, VCA::OUT1_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(55, 310), Port::OUTPUT, module, VCA::OUT2_OUTPUT));

}

} // namespace rack_plugin_AS

using namespace rack_plugin_AS;

RACK_PLUGIN_MODEL_INIT(AS, VCA) {
   Model *modelVCA = Model::create<VCA, VCAWidget>("AS", "VCA", "VCA", AMPLIFIER_TAG);
   return modelVCA;
}
