//***********************************************************************************************
//Tactile CV controller module for VCV Rack by Marc Boulé
//
//Based on code from the Fundamental and Audible Instruments plugins by Andrew Belt and graphics  
//  from the Component Library by Wes Milholen. 
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//
//***********************************************************************************************


#include "ImpromptuModular.hpp"

namespace rack_plugin_ImpromptuModular {

struct Tact : Module {
	static const int numLights = 10;// number of lights per channel

	enum ParamIds {
		ENUMS(TACT_PARAMS, 2),// touch pads
		ENUMS(ATTV_PARAMS, 2),// max knobs
		ENUMS(RATE_PARAMS, 2),// rate knobs
		LINK_PARAM,
		ENUMS(SLIDE_PARAMS, 2),// slide switches
		ENUMS(STORE_PARAMS, 2),// store buttons
		// -- 0.6.7 ^^
		EXP_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(TOP_INPUTS, 2),
		ENUMS(BOT_INPUTS, 2),
		ENUMS(RECALL_INPUTS, 2),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(CV_OUTPUTS, 2),
		// -- 0.6.7 ^^
		ENUMS(EOC_OUTPUTS, 2),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(TACT_LIGHTS, numLights * 2 + numLights * 2), // first N lights for channel L, other N for channel R (*2 for GreenRed)
		ENUMS(CVIN_LIGHTS, 2 * 2),// GreenRed
		NUM_LIGHTS
	};
	
	
	// Constants
	static constexpr float storeInfoTime = 0.5f;// seconds	
	
	// Need to save, with reset
	double cv[2];// actual Tact CV since Tactknob can be different than these when transitioning
	float storeCV[2];

	// Need to save, no reset
	int panelTheme;
	
	// No need to save, with reset
	// none
	
	// No need to save, no reset
	bool scheduledReset;
	long infoStore;// 0 when no info, positive downward step counter when store left channel, negative upward for right
	float infoCVinLight[2];
	SchmittTrigger topTriggers[2];
	SchmittTrigger botTriggers[2];
	SchmittTrigger storeTriggers[2];
	SchmittTrigger recallTriggers[2];
	PulseGenerator eocPulses[2];
	float paramReadRequest[2]; 
	int lightRefreshCounter;
	
	inline bool isLinked(void) {return params[LINK_PARAM].value > 0.5f;}
	inline bool isExpSliding(void) {return params[EXP_PARAM].value > 0.5f;}

	
	Tact() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		// Need to save, no reset
		panelTheme = 0;
		// No need to save, no reset		
		scheduledReset = false;
		infoStore = 0l;
		for (int i = 0; i < 2; i++) {
			infoCVinLight[i] = 0.0f;
			topTriggers[i].reset();
			botTriggers[i].reset();
			storeTriggers[i].reset();
			recallTriggers[i].reset();
			eocPulses[i].reset();
			paramReadRequest[i] = -10.0f;// -10.0f when no request being made, value to read otherwize
		}
		lightRefreshCounter = 0;
		
