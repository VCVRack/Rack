#include "Template.hpp"
#include "dsp/digital.hpp"
#include "formula/Formula.h"

namespace rack_plugin_FrankBussFormula {

struct FrankBussFormulaModule;

class MyTextField : public LedDisplayTextField {
public:
	MyTextField() : LedDisplayTextField() {}
	void setModule(FrankBussFormulaModule* _module) {
		module = _module;
	}
	virtual void onTextChange() override;

private:
	FrankBussFormulaModule* module;
};

struct FrankBussFormulaModule : Module {
	enum ParamIds {
		X_PARAM,
		Y_PARAM,
		Z_PARAM,
		KNOB_PARAM,
		CLAMP_PARAM,
		B_MINUS_1_PARAM,
		B_0_PARAM,
		B_1_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		X_INPUT,
		Y_INPUT,
		Z_INPUT,
		W_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		FORMULA_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		CLAMP_LIGHT,
		B_MINUS_1_LIGHT,
		B_0_LIGHT,
		B_1_LIGHT,
		NUM_LIGHTS
	};

	MyTextField* textField;
	MyTextField* freqField;
	float blinkPhase = 0.0f;

	Formula formula;
	Formula freqFormula;
	bool compiled = false;
	bool doclamp = true;
	bool freqFormulaEnabled = false;
	float radiobutton = 0.0f;
	float phase = 0.0f;

	SchmittTrigger clampTrigger;
	SchmittTrigger bMinus1Trigger;
	SchmittTrigger b0Trigger;
	SchmittTrigger b1Trigger;

	float* formulaP = NULL;
	float* formulaK = NULL;
	float* formulaB = NULL;
	float* formulaW = NULL;
	float* formulaX = NULL;
	float* formulaY = NULL;
	float* formulaZ = NULL;

	float* freqFormulaP = NULL;
	float* freqFormulaK = NULL;
	float* freqFormulaB = NULL;
	float* freqFormulaW = NULL;
	float* freqFormulaX = NULL;
	float* freqFormulaY = NULL;
	float* freqFormulaZ = NULL;

	FrankBussFormulaModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}

	void step() override {
		if (clampTrigger.process(params[CLAMP_PARAM].value)) {
			doclamp = !doclamp;
		}
		if (bMinus1Trigger.process(params[B_MINUS_1_PARAM].value)) {
			radiobutton = -1.0f;
		}	
		if (b0Trigger.process(params[B_0_PARAM].value)) {
			radiobutton = 0.0f;
		}
		if (b1Trigger.process(params[B_1_PARAM].value)) {
			radiobutton = 1.0f;
		}
		
		float deltaTime = engineGetSampleTime();

		// evaluate frequency and output formula
		float val = 0;
		if (compiled) {
			try {
				// get inputs
				float w = inputs[W_INPUT].value;
				float x = inputs[X_INPUT].value;
				float y = inputs[Y_INPUT].value;
				float z = inputs[Z_INPUT].value;
				
				// knob
				float k = params[KNOB_PARAM].value;

				// set all variables
				*formulaP = phase;
				*formulaK = k;
				*formulaB = radiobutton;
				*formulaW = w;
				*formulaX = x;
				*formulaY = y;
				*formulaZ = z;

				if (freqFormulaEnabled) {
					*freqFormulaP = phase;
					*freqFormulaK = k;
					*freqFormulaB = radiobutton;
					*freqFormulaW = w;
					*freqFormulaX = x;
					*freqFormulaY = y;
					*freqFormulaZ = z;
					float freq = evalFormula(freqFormula);
					phase += freq * engineGetSampleTime();
					if (phase > 1.0f) phase -= 1.0f;
				}
				val = evalFormula(formula);
				if (doclamp) val = clamp(val, -5.0f, 5.0f);
			} catch (MathError&) {
				// ignore math errors, e.g. division by zero
			} catch (exception&) {
				// for all other exceptions, set compiled to false, e.g. VariableNotFound
				compiled = false;
			}
		}

		// set output
		outputs[FORMULA_OUTPUT].value = val;

		// Blink light at 1Hz
		blinkPhase += deltaTime;
		if (blinkPhase >= 1.0f)
			blinkPhase -= 1.0f;
		if (compiled) {
			lights[BLINK_LIGHT].value = 1.0f;
		} else {
			lights[BLINK_LIGHT].value = (blinkPhase < 0.5f) ? 1.0f : 0.0f;
		}

		lights[CLAMP_LIGHT].value = (doclamp);
		lights[B_MINUS_1_LIGHT].value = (radiobutton == -1.0f);
		lights[B_0_LIGHT].value = (radiobutton == -0.0f);
		lights[B_1_LIGHT].value = (radiobutton == 1.0f);
	}

	void parseFormula(Formula& formula, string expr) {
		formula.setVariable("pi", float(M_PI));
		formula.setVariable("e", float(M_E));
		
		formula.setVariable("p", 0);
		formula.setVariable("k", 0);
		formula.setVariable("b", 0);
		formula.setVariable("w", 0);
		formula.setVariable("x", 0);
		formula.setVariable("y", 0);
		formula.setVariable("z", 0);

		formula.setExpression(expr);
	}

	float evalFormula(Formula& formula) {
		// eval
		float val = formula.eval();
		if (!isfinite(val) || isnan(val)) val = 0.0f;
		return val;
	}

