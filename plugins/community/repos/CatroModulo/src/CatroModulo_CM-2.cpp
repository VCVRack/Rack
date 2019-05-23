#include "CatroModulo.hpp"

//Catro-Module 8xatn
//parts of code copied from VCV's 8VERT module from the Fundamental pack by Andew Belt.


struct CM2Module : Module {
	enum ParamIds {
		ENUMS(PARAMS_ATN, 8),
		ENUMS(PARAMS_OFF, 8),
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(INPUTS_IN, 8),
		ENUMS(INPUTS_ATN, 8),
		ENUMS(INPUTS_OFF, 8),
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS = 9
	};
	enum LightIds {
		NUM_LIGHTS = 16
	};

	CM2Module() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void CM2Module::step() {
	float mixOut = 0.0f;
	int numconnect = 0;

	for (int i = 0; i < 8; i++) {
		numconnect += (inputs[INPUTS_IN + i].active) ? 1 : 0;
		float out = 0.0f;

		//cv process
		float attn = (inputs[INPUTS_ATN + i].active) ? clamp(inputs[INPUTS_ATN].value, -10.0, 10.0) * 0.1 * params[PARAMS_ATN + i].value : params[PARAMS_ATN + i].value;
		float offset = (inputs[INPUTS_OFF + i].active) ? clamp(inputs[INPUTS_OFF].value, -10.0, 10.0) * 0.1 * params[PARAMS_OFF + i].value : params[PARAMS_OFF + i].value;

		if (inputs[INPUTS_IN + i].active == true || outputs[i].active == true) {
			if (inputs[INPUTS_IN + i].active == true) {
				out = clamp((inputs[INPUTS_IN + i].value * attn + offset), -10.0f, 10.0f);
				mixOut += out;
			} else {
				out = clamp(attn * offset * 10.0f, -10.0f, 10.0f);
			}
			outputs[i].value = out;
			
			lights[2*i + 0].setBrightnessSmooth(fmaxf(0.0f, out * 0.1f));
			lights[2*i + 1].setBrightnessSmooth(fmaxf(0.0f, -out * 0.1f));
		} else {
			lights[2*i + 0].setBrightnessSmooth(0.0f);
			lights[2*i + 1].setBrightnessSmooth(0.0f);
		}
		if (numconnect > 0) {
			outputs[8].value = mixOut / numconnect;
		}else{
			outputs[8].value = 0.0f;
		}
	}
}

struct CM2ModuleWidget : ModuleWidget {
	CM2ModuleWidget(CM2Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/CM-2_new.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));
	//grid
	const float gridrowjacks[8] = {38.4, 77.2, 116.0, 154.7, 193.5, 232.3, 271.0, 309.8};