		onReset();
	}

	
	// widgets are not yet created when module is created 
	// even if widgets not created yet, can use params[] and should handle 0.0f value since step may call 
	//   this before widget creation anyways
	// called from the main thread if by constructor, called by engine thread if right-click initialization
	//   when called by constructor, module is created before the first step() is called
	void onReset() override {
		// Need to save, with reset
		for (int i = 0; i < 2; i++) {
			cv[i] = 0.0f;
			storeCV[i] = 0.0f;
		}
		// No need to save, with reset
		// none
		
		scheduledReset = true;
	}

	
	// widgets randomized before onRandomize() is called
	// called by engine thread if right-click randomize
	void onRandomize() override {
		// Need to save, with reset
		for (int i = 0; i < 2; i++) {
			cv[i] = clamp(params[TACT_PARAMS + i].value, 0.0f, 10.0f);
			storeCV[i] = 0.0f;
		}
		// No need to save, with reset
		// none
	
		scheduledReset = true;
	}

	
	// called by main thread
	json_t *toJson() override {
		json_t *rootJ = json_object();
		// Need to save (reset or not)

		// cv
		json_object_set_new(rootJ, "cv0", json_real(cv[0]));
		json_object_set_new(rootJ, "cv1", json_real(cv[1]));
		
		// storeCV
		json_object_set_new(rootJ, "storeCV0", json_real(storeCV[0]));
		json_object_set_new(rootJ, "storeCV1", json_real(storeCV[1]));
		
		// panelTheme
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));

		return rootJ;
	}

	
	// widgets have their fromJson() called before this fromJson() is called
	// called by main thread
	void fromJson(json_t *rootJ) override {
		// Need to save (reset or not)

		// cv
		json_t *cv0J = json_object_get(rootJ, "cv0");
		if (cv0J)
			cv[0] = json_real_value(cv0J);
		json_t *cv1J = json_object_get(rootJ, "cv1");
		if (cv1J)
			cv[1] = json_real_value(cv1J);

		// storeCV[0]
		json_t *storeCV0J = json_object_get(rootJ, "storeCV0");
		if (storeCV0J)
			storeCV[0] = json_real_value(storeCV0J);
		json_t *storeCV1J = json_object_get(rootJ, "storeCV1");
		if (storeCV1J)
			storeCV[1] = json_real_value(storeCV1J);

		// panelTheme
		json_t *panelThemeJ = json_object_get(rootJ, "panelTheme");
		if (panelThemeJ)
			panelTheme = json_integer_value(panelThemeJ);

		// No need to save, with reset
		// none
		
		scheduledReset = true;
	}

	
	// Advances the module by 1 audio frame with duration 1.0 / engineGetSampleRate()
	void step() override {		
		float sampleRate = engineGetSampleRate();
		float sampleTime = engineGetSampleTime();
		
		// Scheduled reset (just the parts that do not have a place below in rest of function)
		if (scheduledReset) {
			infoStore = 0l;
			for (int i = 0; i < 2; i++) {
				infoCVinLight[i] = 0.0f;
				topTriggers[i].reset();
				botTriggers[i].reset();
				storeTriggers[i].reset();
				recallTriggers[i].reset();
				eocPulses[i].reset();
				paramReadRequest[i] = -10.0f;// -10.0f when no request being made, value to read otherwize
			}		
		}
		
		// store buttons
		for (int i = 0; i < 2; i++) {
			if (storeTriggers[i].process(params[STORE_PARAMS + i].value)) {
				if ( !(i == 1 && isLinked()) ) {// ignore right channel store-button press when linked
					storeCV[i] = cv[i];
					infoStore = (long) (storeInfoTime * sampleRate / displayRefreshStepSkips) * (i == 0 ? 1l : -1l);
				}
			}
		}
		
		// top/bot/recall CV inputs
		for (int i = 0; i < 2; i++) {
			if (topTriggers[i].process(inputs[TOP_INPUTS + i].value)) {
				if ( !(i == 1 && isLinked()) ) {// ignore right channel top cv in when linked
					//tactWidgets[i]->changeValue(10.0f);
					paramReadRequest[i] = 10.0f;
					infoCVinLight[i] = 1.0f;
				}
			}
			if (botTriggers[i].process(inputs[BOT_INPUTS + i].value)) {
				if ( !(i == 1 && isLinked()) ) {// ignore right channel bot cv in when linked
					//tactWidgets[i]->changeValue(0.0f);
					paramReadRequest[i] = 0.0f;
					infoCVinLight[i] = 1.0f;
				}				
			}
			if (recallTriggers[i].process(inputs[RECALL_INPUTS + i].value)) {// ignore right channel recall cv in when linked
				if ( !(i == 1 && isLinked()) ) {
					//tactWidgets[i]->changeValue(storeCV[i]);
					paramReadRequest[i] = storeCV[i];
					if (params[SLIDE_PARAMS + i].value < 0.5f) //if no slide
						cv[i]=storeCV[i];
					infoCVinLight[i] = 1.0f;
				}				
			}
		}
		
		
		// cv
		bool expSliding = isExpSliding();
		for (int i = 0; i < 2; i++) {
			if (paramReadRequest[i] != -10.0f)
				continue;			
			float newParamValue = clamp(params[TACT_PARAMS + i].value, 0.0f, 10.0f);
			if (newParamValue != cv[i]) {
				double transitionRate = params[RATE_PARAMS + i].value; // s/V
				double dt = sampleTime;
				if ((newParamValue - cv[i]) > 0.001f && transitionRate > 0.001) {
					double dV = expSliding ? (cv[i] + 1.0) * (pow(11.0, dt / (10.0 * transitionRate)) - 1.0) : dt/transitionRate;
					double newCV = cv[i] + dV;
					if (newCV > newParamValue) {
						cv[i] = newParamValue;
						eocPulses[i].trigger(0.001f);
					}
					else
						cv[i] = (float)newCV;
				}
				else if ((newParamValue - cv[i]) < -0.001f && transitionRate > 0.001) {
					dt *= -1.0;
					double dV = expSliding ? (cv[i] + 1.0) * (pow(11.0, dt / (10.0 * transitionRate)) - 1.0) : dt/transitionRate;
					double newCV = cv[i] + dV;
					if (newCV < newParamValue) {
						cv[i] = newParamValue;
						eocPulses[i].trigger(0.001f);
					}
					else
						cv[i] = (float)newCV;
				}
				else {// too close to target or rate too fast, thus no slide
					if (fabs(cv[i] - newParamValue) > 1e-6)
						eocPulses[i].trigger(0.001f);
					cv[i] = newParamValue;	
				}
			}
		}
		
	
		// CV and EOC Outputs
		bool eocValues[2] = {eocPulses[0].process((float)sampleTime), eocPulses[1].process((float)sampleTime)};
		for (int i = 0; i < 2; i++) {
			int readChan = isLinked() ? 0 : i;
			outputs[CV_OUTPUTS + i].value = (float)cv[readChan] * params[ATTV_PARAMS + readChan].value;
			outputs[EOC_OUTPUTS + i].value = eocValues[readChan];
		}
		
		
		lightRefreshCounter++;
		if (lightRefreshCounter > displayRefreshStepSkips) {
			lightRefreshCounter = 0;

			// Tactile lights
			if (infoStore > 0l)
				setTLightsStore(0, infoStore, (long) (storeInfoTime * sampleRate / displayRefreshStepSkips) );
			else
				setTLights(0);
			if (infoStore < 0l)
				setTLightsStore(1, infoStore * -1l, (long) (storeInfoTime * sampleRate / displayRefreshStepSkips) );
			else
				setTLights(1);
			if (infoStore != 0l) {
				if (infoStore > 0l)
					infoStore --;
				if (infoStore < 0l)
					infoStore ++;
			}
			// CV input lights
			for (int i = 0; i < 2; i++)
				lights[CVIN_LIGHTS + i * 2].value = infoCVinLight[i];
			
			for (int i = 0; i < 2; i++) {
				infoCVinLight[i] -= (infoCVinLight[i] / lightLambda) * sampleTime * displayRefreshStepSkips;
			}
		}
		
		if (isLinked()) {
			cv[1] = clamp(params[TACT_PARAMS + 1].value, 0.0f, 10.0f);
		}
		
		scheduledReset = false;
	}
	
	void setTLights(int chan) {
		int readChan = isLinked() ? 0 : chan;
		float cvValue = (float)cv[readChan];
		for (int i = 0; i < numLights; i++) {
			float level = clamp( cvValue - ((float)(i)), 0.0f, 1.0f);
			// Green diode
			lights[TACT_LIGHTS + (chan * numLights * 2) + (numLights - 1 - i) * 2 + 0].setBrightness(level);
			// Red diode
			lights[TACT_LIGHTS + (chan * numLights * 2) + (numLights - 1 - i) * 2 + 1].value = 0.0f;
		}
	}
	void setTLightsStore(int chan, long infoCount, long initInfoStore) {
		for (int i = 0; i < numLights; i++) {
			float level = (i == (int) round((float(infoCount)) / ((float)initInfoStore) * (float)(numLights - 1)) ? 1.0f : 0.0f);
			// Green diode
			lights[TACT_LIGHTS + (chan * numLights * 2) + (numLights - 1 - i) * 2 + 0].setBrightness(level);
			// Red diode
			lights[TACT_LIGHTS + (chan * numLights * 2) + (numLights - 1 - i) * 2 + 1].value = 0.0f;
		}	
	}
};


