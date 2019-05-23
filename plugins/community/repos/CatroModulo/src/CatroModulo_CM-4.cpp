#include "CatroModulo.hpp"


//Catro-Module CM-4: vcClk

struct CM4Module : Module {

	enum ParamIds {
		PARAM_BPM,
		PARAM_RST,
		PARAM_SNAP,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_BPM1,
		INPUT_BPM2,
		INPUT_BPM3,
		INPUT_RST,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_BPM1,
		OUTPUT_BPM2,
		OUTPUT_D2,
		OUTPUT_X2,
		OUTPUT_CLKD2,
		OUTPUT_CLK,
		OUTPUT_CLKX2,
		OUTPUT_RST,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	float bpm_display = 0.0;
    //create objects
    //SchmittTrigger recordTrigger[16];
    CM_BpmClock bpmclock;
	
	CM4Module() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
			//initialize objects
	}
	void step() override;
};

void CM4Module::step() {
	if (params[PARAM_SNAP].value == 0){
		bpmclock.setbpm(int( (params[PARAM_BPM].value * 100.0) * 50) / 50.0f );
	}else if (params[PARAM_SNAP].value == 1){
		bpmclock.setbpm(int( (params[PARAM_BPM].value * 100.0) * 0.5) * 2.0f);
	}else if (params[PARAM_SNAP].value == 2){
		bpmclock.setbpm(int( (params[PARAM_BPM].value * 100.0) * 0.1) * 10.0f );
	}

	outputs[OUTPUT_RST].value = (inputs[INPUT_RST].value || params[PARAM_RST].value) * 10.0;
	bpmclock.setReset(outputs[OUTPUT_RST].value);
	
	outputs[OUTPUT_BPM1].value = bpmclock.addcv((inputs[INPUT_BPM1].active) ? inputs[INPUT_BPM1].value : 0.0f);
	outputs[OUTPUT_BPM2].value = bpmclock.addcv((inputs[INPUT_BPM2].active) ? inputs[INPUT_BPM2].value : 0.0f);

	outputs[OUTPUT_D2].value = bpmclock.getcv() * 0.5;
	outputs[OUTPUT_X2].value = bpmclock.getcv() * 2.0;
	bpm_display = bpmclock.getbpm() / 2.0f;

	bpmclock.step(engineGetSampleTime());

	outputs[OUTPUT_CLK].value = bpmclock.track(1) * 10.0;
	outputs[OUTPUT_CLKD2].value = bpmclock.track(2) * 10.0;
	outputs[OUTPUT_CLKX2].value = bpmclock.track(0) * 10.0;
}

struct CM4ModuleWidget : ModuleWidget {

	CM4ModuleWidget(CM4Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/CM-4.svg")));

		//addChild(Widget::create<ScrewSilver>(Vec(30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 16, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(5, 365)));
		// addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 60, 365)));

		//UNIQUE ELEMENTS
		addParam(ParamWidget::create<CM_Knob_huge_red_os>(Vec(3.6 , 56.0), module, CM4Module::PARAM_BPM, 0.0f, 6.0, 0.0f));
		addParam(ParamWidget::create<CM_Switch_small_3>(Vec(7.0, 43.0), module, CM4Module::PARAM_SNAP, 0.0f, 2.0f, 1.0f));

		addInput(Port::create<CM_Input_bpm>(Vec(7.0 , 126.3), Port::INPUT, module, CM4Module::INPUT_BPM1));
		addInput(Port::create<CM_Input_bpm>(Vec(7.0 , 169.1), Port::INPUT, module, CM4Module::INPUT_BPM2));

		addOutput(Port::create<CM_Output_bpm>(Vec(44.4 , 126.3), Port::OUTPUT, module, CM4Module::OUTPUT_BPM1));
		addOutput(Port::create<CM_Output_bpm>(Vec(44.4 , 169.1), Port::OUTPUT, module, CM4Module::OUTPUT_BPM2));

		addOutput(Port::create<CM_Output_bpm>(Vec(7.0 , 212.0), Port::OUTPUT, module, CM4Module::OUTPUT_D2));
		addOutput(Port::create<CM_Output_bpm>(Vec(44.4 , 212.0), Port::OUTPUT, module, CM4Module::OUTPUT_X2));

		addOutput(Port::create<CM_Output_def>(Vec(26.1 , 293.9), Port::OUTPUT, module, CM4Module::OUTPUT_CLK));
		addOutput(Port::create<CM_Output_def>(Vec(3.5 , 326.5), Port::OUTPUT, module, CM4Module::OUTPUT_CLKD2));
		addOutput(Port::create<CM_Output_def>(Vec(48.1 , 326.5), Port::OUTPUT, module, CM4Module::OUTPUT_CLKX2));

		addInput(Port::create<CM_Input_small>(Vec(6.2 , 251.8), Port::INPUT, module, CM4Module::INPUT_RST));
		addParam(ParamWidget::create<CM_Button_small_red>(Vec(29.4 , 251.8), module, CM4Module::PARAM_RST, 0.0f, 1.0f, 0.0f));
		addOutput(Port::create<CM_Output_small>(Vec(52.4 , 251.8), Port::OUTPUT, module, CM4Module::OUTPUT_RST));

		//LCD display
		NumDisplayWidget *display = new NumDisplayWidget();
		display->box.pos = Vec(7.0 , 21.0);
		display->box.size = Vec(61.1 , 20.4);
		display->value = &module->bpm_display;
		addChild(display);
		
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
// Model *modelCM4Module = Model::create<CM4Module, CM4ModuleWidget>("CatroModulo", "CatroModulo_CM-4", "C/M4 : vcClk", CLOCK_TAG);

RACK_PLUGIN_MODEL_INIT(CatroModulo, CM4Module) {
   Model *model = Model::create<CM4Module, CM4ModuleWidget>("CatroModulo", "CatroModulo_CM4", "C/M4 : vcClk", CLOCK_TAG);
   return model;
}