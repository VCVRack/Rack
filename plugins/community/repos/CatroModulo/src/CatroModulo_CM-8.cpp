#include "CatroModulo.hpp"
#include "CM_helpers.hpp"

//Catro-Module CM-8: aAvsBb

struct CM8Module : Module {

	enum ParamIds {
		PARAM__a,
		PARAM__b,
        PARAM_CIA,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT__a,
		INPUT__b,
		INPUT_A,
		INPUT_B,
		INPUT_SNH,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT__a,
		OUTPUT__b,
		OUTPUT_ALTB,
		OUTPUT_BLTA,
		OUTPUT_AISB,
		OUTPUT_ANTB,
		OUTPUT_ACLM,
		OUTPUT_BCLM,
		OUTPUT_AFLD,
		OUTPUT_BFLD,
        OUTPUT_ALO,
        OUTPUT_BLO,
        OUTPUT_AHI,
        OUTPUT_BHI,
        OUTPUT_ARNG,
        OUTPUT_BRNG,
		NUM_OUTPUTS
	};
	enum LightIds {
        LIGHT_ALTB,
		LIGHT_BLTA,
		LIGHT_ACLM,
		LIGHT_BCLM,
		LIGHT_AFLD,
		LIGHT_BFLD,
        LIGHT_ALO,
        LIGHT_BLO,
        LIGHT_AHI,
        LIGHT_BHI,
        LIGHT_ARNG,
        LIGHT_BRNG,
		NUM_LIGHTS
	};

   //initializations
   float lo;
   float hi;
   int cia;
   SchmittTrigger snhTrigger;
   float lastA;
   float lastB;
   float currentA;
   float currentB;
	
	CM8Module() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
			srand(time(NULL));
	}
	void step() override;
};

void CM8Module::step() {
	// set mode
	cia = params[PARAM_CIA].value;


	//set limits
		
		if (cia == 0){
			lo = (inputs[INPUT__a].active) ? (inputs[INPUT__a].value * 0.1) * params[PARAM__a].value : params[PARAM__a].value;
			hi = (inputs[INPUT__b].active) ? (inputs[INPUT__b].value * 0.1) * params[PARAM__b].value : params[PARAM__b].value;
			if (lo > hi){
				hi = (lo + hi) * 0.5;
				lo = hi;
			}
			outputs[OUTPUT__a].value = lo;
			outputs[OUTPUT__b].value = hi;
		}
		if (cia == 1){
			lo = (inputs[INPUT__a].active) ? (inputs[INPUT__a].value * 0.1) * params[PARAM__a].value : params[PARAM__a].value;
			hi = (inputs[INPUT__b].active) ? (inputs[INPUT__b].value * 0.1) * params[PARAM__b].value : params[PARAM__b].value;
			outputs[OUTPUT__a].value = lo;
			outputs[OUTPUT__b].value = hi;
			if (lo > hi){
			std::swap(lo, hi);
			}
		}
		if (cia == 2){
			lo = (inputs[INPUT__a].active) ? (inputs[INPUT__a].value * 0.1) * params[PARAM__a].value : params[PARAM__a].value;
			hi = lo + ((inputs[INPUT__b].active) ? (inputs[INPUT__b].value * 0.1) * params[PARAM__b].value : params[PARAM__b].value);
			outputs[OUTPUT__a].value = lo;
			outputs[OUTPUT__b].value = hi;
			if (lo > hi){
			std::swap(lo, hi);
			}
		}

	currentA = inputs[INPUT_A].value;
	currentB = inputs[INPUT_B].value;

	//handle empty inputs
	if (! inputs[INPUT_A].active) currentA = (inputs[INPUT_B].active) ? cm_gauss(5.0, currentB) : cm_gauss(10.0);
	if (! inputs[INPUT_B].active) currentB = (inputs[INPUT_A].active) ? cm_gauss(5.0, currentA) : cm_gauss(10.0);

	//sample and hold
	if (inputs[INPUT_SNH].active){
		if (snhTrigger.process(inputs[INPUT_SNH].value)){
			lastA = currentA;
			lastB = currentB;
		}
		currentA = lastA;
		currentB = lastB;
	}
	

	//A
	outputs[OUTPUT_ALTB].value = (currentA > currentB) * 10.0;
	outputs[OUTPUT_AISB].value = (currentA == currentB) * 10.0;
	outputs[OUTPUT_ACLM].value = cm_clamp(currentA, lo, hi);
	outputs[OUTPUT_AFLD].value = cm_fold(currentA, lo, hi);
	outputs[OUTPUT_ALO].value = (currentA <= hi) * 10.0;
	outputs[OUTPUT_AHI].value = (currentA >= lo) * 10.0;
	outputs[OUTPUT_ARNG].value = (currentA > lo && currentA < hi) * 10.0;

	//B
	outputs[OUTPUT_BLTA].value = (currentA < currentB) * 10.0;
	outputs[OUTPUT_ANTB].value = !(currentA == currentB) * 10.0;
	outputs[OUTPUT_BCLM].value = cm_clamp(currentB, lo, hi);
	outputs[OUTPUT_BFLD].value = cm_fold(currentB, lo, hi);
	outputs[OUTPUT_BLO].value = (currentB <= hi) * 10.0;
	outputs[OUTPUT_BHI].value = (currentB >= lo) * 10.0;
	outputs[OUTPUT_BRNG].value = (currentB > lo && currentB < hi) * 10.0;


}