struct TactWidget : ModuleWidget {

	struct PanelThemeItem : MenuItem {
		Tact *module;
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

		Tact *module = dynamic_cast<Tact*>(this->module);
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
	
	TactWidget(Tact *module) : ModuleWidget(module) {
		// Main panel from Inkscape
        DynamicSVGPanel *panel = new DynamicSVGPanel();
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/Tact.svg")));
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/dark/Tact_dark.svg")));
        box.size = panel->box.size;
        panel->mode = &module->panelTheme;
        addChild(panel);

		// Screws
		addChild(createDynamicScrew<IMScrew>(Vec(15, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(box.size.x-30, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(15, 365), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(box.size.x-30, 365), &module->panelTheme));
		
		
		static const int rowRuler0 = 34;
		static const int colRulerPadL = 73;
		static const int colRulerPadR = 136;
		
		// Tactile touch pads
		// Right (no dynamic width, but must do first so that left will get mouse events when wider overlaps)
		addParam(createDynamicParam2<IMTactile>(Vec(colRulerPadR, rowRuler0), module, Tact::TACT_PARAMS + 1, -1.0f, 11.0f, 0.0f, nullptr, &module->paramReadRequest[1]));
		// Left (with width dependant on Link value)	
		addParam(createDynamicParam2<IMTactile>(Vec(colRulerPadL, rowRuler0), module, Tact::TACT_PARAMS + 0, -1.0f, 11.0f, 0.0f,  &module->params[Tact::LINK_PARAM].value, &module->paramReadRequest[0]));
			

			
		static const int colRulerLedL = colRulerPadL - 20;
		static const int colRulerLedR = colRulerPadR + 56;
		static const int lightsOffsetY = 19;
		static const int lightsSpacingY = 17;
				
		// Tactile lights
		for (int i = 0 ; i < Tact::numLights; i++) {
			addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(colRulerLedL, rowRuler0 + lightsOffsetY + i * lightsSpacingY), module, Tact::TACT_LIGHTS + i * 2));
			addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(colRulerLedR, rowRuler0 + lightsOffsetY + i * lightsSpacingY), module, Tact::TACT_LIGHTS + (Tact::numLights + i) * 2));
		}

		
		static const int colRulerCenter = 115;// not real center, but pos so that a jack would be centered
		static const int rowRuler2 = 265;// outputs and link
		static const int colRulerC3L = colRulerCenter - 101 - 1;
		static const int colRulerC3R = colRulerCenter + 101;
		
