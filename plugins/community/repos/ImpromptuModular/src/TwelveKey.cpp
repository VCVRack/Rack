//***********************************************************************************************
//Chain-able keyboard module for VCV Rack by Marc Boulé
//
//Based on code from the Fundamental and Audible Instruments plugins by Andrew Belt and graphics  
//  from the Component Library by Wes Milholen. 
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//
//Module inspired by:
//  * the Autodafe keyboard by Antonio Grazioli 
//  * the cf mixer by Clément Foulc
//  * Twisted Electrons' KeyChain 
//
//***********************************************************************************************


#include "ImpromptuModular.hpp"

namespace rack_plugin_ImpromptuModular {

struct TwelveKey : Module {
	enum ParamIds {
		OCTINC_PARAM,
		OCTDEC_PARAM,
		ENUMS(KEY_PARAMS, 12),
		NUM_PARAMS
	};
	enum InputIds {
		GATE_INPUT,
		CV_INPUT,	
		OCT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		GATE_OUTPUT,
		CV_OUTPUT,	
		OCT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		PRESS_LIGHT,// no longer used
		ENUMS(KEY_LIGHTS, 12),
		NUM_LIGHTS
	};
	
	// Need to save
	int panelTheme = 0;
	int octaveNum;// 0 to 9
	float cv;
	bool stateInternal;// false when pass through CV and Gate, true when CV and gate from this module
	
	// No need to save
	unsigned long noteLightCounter;// 0 when no key to light, downward step counter timer when key lit
	int lastKeyPressed;// 0 to 11

	
	unsigned int lightRefreshCounter = 0;
	//float gateLight = 0.0f;
	Trigger keyTriggers[12];
	Trigger gateInputTrigger;
	Trigger octIncTrigger;
	Trigger octDecTrigger;
	

	TwelveKey() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void onReset() override {
		octaveNum = 4;
		cv = 0.0f;
		stateInternal = inputs[GATE_INPUT].active ? false : true;
		noteLightCounter = 0ul;
		lastKeyPressed = 0;
	}

	void onRandomize() override {
		octaveNum = randomu32() % 10;
		cv = ((float)(octaveNum - 4)) + ((float)(randomu32() % 12)) / 12.0f;
		stateInternal = inputs[GATE_INPUT].active ? false : true;
		noteLightCounter = 0ul;
		lastKeyPressed = 0;
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();
		
		// panelTheme
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));

		// cv
		json_object_set_new(rootJ, "cv", json_real(cv));
		
		// octave
		json_object_set_new(rootJ, "octave", json_integer(octaveNum));
		
		// stateInternal
		json_object_set_new(rootJ, "stateInternal", json_boolean(stateInternal));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// panelTheme
		json_t *panelThemeJ = json_object_get(rootJ, "panelTheme");
		if (panelThemeJ)
			panelTheme = json_integer_value(panelThemeJ);

		// cv
		json_t *cvJ = json_object_get(rootJ, "cv");
		if (cvJ)
			cv = json_number_value(cvJ);
		
		// octave
		json_t *octaveJ = json_object_get(rootJ, "octave");
		if (octaveJ)
			octaveNum = json_integer_value(octaveJ);

		// stateInternal
		json_t *stateInternalJ = json_object_get(rootJ, "stateInternal");
		if (stateInternalJ)
			stateInternal = json_is_true(stateInternalJ);
	}

	
	void step() override {		
		static const float noteLightTime = 0.5f;// seconds
		
		//********** Buttons, knobs, switches and inputs **********
		
		bool upOctTrig = false;
		bool downOctTrig = false;
		
		if ((lightRefreshCounter & userInputsStepSkipMask) == 0) {
		
			// Octave buttons and input
			upOctTrig = octIncTrigger.process(params[OCTINC_PARAM].value);
			downOctTrig = octDecTrigger.process(params[OCTDEC_PARAM].value);
				
			// Keyboard buttons and gate input
			for (int i = 0; i < 12; i++) {
				if (keyTriggers[i].process(params[KEY_PARAMS + i].value)) {
					cv = ((float)(octaveNum - 4)) + ((float) i) / 12.0f;
					stateInternal = true;
					noteLightCounter = (unsigned long) (noteLightTime * engineGetSampleRate() / displayRefreshStepSkips);
					lastKeyPressed = i;
				}
			}
		
		}// userInputs refresh
		
		
		if (inputs[OCT_INPUT].active)
			octaveNum = ((int) floor(inputs[OCT_INPUT].value));
		else if (upOctTrig)
			octaveNum++;
		else if (downOctTrig)
			octaveNum--;
		if (octaveNum > 9) octaveNum = 9;
		if (octaveNum < 0) octaveNum = 0;
		
		if (gateInputTrigger.process(inputs[GATE_INPUT].value)) {// no input refresh here, don't want propagation lag in long 12-key chain
			cv = inputs[CV_INPUT].value;			
			stateInternal = false;
		}
		
		
		//********** Outputs and lights **********
		
		// cv output
		outputs[CV_OUTPUT].value = cv;
		
		// gate output
		if (stateInternal == false) {// if receiving a key from left chain
			outputs[GATE_OUTPUT].value = inputs[GATE_INPUT].value;
		}
		else {// key from this
			outputs[GATE_OUTPUT].value = (params[KEY_PARAMS + lastKeyPressed].value > 0.5f) ? 10.0f : 0.0f;
		}
		
		// Octave output
		outputs[OCT_OUTPUT].value = round( (float)(octaveNum + 1) );
		
		lightRefreshCounter++;
		if (lightRefreshCounter >= displayRefreshStepSkips) {
			lightRefreshCounter = 0;

			// Key lights
			for (int i = 0; i < 12; i++)
				lights[KEY_LIGHTS + i].value = (( i == lastKeyPressed && (noteLightCounter > 0ul || params[KEY_PARAMS + i].value > 0.5f)) ? 1.0f : 0.0f);
			
			if (noteLightCounter > 0ul)
				noteLightCounter--;
		}
	}
};


