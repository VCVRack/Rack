#include "CatroModulo.hpp"

//Catro-Module CM-10: bitStep

struct CM10Module : Module {

	enum ParamIds {
        ENUMS(PARAM_REC, 2),
        ENUMS(PARAM_PLAY, 2),
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(INPUT_IN, 2),
		ENUMS(INPUT_STEP, 2),
		ENUMS(INPUT_REC, 2),
		ENUMS(INPUT_PLAY, 2),
		NUM_INPUTS
	};
	enum OutputIds {
        ENUMS(OUTPUT_OUT, 2),
        ENUMS(OUTPUT_STEP, 2),
        ENUMS(OUTPUT_CURRENT, 2),
		NUM_OUTPUTS
	};

	enum LightIds {
        NUM_LIGHTS
	};

   //initializations
	SchmittTrigger stepTrigger[2];
    SchmittTrigger recTrigger[2];
    SchmittTrigger playTrigger[2];
    bool lit[2] = {};
    bool currentin[2] = {};
    bool out[2] = {};
    bool rec[2] = {};
    bool play[2] = {};
    	
	CM10Module() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}

	void step() override;

};

void CM10Module::step() {
    for (int i = 0; i < 2; i++){
        if (stepTrigger[i].process(inputs[INPUT_STEP + i].value)){
            out[i] = currentin[i];
            if (play[i] == true){
                currentin[i] = rec[i];
                play[i] = false;
            }else{
                currentin[i] = inputs[INPUT_IN + i].value;
            }
            
            lit[i] = currentin[i];
        }
        
        if (recTrigger[i].process((inputs[INPUT_REC + i].value || params[PARAM_REC + i].value) * 10.0)){
            rec[i] = currentin[i];
        }

        if (playTrigger[i].process((inputs[INPUT_PLAY + i].value || params[PARAM_PLAY + i].value) * 10.0)){
            play[i] = true;
        }

        //set outputs
        outputs[OUTPUT_OUT + i].value = out[i] * 10.0;
        outputs[OUTPUT_STEP + i].value = (bool)(inputs[INPUT_STEP + i].value) * 10.0;
        outputs[OUTPUT_CURRENT + i].value = currentin[i] * 10.0;
    }	
}

struct CM10ModuleWidget : ModuleWidget {

	CM10ModuleWidget(CM10Module *module) : ModuleWidget(module) {
        //positionings
        float c1 = 3.2;
        float c2 = 33.2;
        float rr[6] = {50.7, 102.2, 163.1, 219.6, 271.1, 331.9}; //update positions

		setPanel(SVG::load(assetPlugin(plugin, "res/CM-10.svg")));

		//addChild(Widget::create<ScrewSilver>(Vec(30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 16, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(5, 365)));
		// addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 60, 365)));

        //Step 1
        addParam(ParamWidget::create<CM_Button_small_red>(Vec(4.7 , 130.1), module, CM10Module::PARAM_REC, 0.0, 1.0, 0.0f));
        addParam(ParamWidget::create<CM_Button_small_red>(Vec(34.7 , 130.1), module, CM10Module::PARAM_PLAY, 0.0, 1.0, 0.0f));
        addInput(Port::create<CM_Input_def>(Vec(c1, rr[0]), Port::INPUT, module, CM10Module::INPUT_IN));
        addInput(Port::create<CM_Input_def>(Vec(c1, rr[1]), Port::INPUT, module, CM10Module::INPUT_STEP));
        addInput(Port::create<CM_Input_small>(Vec(c1, rr[2]), Port::INPUT, module, CM10Module::INPUT_REC));
        addInput(Port::create<CM_Input_small>(Vec(c2, rr[2]), Port::INPUT, module, CM10Module::INPUT_PLAY));

        addOutput(Port::create<CM_Output_def>(Vec(c2 , rr[0]), Port::OUTPUT, module, CM10Module::OUTPUT_OUT));
        addOutput(Port::create<CM_Output_def>(Vec(c2 , rr[1]), Port::OUTPUT, module, CM10Module::OUTPUT_STEP));
        addOutput(Port::create<CM_Output_def>(Vec(18.2 , 72.6 ), Port::OUTPUT, module, CM10Module::OUTPUT_CURRENT));


        //LCD displays
		BigLedIndicator *display1 = new BigLedIndicator();
		display1->box.pos = Vec(5.3 , 22.1);
		display1->box.size = Vec(49.6 , 19.0);
		display1->lit = &module->lit[0];
		addChild(display1);

        //Step 2
        addParam(ParamWidget::create<CM_Button_small_red>(Vec(4.7 , 300.4), module, CM10Module::PARAM_REC + 1, 0.0, 1.0, 0.0f));
        addParam(ParamWidget::create<CM_Button_small_red>(Vec(34.7 , 300.4), module, CM10Module::PARAM_PLAY + 1, 0.0, 1.0, 0.0f));
        addInput(Port::create<CM_Input_def>(Vec(c1, rr[3]), Port::INPUT, module, CM10Module::INPUT_IN + 1));
        addInput(Port::create<CM_Input_def>(Vec(c1, rr[4]), Port::INPUT, module, CM10Module::INPUT_STEP + 1));
        addInput(Port::create<CM_Input_small>(Vec(c1, rr[5]), Port::INPUT, module, CM10Module::INPUT_REC + 1));
        addInput(Port::create<CM_Input_small>(Vec(c2, rr[5]), Port::INPUT, module, CM10Module::INPUT_PLAY + 1));

        addOutput(Port::create<CM_Output_def>(Vec(c2 , rr[3]), Port::OUTPUT, module, CM10Module::OUTPUT_OUT + 1));
        addOutput(Port::create<CM_Output_def>(Vec(c2 , rr[4]), Port::OUTPUT, module, CM10Module::OUTPUT_STEP + 1));
        addOutput(Port::create<CM_Output_def>(Vec(18.2 , 241.4 ), Port::OUTPUT, module, CM10Module::OUTPUT_CURRENT)); //update pos

        //LCD displays
		BigLedIndicator *display2 = new BigLedIndicator();
		display2->box.pos = Vec(5.3 , 190.9);
		display2->box.size = Vec(49.6 , 19.0);
		display2->lit = &module->lit[1];
		addChild(display2);

	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelCM10Module = Model::create<CM10Module, CM10ModuleWidget>("CatroModulo", "CatroModulo_CM-10", "C/M10 : bitStep", SAMPLE_AND_HOLD_TAG);

RACK_PLUGIN_MODEL_INIT(CatroModulo, CM10Module) {
   Model *model = Model::create<CM10Module, CM10ModuleWidget>("CatroModulo", "CatroModulo_CM10", "C/M10 : bitStep", SAMPLE_AND_HOLD_TAG);
   return model;
}