	void onCreate () override
	{
		compiled = false;
		phase = 0;
		if (textField->text.size() > 0) {
			try {
				parseFormula(formula, textField->text);
				freqFormulaEnabled = false;
				if (freqField->text.size() > 0) {
					parseFormula(freqFormula, freqField->text);
					freqFormulaEnabled = true;
				}
				
				formulaP = formula.getVariableAddress("p");
				formulaK = formula.getVariableAddress("k");
				formulaB = formula.getVariableAddress("b");
				formulaW = formula.getVariableAddress("w");
				formulaX = formula.getVariableAddress("x");
				formulaY = formula.getVariableAddress("y");
				formulaZ = formula.getVariableAddress("z");

				if (freqFormulaEnabled) {
					freqFormulaP = freqFormula.getVariableAddress("p");
					freqFormulaK = freqFormula.getVariableAddress("k");
					freqFormulaB = freqFormula.getVariableAddress("b");
					freqFormulaW = freqFormula.getVariableAddress("w");
					freqFormulaX = freqFormula.getVariableAddress("x");
					freqFormulaY = freqFormula.getVariableAddress("y");
					freqFormulaZ = freqFormula.getVariableAddress("z");
				}
				
				compiled = true;
			} catch (exception& e) {
				printf("formula exception: %s\n", e.what());
			}
		}
	}

	void onReset () override
	{
		onCreate();
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();

		json_object_set_new(rootJ, "text", json_string(textField->text.c_str()));
		json_object_set_new(rootJ, "freq", json_string(freqField->text.c_str()));
		json_object_set_new(rootJ, "clamp", json_boolean(doclamp));
		json_object_set_new(rootJ, "button", json_real(radiobutton));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *textJ = json_object_get(rootJ, "text");
		if (textJ) textField->text = json_string_value(textJ);

		json_t *freqJ = json_object_get(rootJ, "freq");
		if (freqJ) freqField->text = json_string_value(freqJ);

		json_t *clampJ = json_object_get(rootJ, "clamp");
		if (clampJ) doclamp = json_is_true(clampJ);
		
		json_t *buttonJ = json_object_get(rootJ, "button");
		if (buttonJ) radiobutton = (float)json_real_value(buttonJ);

		onCreate();
	}

};

void MyTextField::onTextChange() {
	module->onCreate();
}

struct FrankBussFormulaWidget : ModuleWidget {
	FrankBussFormulaWidget(FrankBussFormulaModule *module) : ModuleWidget(module) {

		setPanel(SVG::load(assetPlugin(plugin, "res/MyModule.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		textField = Widget::create<MyTextField>(mm2px(Vec(3, 15)));
		textField->setModule(module);
		textField->box.size = mm2px(Vec(85, 50));
		textField->multiline = true;
		addChild(textField);
		module->textField = textField;

		freqField = Widget::create<MyTextField>(mm2px(Vec(16, 68)));
		freqField->setModule(module);
		freqField->box.size = mm2px(Vec(72, 10));
		freqField->multiline = false;
		addChild(freqField);
		module->freqField = freqField;

		addParam(ParamWidget::create<LEDButton>(Vec(30, 260), module, FrankBussFormulaModule::B_MINUS_1_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(34.4f, 264.4f), module, FrankBussFormulaModule::B_MINUS_1_LIGHT));
		
		addParam(ParamWidget::create<LEDButton>(Vec(75, 260), module, FrankBussFormulaModule::B_0_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(79.4f, 264.4f), module, FrankBussFormulaModule::B_0_LIGHT));

		addParam(ParamWidget::create<LEDButton>(Vec(120, 260), module, FrankBussFormulaModule::B_1_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(124.4f, 264.4f), module, FrankBussFormulaModule::B_1_LIGHT));

		addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(170, 240), module, FrankBussFormulaModule::KNOB_PARAM, -1.0f, 1.0f, 0.0f));

		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(240, 240), module, FrankBussFormulaModule::BLINK_LIGHT));

		addInput(Port::create<PJ301MPort>(Vec(20, 310), Port::INPUT, module, FrankBussFormulaModule::W_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(60, 310), Port::INPUT, module, FrankBussFormulaModule::X_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(100, 310), Port::INPUT, module, FrankBussFormulaModule::Y_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(140, 310), Port::INPUT, module, FrankBussFormulaModule::Z_INPUT));

		addParam(ParamWidget::create<LEDButton>(Vec(190, 314), module, FrankBussFormulaModule::CLAMP_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(194.4f, 318.4f), module, FrankBussFormulaModule::CLAMP_LIGHT));

		addOutput(Port::create<PJ301MPort>(Vec(220, 310), Port::OUTPUT, module, FrankBussFormulaModule::FORMULA_OUTPUT));
	}

	// for backward compatibility, now it is all saved in the module
	void fromJson(json_t *rootJ) override {
		ModuleWidget::fromJson(rootJ);

		// text
		json_t *textJ = json_object_get(rootJ, "text");
		if (textJ)
		{
			textField->text = json_string_value(textJ);
			module->onCreate();
		}
	}
	
	MyTextField* textField;
	MyTextField* freqField;
};

} // namespace rack_plugin_FrankBussFormula

using namespace rack_plugin_FrankBussFormula;

RACK_PLUGIN_MODEL_INIT(FrankBussFormula, FrankBussFormula) {
   Model *modelFrankBussFormula = Model::create<FrankBussFormulaModule, FrankBussFormulaWidget>("Frank Buss", "FrankBussFormula", "Formula", UTILITY_TAG);
   return modelFrankBussFormula;
}