	//ATN knobs
	addParam(ParamWidget::create<CM_Knob_big_attn>(Vec(49.0, gridrowjacks[0] - 16.5), module, 0, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<CM_Knob_big_attn>(Vec(49.0, gridrowjacks[1] - 16.5), module, 1, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<CM_Knob_big_attn>(Vec(49.0, gridrowjacks[2] - 16.5), module, 2, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<CM_Knob_big_attn>(Vec(49.0, gridrowjacks[3] - 16.5), module, 3, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<CM_Knob_big_attn>(Vec(49.0, gridrowjacks[4] - 16.5), module, 4, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<CM_Knob_big_attn>(Vec(49.0, gridrowjacks[5] - 16.5), module, 5, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<CM_Knob_big_attn>(Vec(49.0, gridrowjacks[6] - 16.5), module, 6, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<CM_Knob_big_attn>(Vec(49.0, gridrowjacks[7] - 16.5), module, 7, -1.0f, 1.0f, 0.0f));

	//ATN CV
	addInput(Port::create<CM_Input_small>(Vec(32.0, gridrowjacks[0] + 5.1), Port::INPUT, module, CM2Module::INPUTS_ATN + 0));
	addInput(Port::create<CM_Input_small>(Vec(32.0, gridrowjacks[1] + 5.1), Port::INPUT, module, CM2Module::INPUTS_ATN + 1));
	addInput(Port::create<CM_Input_small>(Vec(32.0, gridrowjacks[2] + 5.1), Port::INPUT, module, CM2Module::INPUTS_ATN + 2));
	addInput(Port::create<CM_Input_small>(Vec(32.0, gridrowjacks[3] + 5.1), Port::INPUT, module, CM2Module::INPUTS_ATN + 3));
	addInput(Port::create<CM_Input_small>(Vec(32.0, gridrowjacks[4] + 5.1), Port::INPUT, module, CM2Module::INPUTS_ATN + 4));
	addInput(Port::create<CM_Input_small>(Vec(32.0, gridrowjacks[5] + 5.1), Port::INPUT, module, CM2Module::INPUTS_ATN + 5));
	addInput(Port::create<CM_Input_small>(Vec(32.0, gridrowjacks[6] + 5.1), Port::INPUT, module, CM2Module::INPUTS_ATN + 6));
	addInput(Port::create<CM_Input_small>(Vec(32.0, gridrowjacks[7] + 5.1), Port::INPUT, module, CM2Module::INPUTS_ATN + 7));

	//OFFSET knobs
	addParam(ParamWidget::create<CM_Knob_big_offset>(Vec(98.5, gridrowjacks[0] - 16.5), module, 8, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<CM_Knob_big_offset>(Vec(98.5, gridrowjacks[1] - 16.5), module, 9, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<CM_Knob_big_offset>(Vec(98.5, gridrowjacks[2] - 16.5), module, 10, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<CM_Knob_big_offset>(Vec(98.5, gridrowjacks[3] - 16.5), module, 11, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<CM_Knob_big_offset>(Vec(98.5, gridrowjacks[4] - 16.5), module, 12, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<CM_Knob_big_offset>(Vec(98.5, gridrowjacks[5] - 16.5), module, 13, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<CM_Knob_big_offset>(Vec(98.5, gridrowjacks[6] - 16.5), module, 14, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<CM_Knob_big_offset>(Vec(98.5, gridrowjacks[7] - 16.5), module, 15, -1.0f, 1.0f, 0.0f));

	//OFFSET CV
	addInput(Port::create<CM_Input_small>(Vec(81.3, gridrowjacks[0] + 5.1), Port::INPUT, module, CM2Module::INPUTS_OFF + 0));
	addInput(Port::create<CM_Input_small>(Vec(81.3, gridrowjacks[1] + 5.1), Port::INPUT, module, CM2Module::INPUTS_OFF + 1));
	addInput(Port::create<CM_Input_small>(Vec(81.3, gridrowjacks[2] + 5.1), Port::INPUT, module, CM2Module::INPUTS_OFF + 2));
	addInput(Port::create<CM_Input_small>(Vec(81.3, gridrowjacks[3] + 5.1), Port::INPUT, module, CM2Module::INPUTS_OFF + 3));
	addInput(Port::create<CM_Input_small>(Vec(81.3, gridrowjacks[4] + 5.1), Port::INPUT, module, CM2Module::INPUTS_OFF + 4));
	addInput(Port::create<CM_Input_small>(Vec(81.3, gridrowjacks[5] + 5.1), Port::INPUT, module, CM2Module::INPUTS_OFF + 5));
	addInput(Port::create<CM_Input_small>(Vec(81.3, gridrowjacks[6] + 5.1), Port::INPUT, module, CM2Module::INPUTS_OFF + 6));
	addInput(Port::create<CM_Input_small>(Vec(81.3, gridrowjacks[7] + 5.1), Port::INPUT, module, CM2Module::INPUTS_OFF + 7));

	//Signal IN
	addInput(Port::create<CM_Input_def>(Vec(5.0, gridrowjacks[0]), Port::INPUT, module, CM2Module::INPUTS_IN + 0));
	addInput(Port::create<CM_Input_def>(Vec(5.0, gridrowjacks[1]), Port::INPUT, module, CM2Module::INPUTS_IN + 1));
	addInput(Port::create<CM_Input_def>(Vec(5.0, gridrowjacks[2]), Port::INPUT, module, CM2Module::INPUTS_IN + 2));
	addInput(Port::create<CM_Input_def>(Vec(5.0, gridrowjacks[3]), Port::INPUT, module, CM2Module::INPUTS_IN + 3));
	addInput(Port::create<CM_Input_def>(Vec(5.0, gridrowjacks[4]), Port::INPUT, module, CM2Module::INPUTS_IN + 4));
	addInput(Port::create<CM_Input_def>(Vec(5.0, gridrowjacks[5]), Port::INPUT, module, CM2Module::INPUTS_IN + 5));
	addInput(Port::create<CM_Input_def>(Vec(5.0, gridrowjacks[6]), Port::INPUT, module, CM2Module::INPUTS_IN + 6));
	addInput(Port::create<CM_Input_def>(Vec(5.0, gridrowjacks[7]), Port::INPUT, module, CM2Module::INPUTS_IN + 7));

	//Signal OUT
	addOutput(Port::create<CM_Output_def>(Vec(134.6, gridrowjacks[0]), Port::OUTPUT, module, 0));
	addOutput(Port::create<CM_Output_def>(Vec(134.6, gridrowjacks[1]), Port::OUTPUT, module, 1));
	addOutput(Port::create<CM_Output_def>(Vec(134.6, gridrowjacks[2]), Port::OUTPUT, module, 2));
	addOutput(Port::create<CM_Output_def>(Vec(134.6, gridrowjacks[3]), Port::OUTPUT, module, 3));
	addOutput(Port::create<CM_Output_def>(Vec(134.6, gridrowjacks[4]), Port::OUTPUT, module, 4));
	addOutput(Port::create<CM_Output_def>(Vec(134.6, gridrowjacks[5]), Port::OUTPUT, module, 5));
	addOutput(Port::create<CM_Output_def>(Vec(134.6, gridrowjacks[6]), Port::OUTPUT, module, 6));
	addOutput(Port::create<CM_Output_def>(Vec(134.6, gridrowjacks[7]), Port::OUTPUT, module, 7));

	addOutput(Port::create<CM_Output_small>(Vec(98.1, 336.3), Port::OUTPUT, module, 8));

	addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(163.5, gridrowjacks[0] + 10), module, 0));
	addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(163.5, gridrowjacks[1] + 10), module, 2));
	addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(163.5, gridrowjacks[2] + 10), module, 4));
	addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(163.5, gridrowjacks[3] + 10), module, 6));
	addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(163.5, gridrowjacks[4] + 10), module, 8));
	addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(163.5, gridrowjacks[5] + 10), module, 10));
	addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(163.5, gridrowjacks[6] + 10), module, 12));
	addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(163.5, gridrowjacks[7] + 10), module, 14));

	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
// Model *modelCM2Module = Model::create<CM2Module, CM2ModuleWidget>("CatroModulo", "CatroModulo_CM-2", "C/M2 : 8xatn", ATTENUATOR_TAG);

RACK_PLUGIN_MODEL_INIT(CatroModulo, CM2Module) {
   Model *model = Model::create<CM2Module, CM2ModuleWidget>("CatroModulo", "CatroModulo_CM2", "C/M2 : 8xatn", ATTENUATOR_TAG);
   return model;
}