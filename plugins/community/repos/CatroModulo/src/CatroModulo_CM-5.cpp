#include "CatroModulo.hpp"


//Catro-Module CM-5: vcClk++


struct CM5Module : Module {
	enum ParamIds {
		PARAM_RST,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_BPM,
		INPUT_BPM2,
		INPUT_RST,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_CLKX1,
		OUTPUT_CLKX2,
		OUTPUT_CLKX3,
		OUTPUT_CLKX4,
		OUTPUT_CLKX5,
		OUTPUT_CLKX7,
		OUTPUT_CLKX9,
		OUTPUT_BPMX1,
		OUTPUT_BPMX2,
		OUTPUT_BPMX3,
		OUTPUT_BPMX4,
		OUTPUT_BPMX5,
		OUTPUT_BPMX7,
		OUTPUT_BPMX9,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(LIGHTS_CLK, 14), 
		NUM_LIGHTS
	};

    //create objects
    //SchmittTrigger recordTrigger[16];
    CM_BpmClock clock1;
	CM_BpmClock clock2;
	CM_BpmClock clock3;
	CM_BpmClock clock4;
	CM_BpmClock clock5;
	
	CM5Module() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
			//initialize objects
	}
	void step() override;
};

void CM5Module::step() {

	//set from first bpm cv input
	clock1.setcv(inputs[INPUT_BPM].value * 2.0);
	clock2.setcv(inputs[INPUT_BPM].value * 3.0);
	clock3.setcv(inputs[INPUT_BPM].value * 5.0);
	clock4.setcv(inputs[INPUT_BPM].value * 7.0);
	clock5.setcv(inputs[INPUT_BPM].value * 9.0);

	//generate bpm outputs before adding second cv
	outputs[OUTPUT_BPMX1].value = clock1.getcv() * 0.5;
	outputs[OUTPUT_BPMX2].value = clock1.getcv();
	outputs[OUTPUT_BPMX3].value = clock2.getcv();
	outputs[OUTPUT_BPMX4].value = clock1.getcv() * 2.0;
	outputs[OUTPUT_BPMX5].value = clock3.getcv();
	outputs[OUTPUT_BPMX7].value = clock4.getcv();
	outputs[OUTPUT_BPMX9].value = clock5.getcv();

	//add second bpm cv input
	clock1.addcv(inputs[INPUT_BPM2].value * 2.0);
	clock2.addcv(inputs[INPUT_BPM2].value * 3.0);
	clock3.addcv(inputs[INPUT_BPM2].value * 5.0);
	clock4.addcv(inputs[INPUT_BPM2].value * 7.0);
	clock5.addcv(inputs[INPUT_BPM2].value * 9.0);

	//perform reset if needed
	clock1.setReset(inputs[INPUT_RST].value || params[PARAM_RST].value);
	clock2.setReset(inputs[INPUT_RST].value || params[PARAM_RST].value);
	clock3.setReset(inputs[INPUT_RST].value || params[PARAM_RST].value);
	clock4.setReset(inputs[INPUT_RST].value || params[PARAM_RST].value);
	clock5.setReset(inputs[INPUT_RST].value || params[PARAM_RST].value);

	//step clocks
	clock1.step(engineGetSampleTime());
	clock2.step(engineGetSampleTime());
	clock3.step(engineGetSampleTime());
	clock4.step(engineGetSampleTime());
	clock5.step(engineGetSampleTime());

	//set clock outputs
	outputs[OUTPUT_CLKX1].value = clock1.track(2) * 10.0;
	outputs[OUTPUT_CLKX2].value = clock1.track(1) * 10.0;
	outputs[OUTPUT_CLKX3].value = clock2.track(1) * 10.0;
	outputs[OUTPUT_CLKX4].value = clock1.track(0) * 10.0;
	outputs[OUTPUT_CLKX5].value = clock3.track(1) * 10.0;
	outputs[OUTPUT_CLKX7].value = clock4.track(1) * 10.0;
	outputs[OUTPUT_CLKX9].value = clock5.track(1) * 10.0;

	//set clock lights
	for (int i = 0; i < 7; i++){
		lights[2 * i + 0].setBrightnessSmooth(fmaxf(0.0f, outputs[OUTPUT_CLKX1 + i].value * 0.2f));
		lights[2 * i + 1].setBrightnessSmooth(fmaxf(0.0f, -outputs[OUTPUT_CLKX1 + i].value * 0.2f));
	}



}

