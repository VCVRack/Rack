//***********************************************************************************************
//Engineering Test 1
//
//Based on code from the Fundamental and AudibleInstruments plugins by Andrew Belt 
//and graphics from the Component Library by Wes Milholen 
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//
//Module concept by Marc Boulé
//***********************************************************************************************


#include "ImpromptuModular.hpp"
#include "dsp/digital.hpp"


struct EngTest1 : Module {
	enum ParamIds {
		ENUMS(NOTETYPE_PARAMS, 5),
		ENUMS(STEP_PARAMS, 24),
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(NOTETYPE_LIGHTS, 5),
		ENUMS(STEP_LIGHTS, 24 * 2),// room for GreenRed
		NUM_LIGHTS
	};
	
	
	// Need to save
	int panelTheme;
	int noteType;// 0 is full note, 1 is half note, 2 is quarter note, etc...
	uint8_t notes[24];
	
	// No need to save


	SchmittTrigger noteTypeTriggers[5];
	SchmittTrigger stepTriggers[24];

	
	EngTest1() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		panelTheme = 0;
		for (int i = 0; i < 5; i++)
			noteTypeTriggers[i].reset();
		for (int i = 0; i < 24; i++)
			stepTriggers[i].reset();

		onReset();
	}

	
	// widgets are not yet created when module is created (and when onReset() is called by constructor)
	// onReset() is also called when right-click initialization of module
	void onReset() override {
		noteType = 2;
		for (int i = 0; i < 24; i++) {
			notes[i] = (i / 6) * 6;
		}
	}

	
	// widgets randomized before onRandomize() is called
	void onRandomize() override {

	}


	json_t *toJson() override {
		json_t *rootJ = json_object();
	
		return rootJ;
	}

	
	// widgets loaded before this fromJson() is called
	void fromJson(json_t *rootJ) override {

	}

	
	// Advances the module by 1 audio frame with duration 1.0 / engineGetSampleRate()
	void step() override {

		// Step buttons
		for (int i = 0; i < 24; i++) {
			if (stepTriggers[i].process(params[STEP_PARAMS + i].value)) {
				int count = 3 * (1 << (3 - noteType));
				int j;
				for (j = 0; j < count; j++) {
					if ((i + j) >= 24)
						break;
					notes[i + j] = i;
				}
				if ((i + j) < 24) {
					int oldVal = notes[i + j];
					int newVal = i + j;
					for (; j < 24; j++) {// j < 24 is safety
						if ((i + j) >= 24)
							break;
						if (notes[i + j] == oldVal)
							notes[i + j] = newVal;
					}
				}
			}
		}
		
		// Note type buttons
		for (int i = 0; i < 5; i++) {
			if (noteTypeTriggers[i].process(params[NOTETYPE_PARAMS + i].value)) {
				noteType = i;
			}
		}
	

		// Step lights
		bool isGreen = true;
		for (int i = 0; i < 24; i++) {
			if (i > 0 && notes[i] != notes[i-1])
				isGreen = !isGreen;
			setGreenRed(STEP_LIGHTS + i * 2, isGreen ? 1.0f : 0.0f, !isGreen ? 1.0f : 0.0f);
		}
	
		// Note type lights
		for (int i = 0; i < 5; i++)
			lights[NOTETYPE_LIGHTS + i].value = (i == noteType ? 1.0f : 0.0f);
	
	}// step()
	
	
	void setGreenRed(int id, float green, float red) {
		lights[id + 0].value = green;
		lights[id + 1].value = red;
	}

};// EngTest1 : module

struct EngTest1Widget : ModuleWidget {

	EngTest1Widget(EngTest1 *module) : ModuleWidget(module) {		
		// Main panel from Inkscape
        DynamicSVGPanel* panel = new DynamicSVGPanel();
        panel->mode = &module->panelTheme;
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/EngTest1.svg")));
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/dark/EngTest1_dark.svg")));
        box.size = panel->box.size;
        addChild(panel);		
		
		// Screws
		addChild(createDynamicScrew<IMScrew>(Vec(15, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(15, 365), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(panel->box.size.x-30, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(panel->box.size.x-30, 365), &module->panelTheme));
		
		
		// ****** Top portion (2 switches and LED button array ******
		
		static const int rowRuler0 = 65;
		static const int colRulerSteps = 15;
		static const int spacingSteps = 20;
		//static const int spacingSteps4 = 4;
		
		
		// Step LED buttons
		int posX = colRulerSteps;
		for (int x = 0; x < 24; x++) {
			addParam(ParamWidget::create<LEDButton>(Vec(posX, rowRuler0 + 8 - 4.4f), module, EngTest1::STEP_PARAMS + x, 0.0f, 1.0f, 0.0f));
			addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(posX + 4.4f, rowRuler0 + 8), module, EngTest1::STEP_LIGHTS + x * 2));
			posX += spacingSteps;
			//if ((x + 1) % 4 == 0)
				//posX += spacingSteps4;
		}

			
		// Note type buttons
		static const int rowRuler1 = 160;
		static const int posLEDvsButton = 25;
		static const int spacingButtons = 40;
		static const int columnRulerMB1 = colRulerSteps + 10;
		for (int x = 0; x < 5; x++) {
			addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(columnRulerMB1 + x * spacingButtons + offsetMediumLight, rowRuler1 + offsetMediumLight - posLEDvsButton), module, EngTest1::NOTETYPE_LIGHTS + x));		
			addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerMB1 + x * spacingButtons + offsetCKD6b, rowRuler1 + offsetCKD6b), module, EngTest1::NOTETYPE_PARAMS + x, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		}
		
	}
};


Model *modelEngTest1 = Model::create<EngTest1, EngTest1Widget>("Impromptu Modular", "Eng-Test-1", "??? - Eng-Test-1", BLANK_TAG);

/*CHANGE LOG

0.6.10:
created 

*/