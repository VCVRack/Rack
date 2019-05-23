#include "CatroModulo.hpp"
#include "CM_helpers.hpp"

//Catro-Module CM-9: 1-8-1

struct CM9Module : Module {

	enum ParamIds {
		PARAM_SEL,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_SEL,
		INPUT_CLK,
		INPUT_RST,
		INPUT_1,
		ENUMS(INPUT_IN, 8), 
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(OUTPUT_OUT, 8),
        OUTPUT_1,
		NUM_OUTPUTS
	};

	enum LightIds {
        		NUM_LIGHTS
	};

	enum Modes {
		MODE_SPLIT,
		MODE_JOIN,
		MODE_GATE,
		MODE_PASS,
	};

   //initializations
	int mode;
    SchmittTrigger clkTrigger;
	SchmittTrigger rstTrigger;
	CM_stepper stepper;
	int selector;
	float ledx = 30.9;
	float ledy = 50.0;
	float ins[8];
	float outs[8];
	bool gatemode = 0;
	
	CM9Module() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}

	void step() override;

};

void CM9Module::step() {
	//process inputs
	int inputsconnected = 0;
	for (int i = 0; i < 8; i++){
		if (inputs[INPUT_IN + i].active){
			inputsconnected++;
			ins[i] = inputs[INPUT_IN + i].value;
		}else{
			ins[i] = 10.0;
		}
	}

	//process selector
	float selectorparam = clamp(round((inputs[INPUT_SEL].active) ? inputs[INPUT_SEL].value * 0.1 * params[PARAM_SEL].value : params[PARAM_SEL].value), 0, 7);

	//stepper
	if (inputs[INPUT_CLK].active){
		if (inputs[INPUT_RST].active){
			if (rstTrigger.process(inputs[INPUT_RST].value)){
				stepper.reset();
			}
		}
		if (clkTrigger.process(inputs[INPUT_CLK].value)){
			selector = stepper.step(selectorparam);
		}
	}else{
		selector = selectorparam;
	}

	//process outputs
	//reset all to 0
	for (int i = 0; i < NUM_OUTPUTS; i++){
			outputs[OUTPUT_OUT + i].value = 0.0;
		}
	gatemode = true;

	if (inputs[INPUT_1].active){
		outputs[OUTPUT_OUT + selector].value = inputs[INPUT_1].value;
		gatemode = false;
	}
	if (outputs[OUTPUT_1].active){
		if (inputsconnected > 0){
			outputs[OUTPUT_1].value = inputs[INPUT_IN + selector].value;
		}else{
			outputs[OUTPUT_1].value = selector * 1.4285714285714285714285714285714f;
		}
		
		
	}
	if (gatemode == true){
		if (inputsconnected > 0){
			outputs[OUTPUT_OUT + selector].value = inputs[INPUT_IN + selector].value;
		}else{
			if (inputs[INPUT_CLK].active){
				outputs[OUTPUT_OUT + selector].value = (inputs[INPUT_CLK].value > 0) ? 10.0f : 0.0f;
			}else{
				outputs[OUTPUT_OUT + selector].value = 10.0f;
			}
		}
	}

	//indicator leds
	ledy = 114.1 + 27.7 * selector;

}

struct CM9ModuleWidget : ModuleWidget {

	CM9ModuleWidget(CM9Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/CM-9.svg")));

		//addChild(Widget::create<ScrewSilver>(Vec(30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 16, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(5, 365)));
		// addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 60, 365)));

		//widget items
        addParam(ParamWidget::create<CM_Knob_big_def_tt>(Vec(7.0 , 20.2), module, CM9Module::PARAM_SEL, 0.0, 7.0, 0.0f));

		addInput(Port::create<CM_Input_small>(Vec(2.8, 65.9), Port::INPUT, module, CM9Module::INPUT_SEL));
        addInput(Port::create<CM_Input_small>(Vec(50.2 , 30.0), Port::INPUT, module, CM9Module::INPUT_CLK));
        addInput(Port::create<CM_Input_small>(Vec(50.2 , 60.2), Port::INPUT, module, CM9Module::INPUT_RST));
        

		float a = 5.1;
		float b = 46.4;
		float c[8] = {107.5, 135.2, 163.0, 190.7, 218.5, 246.3, 274.0, 301.8};

		addInput(Port::create<CM_Input_def>(Vec(25.7, 77.5), Port::INPUT, module, CM9Module::INPUT_1));

		for (int i = 0; i < 8; i++){
        addInput(Port::create<CM_Input_def>(Vec(a, c[i]), Port::INPUT, module, CM9Module::INPUT_IN + i));
		}

		for (int i = 0; i < 8; i++){
        addOutput(Port::create<CM_Output_def>(Vec(b , c[i] - 6.1), Port::OUTPUT, module, CM9Module::OUTPUT_OUT + i));
		}

        addOutput(Port::create<CM_Output_def>(Vec(25.7 , 326.6), Port::OUTPUT, module, CM9Module::OUTPUT_1));


		//led selector display
		CM9_LedIndicator *ledindicator = new CM9_LedIndicator();
		ledindicator->posx = &module->ledx;
		ledindicator->posy = &module->ledy;
		addChild(ledindicator);

	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
// Model *modelCM9Module = Model::create<CM9Module, CM9ModuleWidget>("CatroModulo", "CatroModulo_CM-9", "C/M9 : 1-8-1", SWITCH_TAG);

RACK_PLUGIN_MODEL_INIT(CatroModulo, CM9Module) {
   Model *model = Model::create<CM9Module, CM9ModuleWidget>("CatroModulo", "CatroModulo_CM9", "C/M9 : 1-8-1", SWITCH_TAG);
   return model;
}