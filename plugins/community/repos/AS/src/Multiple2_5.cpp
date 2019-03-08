//**************************************************************************************
//Multiple 2x5 module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//**************************************************************************************
#include "AS.hpp"

struct Multiple2_5 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		INPUT1,
		INPUT2,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT11,
		OUT12,
		OUT13,
		OUT14,
		OUT15,
		OUT21,
		OUT22,
		OUT23,
		OUT24,
		OUT25,
		NUM_OUTPUTS
	};

	Multiple2_5() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;
};


void Multiple2_5::step() {

	float IN1 = inputs[INPUT1].value;
	float IN2 = inputs[INPUT2].value;
	
	// Set outputs
	outputs[OUT11].value = IN1;
	outputs[OUT12].value = IN1;
	outputs[OUT13].value = IN1;
	outputs[OUT14].value = IN1;
	outputs[OUT15].value = IN1;
	/* Update suggested by MarcBrule
	If the input 2 is not active the second row of outputs takes the value of input 1
	so the module becomes a multi 1 x 10, nice and simple idea!
	*/
	if(!inputs[INPUT2].active){
		outputs[OUT21].value = IN1;
		outputs[OUT22].value = IN1;
		outputs[OUT23].value = IN1;
		outputs[OUT24].value = IN1;
		outputs[OUT25].value = IN1;

	}else{
		outputs[OUT21].value = IN2;
		outputs[OUT22].value = IN2;
		outputs[OUT23].value = IN2;
		outputs[OUT24].value = IN2;
		outputs[OUT25].value = IN2;
	}
	/*
	outputs[OUT21].value = IN2;
	outputs[OUT22].value = IN2;
	outputs[OUT23].value = IN2;
	outputs[OUT24].value = IN2;
	outputs[OUT25].value = IN2;
	*/
	//Is it necessary to check for active outputs in this case?
	/*
	if (outputs[OUT11].active) {
		outputs[OUT11].value = IN1;
	}
	
	if (outputs[OUT12].active) {
		outputs[OUT12].value = IN1;
	}

	if (outputs[OUT13].active) {
		outputs[OUT13].value= IN1;
	}

	if (outputs[OUT14].active) {
		outputs[OUT14].value = IN1;
	}
	if (outputs[OUT15].active) {
		outputs[OUT15].value = IN1;
	}

	if (outputs[OUT21].active) {
		outputs[OUT21].value = IN2;
	}

	if (outputs[OUT22].active) {
		outputs[OUT22].value = IN2;
	}

	if (outputs[OUT23].active) {
		outputs[OUT23].value = IN2;
	}

	if (outputs[OUT24].active) {
		outputs[OUT24].value = IN2;
	}
	if (outputs[OUT25].active) {
		outputs[OUT25].value = IN2;
	}
	*/
}

struct Multiple2_5Widget : ModuleWidget 
{ 
    Multiple2_5Widget(Multiple2_5 *module);
};


Multiple2_5Widget::Multiple2_5Widget(Multiple2_5 *module) : ModuleWidget(module) {

  setPanel(SVG::load(assetPlugin(plugin, "res/Multiple2_5.svg")));
	//SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	//INPUTS
	addInput(Port::create<as_PJ301MPort>(Vec(8, 65), Port::INPUT, module, Multiple2_5::INPUT1));
	addInput(Port::create<as_PJ301MPort>(Vec(43, 65), Port::INPUT, module, Multiple2_5::INPUT2));
	//OUTPUTS
	addOutput(Port::create<as_PJ301MPort>(Vec(8, 130), Port::OUTPUT, module, Multiple2_5::OUT11));
	addOutput(Port::create<as_PJ301MPort>(Vec(43, 130), Port::OUTPUT, module, Multiple2_5::OUT21));
	
	addOutput(Port::create<as_PJ301MPort>(Vec(8, 175), Port::OUTPUT, module, Multiple2_5::OUT12));
	addOutput(Port::create<as_PJ301MPort>(Vec(43, 175), Port::OUTPUT, module, Multiple2_5::OUT22));

	addOutput(Port::create<as_PJ301MPort>(Vec(8, 220), Port::OUTPUT, module, Multiple2_5::OUT13));
	addOutput(Port::create<as_PJ301MPort>(Vec(43, 220), Port::OUTPUT, module, Multiple2_5::OUT23));

	addOutput(Port::create<as_PJ301MPort>(Vec(8, 265), Port::OUTPUT, module, Multiple2_5::OUT14));
	addOutput(Port::create<as_PJ301MPort>(Vec(43, 265), Port::OUTPUT, module, Multiple2_5::OUT24));

	addOutput(Port::create<as_PJ301MPort>(Vec(8, 310), Port::OUTPUT, module, Multiple2_5::OUT15));
	addOutput(Port::create<as_PJ301MPort>(Vec(43, 310), Port::OUTPUT, module, Multiple2_5::OUT25));

}

RACK_PLUGIN_MODEL_INIT(AS, Multiple2_5) {
   Model *modelMultiple2_5 = Model::create<Multiple2_5, Multiple2_5Widget>("AS", "Multiple2_5", "Multiple 2 x 5", MULTIPLE_TAG, UTILITY_TAG);
   return modelMultiple2_5;
}