struct CM8ModuleWidget : ModuleWidget {

	CM8ModuleWidget(CM8Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/CM-8.svg")));

		//addChild(Widget::create<ScrewSilver>(Vec(30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 16, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(5, 365)));
		// addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 60, 365)));

		//widget items
        addParam(ParamWidget::create<CM_Knob_big_def_tt>(Vec(34.2 , 18.0), module, CM8Module::PARAM__a, -10.0, 10.0, 0.0f));
		addParam(ParamWidget::create<CM_Knob_big_def_tt>(Vec(5.4 , 58.0), module, CM8Module::PARAM__b, -10.0, 10.0, 0.0f));

		addInput(Port::create<CM_Input_small>(Vec(8.4 , 18.0), Port::INPUT, module, CM8Module::INPUT__a));
        addInput(Port::create<CM_Input_small>(Vec(50.0 , 57.1), Port::INPUT, module, CM8Module::INPUT__b));

        addOutput(Port::create<CM_Output_small>(Vec(8.4 , 39.1), Port::OUTPUT, module, CM8Module::OUTPUT__a));
        addOutput(Port::create<CM_Output_small>(Vec(50.0 , 78.3), Port::OUTPUT, module, CM8Module::OUTPUT__b));

		addParam(ParamWidget::create<CM_Switch_small_3>(Vec(16.4, 103.3), module, CM8Module::PARAM_CIA, 0.0f, 2.0f, 0.0f));
		addInput(Port::create<CM_Input_small>(Vec(54.0 , 112.7), Port::INPUT, module, CM8Module::INPUT_SNH));

		float a = 5.4;
		float b = 46.0;
		float c[8] = {138.8, 166.0, 193.2, 221.9, 249.1, 277.1, 304.3, 331.5};

		addInput(Port::create<CM_Input_def>(Vec(a, c[0]), Port::INPUT, module, CM8Module::INPUT_A));
		addInput(Port::create<CM_Input_def>(Vec(b, c[0]), Port::INPUT, module, CM8Module::INPUT_B));

		addOutput(Port::create<CM_Output_def>(Vec(a , c[1]), Port::OUTPUT, module, CM8Module::OUTPUT_ALTB));
		addOutput(Port::create<CM_Output_def>(Vec(b , c[1]), Port::OUTPUT, module, CM8Module::OUTPUT_BLTA));

		addOutput(Port::create<CM_Output_def>(Vec(a , c[2]), Port::OUTPUT, module, CM8Module::OUTPUT_AISB));
		addOutput(Port::create<CM_Output_def>(Vec(b , c[2]), Port::OUTPUT, module, CM8Module::OUTPUT_ANTB));

		addOutput(Port::create<CM_Output_def>(Vec(a , c[3]), Port::OUTPUT, module, CM8Module::OUTPUT_ACLM));
		addOutput(Port::create<CM_Output_def>(Vec(b , c[3]), Port::OUTPUT, module, CM8Module::OUTPUT_BCLM));

		addOutput(Port::create<CM_Output_def>(Vec(a , c[4]), Port::OUTPUT, module, CM8Module::OUTPUT_AFLD));
		addOutput(Port::create<CM_Output_def>(Vec(b , c[4]), Port::OUTPUT, module, CM8Module::OUTPUT_BFLD));

		addOutput(Port::create<CM_Output_def>(Vec(a , c[5]), Port::OUTPUT, module, CM8Module::OUTPUT_ALO));
		addOutput(Port::create<CM_Output_def>(Vec(b , c[5]), Port::OUTPUT, module, CM8Module::OUTPUT_BLO));

		addOutput(Port::create<CM_Output_def>(Vec(a , c[6]), Port::OUTPUT, module, CM8Module::OUTPUT_AHI));
		addOutput(Port::create<CM_Output_def>(Vec(b , c[6]), Port::OUTPUT, module, CM8Module::OUTPUT_BHI));

		addOutput(Port::create<CM_Output_def>(Vec(a , c[7]), Port::OUTPUT, module, CM8Module::OUTPUT_ARNG));
		addOutput(Port::create<CM_Output_def>(Vec(b , c[7]), Port::OUTPUT, module, CM8Module::OUTPUT_BRNG));

	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
// Model *modelCM8Module = Model::create<CM8Module, CM8ModuleWidget>("CatroModulo", "CatroModulo_CM-8", "C/M8 : aAvsBb", LOGIC_TAG);

RACK_PLUGIN_MODEL_INIT(CatroModulo, CM8Module) {
   Model *model = Model::create<CM8Module, CM8ModuleWidget>("CatroModulo", "CatroModulo_CM8", "C/M8 : aAvsBb", LOGIC_TAG);
   return model;
}