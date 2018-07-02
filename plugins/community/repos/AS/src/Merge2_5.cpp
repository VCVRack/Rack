//**************************************************************************************
//Multiple 2x5 module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//**************************************************************************************
#include "AS.hpp"

struct Merge2_5 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		INPUT11,
		INPUT12,
		INPUT13,
		INPUT14,
		INPUT15,
		INPUT21,
		INPUT22,
		INPUT23,
		INPUT24,
		INPUT25,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT1,
		OUTPUT2,
		NUM_OUTPUTS
	};

	Merge2_5() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;
};


void Merge2_5::step() {

	float IN11 = inputs[INPUT11].value;
	float IN12 = inputs[INPUT12].value;
	float IN13 = inputs[INPUT13].value;
	float IN14 = inputs[INPUT14].value;
	float IN15 = inputs[INPUT15].value;
	float IN21 = inputs[INPUT21].value;
	float IN22 = inputs[INPUT22].value;
	float IN23 = inputs[INPUT23].value;
	float IN24 = inputs[INPUT24].value;
	float IN25 = inputs[INPUT25].value;

	
	// Set outputs
	outputs[OUTPUT1].value = IN11 + IN12 + IN13 + IN14 + IN15;
	outputs[OUTPUT2].value = IN21 + IN22 + IN23 + IN24 + IN25;

}

struct Merge2_5Widget : ModuleWidget 
{ 
    Merge2_5Widget(Merge2_5 *module);
};


Merge2_5Widget::Merge2_5Widget(Merge2_5 *module) : ModuleWidget(module) {

  setPanel(SVG::load(assetPlugin(plugin, "res/Merge2_5.svg"))); 
   
	//SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	//INPUTS 
	addInput(Port::create<as_PJ301MPort>(Vec(8, 75), Port::INPUT, module, Merge2_5::INPUT11));
	addInput(Port::create<as_PJ301MPort>(Vec(43, 75), Port::INPUT, module, Merge2_5::INPUT21));
	
	addInput(Port::create<as_PJ301MPort>(Vec(8, 120), Port::INPUT, module, Merge2_5::INPUT12));
	addInput(Port::create<as_PJ301MPort>(Vec(43, 120), Port::INPUT, module, Merge2_5::INPUT22));

	addInput(Port::create<as_PJ301MPort>(Vec(8, 165), Port::INPUT, module, Merge2_5::INPUT13));
	addInput(Port::create<as_PJ301MPort>(Vec(43, 165), Port::INPUT, module, Merge2_5::INPUT23));

	addInput(Port::create<as_PJ301MPort>(Vec(8, 210), Port::INPUT, module, Merge2_5::INPUT14));
	addInput(Port::create<as_PJ301MPort>(Vec(43, 210), Port::INPUT, module, Merge2_5::INPUT24));

	addInput(Port::create<as_PJ301MPort>(Vec(8, 255), Port::INPUT, module, Merge2_5::INPUT15));
	addInput(Port::create<as_PJ301MPort>(Vec(43, 255), Port::INPUT, module, Merge2_5::INPUT25));

	//OUTPUTS
	addOutput(Port::create<as_PJ301MPort>(Vec(8, 310), Port::OUTPUT, module, Merge2_5::OUTPUT1));
	addOutput(Port::create<as_PJ301MPort>(Vec(43, 310), Port::OUTPUT, module, Merge2_5::OUTPUT2));

}

RACK_PLUGIN_MODEL_INIT(AS, Merge2_5) {
   Model *modelMerge2_5 = Model::create<Merge2_5, Merge2_5Widget>("AS", "Merge2_5", "Merge 2 x 5", MULTIPLE_TAG, UTILITY_TAG);
   return modelMerge2_5;
}