		// Recall CV inputs
		addInput(createDynamicPort<IMPort>(Vec(colRulerC3L, rowRuler2), Port::INPUT, module, Tact::RECALL_INPUTS + 0, &module->panelTheme));		
		addInput(createDynamicPort<IMPort>(Vec(colRulerC3R, rowRuler2), Port::INPUT, module, Tact::RECALL_INPUTS + 1, &module->panelTheme));		
		

		static const int rowRuler1d = rowRuler2 - 54;
		
		// Slide switches
		addParam(ParamWidget::create<CKSS>(Vec(colRulerC3L + hOffsetCKSS, rowRuler1d + vOffsetCKSS), module, Tact::SLIDE_PARAMS + 0, 0.0f, 1.0f, 0.0f));		
		addParam(ParamWidget::create<CKSS>(Vec(colRulerC3R + hOffsetCKSS, rowRuler1d + vOffsetCKSS), module, Tact::SLIDE_PARAMS + 1, 0.0f, 1.0f, 0.0f));		


		static const int rowRuler1c = rowRuler1d - 46;

		// Store buttons
		addParam(ParamWidget::create<TL1105>(Vec(colRulerC3L + offsetTL1105, rowRuler1c + offsetTL1105), module, Tact::STORE_PARAMS + 0, 0.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<TL1105>(Vec(colRulerC3R + offsetTL1105, rowRuler1c + offsetTL1105), module, Tact::STORE_PARAMS + 1, 0.0f, 1.0f, 0.0f));
		
		
		static const int rowRuler1b = rowRuler1c - 59;
		
		// Attv knobs
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerC3L + offsetIMSmallKnob, rowRuler1b + offsetIMSmallKnob), module, Tact::ATTV_PARAMS + 0, -1.0f, 1.0f, 1.0f, &module->panelTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerC3R + offsetIMSmallKnob, rowRuler1b + offsetIMSmallKnob), module, Tact::ATTV_PARAMS + 1, -1.0f, 1.0f, 1.0f, &module->panelTheme));

		
		static const int rowRuler1a = rowRuler1b - 59;
		
		// Rate knobs
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerC3L + offsetIMSmallKnob, rowRuler1a + offsetIMSmallKnob), module, Tact::RATE_PARAMS + 0, 0.0f, 4.0f, 0.2f, &module->panelTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerC3R + offsetIMSmallKnob, rowRuler1a + offsetIMSmallKnob), module, Tact::RATE_PARAMS + 1, 0.0f, 4.0f, 0.2f, &module->panelTheme));
		

		static const int colRulerC1L = colRulerCenter - 30 - 1;
		static const int colRulerC1R = colRulerCenter + 30; 
		static const int colRulerC2L = colRulerCenter - 65 - 1;
		static const int colRulerC2R = colRulerCenter + 65 + 1; 

		// Exp switch
		addParam(ParamWidget::create<CKSS>(Vec(colRulerCenter + hOffsetCKSS, rowRuler2 + vOffsetCKSS), module, Tact::EXP_PARAM, 0.0f, 1.0f, 0.0f));		

		// Top/bot CV Inputs
		addInput(createDynamicPort<IMPort>(Vec(colRulerC2L, rowRuler2), Port::INPUT, module, Tact::TOP_INPUTS + 0, &module->panelTheme));		
		addInput(createDynamicPort<IMPort>(Vec(colRulerC1L, rowRuler2), Port::INPUT, module, Tact::BOT_INPUTS + 0, &module->panelTheme));		
		addInput(createDynamicPort<IMPort>(Vec(colRulerC1R, rowRuler2), Port::INPUT, module, Tact::BOT_INPUTS + 1, &module->panelTheme));	
		addInput(createDynamicPort<IMPort>(Vec(colRulerC2R, rowRuler2), Port::INPUT, module, Tact::TOP_INPUTS + 1, &module->panelTheme));		

		
		static const int rowRuler3 = rowRuler2 + 54;

		// Link switch
		addParam(ParamWidget::create<CKSS>(Vec(colRulerCenter + hOffsetCKSS, rowRuler3 + vOffsetCKSS), module, Tact::LINK_PARAM, 0.0f, 1.0f, 0.0f));		

		// Outputs
		addOutput(createDynamicPort<IMPort>(Vec(colRulerCenter - 49 - 1, rowRuler3), Port::OUTPUT, module, Tact::CV_OUTPUTS + 0, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerCenter + 49, rowRuler3), Port::OUTPUT, module, Tact::CV_OUTPUTS + 1, &module->panelTheme));
		
		// EOC
		addOutput(createDynamicPort<IMPort>(Vec(colRulerCenter - 89 - 1, rowRuler3), Port::OUTPUT, module, Tact::EOC_OUTPUTS + 0, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerCenter + 89, rowRuler3), Port::OUTPUT, module, Tact::EOC_OUTPUTS + 1, &module->panelTheme));

		
		// Lights
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(colRulerCenter - 47 - 1 + offsetMediumLight, rowRuler2 - 24 + offsetMediumLight), module, Tact::CVIN_LIGHTS + 0 * 2));		
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(colRulerCenter + 47 + 1 + offsetMediumLight, rowRuler2 - 24 + offsetMediumLight), module, Tact::CVIN_LIGHTS + 1 * 2));		

	}
};

} // namespace rack_plugin_ImpromptuModular

using namespace rack_plugin_ImpromptuModular;

RACK_PLUGIN_MODEL_INIT(ImpromptuModular, Tact) {
   Model *modelTact = Model::create<Tact, TactWidget>("Impromptu Modular", "Tact", "CTRL - Tact", CONTROLLER_TAG);
   return modelTact;
}

/*CHANGE LOG

0.6.9:
move EOC outputs to main panel and remove expansion panel

0.6.8:
remove widget pointer dependancy in module for changing a tact param value
implement exponential sliding
add expansion panel with EOC outputs 

0.6.7:
created

*/