struct TwelveKeyWidget : ModuleWidget {

	struct OctaveNumDisplayWidget : TransparentWidget {
		int *octaveNum;
		std::shared_ptr<Font> font;
		
		OctaveNumDisplayWidget() {
			font = Font::load(assetPlugin(plugin, "res/fonts/Segment14.ttf"));
		}

		void draw(NVGcontext *vg) override {
			NVGcolor textColor = prepareDisplay(vg, &box, 18);
			nvgFontFaceId(vg, font->handle);
			//nvgTextLetterSpacing(vg, 2.5);

			Vec textPos = Vec(6, 24);
			nvgFillColor(vg, nvgTransRGBA(textColor, displayAlpha));
			nvgText(vg, textPos.x, textPos.y, "~", NULL);
			nvgFillColor(vg, textColor);
			char displayStr[2];
			displayStr[0] = 0x30 + (char) *octaveNum;
			displayStr[1] = 0;
			nvgText(vg, textPos.x, textPos.y, displayStr, NULL);
		}
	};
	
	
	struct PanelThemeItem : MenuItem {
		TwelveKey *module;
		int theme;
		void onAction(EventAction &e) override {
			module->panelTheme = theme;
		}
		void step() override {
			rightText = (module->panelTheme == theme) ? "✔" : "";
		}
	};
	Menu *createContextMenu() override {
		Menu *menu = ModuleWidget::createContextMenu();

		MenuLabel *spacerLabel = new MenuLabel();
		menu->addChild(spacerLabel);

		TwelveKey *module = dynamic_cast<TwelveKey*>(this->module);
		assert(module);

		MenuLabel *themeLabel = new MenuLabel();
		themeLabel->text = "Panel Theme";
		menu->addChild(themeLabel);

		PanelThemeItem *lightItem = new PanelThemeItem();
		lightItem->text = lightPanelID;// ImpromptuModular.hpp
		lightItem->module = module;
		lightItem->theme = 0;
		menu->addChild(lightItem);

		PanelThemeItem *darkItem = new PanelThemeItem();
		darkItem->text = darkPanelID;// ImpromptuModular.hpp
		darkItem->module = module;
		darkItem->theme = 1;
		menu->addChild(darkItem);

		return menu;
	}	
	
	
	TwelveKeyWidget(TwelveKey *module) : ModuleWidget(module) {
		// Main panel from Inkscape
        DynamicSVGPanel *panel = new DynamicSVGPanel();
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/TwelveKey.svg")));
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/dark/TwelveKey_dark.svg")));
        box.size = panel->box.size;
        panel->mode = &module->panelTheme;
        addChild(panel);

		// Screws
		addChild(createDynamicScrew<IMScrew>(Vec(15, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(box.size.x-30, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(15, 365), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(box.size.x-30, 365), &module->panelTheme));



		// ****** Top portion (keys) ******

		static const int offsetKeyLEDx = 12;
		static const int offsetKeyLEDy = 41;

		// Black keys
		addParam(ParamWidget::create<InvisibleKey>(Vec(30, 40), module, TwelveKey::KEY_PARAMS + 1, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(30+offsetKeyLEDx, 40+offsetKeyLEDy), module, TwelveKey::KEY_LIGHTS + 1));
		addParam(ParamWidget::create<InvisibleKey>(Vec(71, 40), module, TwelveKey::KEY_PARAMS + 3, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(71+offsetKeyLEDx, 40+offsetKeyLEDy), module, TwelveKey::KEY_LIGHTS + 3));
		addParam(ParamWidget::create<InvisibleKey>(Vec(154, 40), module, TwelveKey::KEY_PARAMS + 6, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(154+offsetKeyLEDx, 40+offsetKeyLEDy), module, TwelveKey::KEY_LIGHTS + 6));
		addParam(ParamWidget::create<InvisibleKey>(Vec(195, 40), module, TwelveKey::KEY_PARAMS + 8, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(195+offsetKeyLEDx, 40+offsetKeyLEDy), module, TwelveKey::KEY_LIGHTS + 8));
		addParam(ParamWidget::create<InvisibleKey>(Vec(236, 40), module, TwelveKey::KEY_PARAMS + 10, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(236+offsetKeyLEDx, 40+offsetKeyLEDy), module, TwelveKey::KEY_LIGHTS + 10));

		// White keys
		addParam(ParamWidget::create<InvisibleKey>(Vec(10, 112), module, TwelveKey::KEY_PARAMS + 0, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(10+offsetKeyLEDx, 112+offsetKeyLEDy), module, TwelveKey::KEY_LIGHTS + 0));
		addParam(ParamWidget::create<InvisibleKey>(Vec(51, 112), module, TwelveKey::KEY_PARAMS + 2, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(51+offsetKeyLEDx, 112+offsetKeyLEDy), module, TwelveKey::KEY_LIGHTS + 2));
		addParam(ParamWidget::create<InvisibleKey>(Vec(92, 112), module, TwelveKey::KEY_PARAMS + 4, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(92+offsetKeyLEDx, 112+offsetKeyLEDy), module, TwelveKey::KEY_LIGHTS + 4));
		addParam(ParamWidget::create<InvisibleKey>(Vec(133, 112), module, TwelveKey::KEY_PARAMS + 5, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(133+offsetKeyLEDx, 112+offsetKeyLEDy), module, TwelveKey::KEY_LIGHTS + 5));
		addParam(ParamWidget::create<InvisibleKey>(Vec(174, 112), module, TwelveKey::KEY_PARAMS + 7, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(174+offsetKeyLEDx, 112+offsetKeyLEDy), module, TwelveKey::KEY_LIGHTS + 7));
		addParam(ParamWidget::create<InvisibleKey>(Vec(215, 112), module, TwelveKey::KEY_PARAMS + 9, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(215+offsetKeyLEDx, 112+offsetKeyLEDy), module, TwelveKey::KEY_LIGHTS + 9));
		addParam(ParamWidget::create<InvisibleKey>(Vec(256, 112), module, TwelveKey::KEY_PARAMS + 11, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(256+offsetKeyLEDx, 112+offsetKeyLEDy), module, TwelveKey::KEY_LIGHTS + 11));
		
		
		// ****** Bottom portion ******

		// Column rulers (horizontal positions)
		static const int columnRulerL = 30;
		static const int columnRulerR = box.size.x - 25 - columnRulerL;
		static const int columnRulerM = box.size.x / 2 - 14;
		
		// Row rulers (vertical positions)
		static const int rowRuler0 = 220;
		static const int rowRulerStep = 49;
		static const int rowRuler1 = rowRuler0 + rowRulerStep;
		static const int rowRuler2 = rowRuler1 + rowRulerStep;
		
		// Left side inputs
		
		
		addInput(createDynamicPort<IMPort>(Vec(columnRulerL, rowRuler0), Port::INPUT, module, TwelveKey::CV_INPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(columnRulerL, rowRuler1), Port::INPUT, module, TwelveKey::GATE_INPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(columnRulerL, rowRuler2), Port::INPUT, module, TwelveKey::OCT_INPUT, &module->panelTheme));

		// Middle
		// Press LED (moved other controls below up by 16 px when removed, to better center)
		//addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(columnRulerM + offsetMediumLight, rowRuler0 - 31 + offsetMediumLight), module, TwelveKey::PRESS_LIGHT));
		// Octave display
		OctaveNumDisplayWidget *octaveNumDisplay = new OctaveNumDisplayWidget();
		octaveNumDisplay->box.pos = Vec(columnRulerM + 2, rowRuler1 - 27 + vOffsetDisplay);
		octaveNumDisplay->box.size = Vec(24, 30);// 1 character
		octaveNumDisplay->octaveNum = &module->octaveNum;
		addChild(octaveNumDisplay);
		
		// Octave buttons
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerM - 20 + offsetCKD6b, rowRuler2 - 26 + offsetCKD6b), module, TwelveKey::OCTDEC_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerM + 22 + offsetCKD6b, rowRuler2 - 26 + offsetCKD6b), module, TwelveKey::OCTINC_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		
		// Right side outputs
		addOutput(createDynamicPort<IMPort>(Vec(columnRulerR, rowRuler0), Port::OUTPUT, module, TwelveKey::CV_OUTPUT, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRulerR, rowRuler1), Port::OUTPUT, module, TwelveKey::GATE_OUTPUT, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRulerR, rowRuler2), Port::OUTPUT, module, TwelveKey::OCT_OUTPUT, &module->panelTheme));
	}
};

} // namespace rack_plugin_ImpromptuModular

using namespace rack_plugin_ImpromptuModular;

RACK_PLUGIN_MODEL_INIT(ImpromptuModular, TwelveKey) {
   Model *modelTwelveKey = Model::create<TwelveKey, TwelveKeyWidget>("Impromptu Modular", "Twelve-Key", "CTRL - Twelve-Key", CONTROLLER_TAG);
   return modelTwelveKey;
}

/*CHANGE LOG

0.6.12:
input refresh optimization

*/