struct CM5ModuleWidget : ModuleWidget {
	CM5ModuleWidget(CM5Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/CM-5.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(2, 365)));
		
		//UNIQUE ELEMENTS
		addParam(ParamWidget::create<CM_I_def_tinybuttonL>(Vec(8.7 , 330.3), module, CM5Module::PARAM_RST, 0.0f, 1.0f, 0.0f));
		addInput(Port::create<CM_Input_def>(Vec(17.3, 330.3), Port::INPUT, module, CM5Module::INPUT_RST));

		addInput(Port::create<CM_Input_bpm>(Vec(0.0 , 25.2), Port::INPUT, module, CM5Module::INPUT_BPM));
		addInput(Port::create<CM_Input_bpm>(Vec(20.7 , 37.7), Port::INPUT, module, CM5Module::INPUT_BPM2));

		//clock outputs
		addOutput(Port::create<CM_Output_def>(Vec(20.7 , 74.6 ), Port::OUTPUT, module, CM5Module::OUTPUT_CLKX1));
		addOutput(Port::create<CM_Output_def>(Vec(20.7 , 111.5), Port::OUTPUT, module, CM5Module::OUTPUT_CLKX2));
		addOutput(Port::create<CM_Output_def>(Vec(20.7 , 148.3), Port::OUTPUT, module, CM5Module::OUTPUT_CLKX3));
		addOutput(Port::create<CM_Output_def>(Vec(20.7 , 185.2), Port::OUTPUT, module, CM5Module::OUTPUT_CLKX4));
		addOutput(Port::create<CM_Output_def>(Vec(20.7 , 222.0), Port::OUTPUT, module, CM5Module::OUTPUT_CLKX5));
		addOutput(Port::create<CM_Output_def>(Vec(20.7 , 258.9), Port::OUTPUT, module, CM5Module::OUTPUT_CLKX7));
		addOutput(Port::create<CM_Output_def>(Vec(20.7 , 295.8), Port::OUTPUT, module, CM5Module::OUTPUT_CLKX9));

		//bpm cv outputs
		addOutput(Port::create<CM_Output_bpm>(Vec(0 , 61.3) , Port::OUTPUT, module, CM5Module::OUTPUT_BPMX1));
		addOutput(Port::create<CM_Output_bpm>(Vec(0 , 98.2) , Port::OUTPUT, module, CM5Module::OUTPUT_BPMX2));
		addOutput(Port::create<CM_Output_bpm>(Vec(0 , 135.0), Port::OUTPUT, module, CM5Module::OUTPUT_BPMX3));
		addOutput(Port::create<CM_Output_bpm>(Vec(0 , 171.9), Port::OUTPUT, module, CM5Module::OUTPUT_BPMX4));
		addOutput(Port::create<CM_Output_bpm>(Vec(0 , 208.7), Port::OUTPUT, module, CM5Module::OUTPUT_BPMX5));
		addOutput(Port::create<CM_Output_bpm>(Vec(0 , 245.6), Port::OUTPUT, module, CM5Module::OUTPUT_BPMX7));
		addOutput(Port::create<CM_Output_bpm>(Vec(0 , 282.5), Port::OUTPUT, module, CM5Module::OUTPUT_BPMX9));

		//clock lights
		addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(41.0 , 74.6  + 21.0), module, CM5Module::LIGHTS_CLK));
		addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(41.0 , 111.5 + 21.0), module, CM5Module::LIGHTS_CLK + 2));
		addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(41.0 , 148.3 + 21.0), module, CM5Module::LIGHTS_CLK + 4));
		addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(41.0 , 185.2 + 21.0), module, CM5Module::LIGHTS_CLK + 6));
		addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(41.0 , 222.0 + 21.0), module, CM5Module::LIGHTS_CLK + 8));
		addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(41.0 , 258.9 + 21.0), module, CM5Module::LIGHTS_CLK + 10));
		addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(41.0 , 295.8 + 21.0), module, CM5Module::LIGHTS_CLK + 12));
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
//Model *modelCM5Module = Model::create<CM5Module, CM5ModuleWidget>("CatroModulo", "CatroModulo_CM-5", "C/M5 : vcClk++", CLOCK_TAG);

RACK_PLUGIN_MODEL_INIT(CatroModulo, CM5Module) {
   Model *model = Model::create<CM5Module, CM5ModuleWidget>("CatroModulo", "CatroModulo_CM5", "C/M5 : vcClk++", CLOCK_TAG);
   return model;
}