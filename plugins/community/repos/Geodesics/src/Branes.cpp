//***********************************************************************************************
//Colliding Sample and Hold module for VCV Rack by Pierre Collard and Marc Boulé
//
//Based on code from the Fundamental plugins by Andrew Belt and graphics  
//  from the Component Library by Wes Milholen. 
//Also based on code from Joel Robichaud's Nohmad Noise module
//See ./LICENSE.txt for all licenses
//
//***********************************************************************************************


#include <dsp/filter.hpp>
#include <random>
#include "Geodesics.hpp"

namespace rack_plugin_Geodesics {

// By Joel Robichaud - Nohmad Noise module
struct NoiseGenerator {
	std::mt19937 rng;
	std::uniform_real_distribution<float> uniform;

	NoiseGenerator() : uniform(-1.0f, 1.0f) {
		rng.seed(std::random_device()());
	}

	float white() {
		return uniform(rng);
	}
};


//*****************************************************************************


// By Joel Robichaud - Nohmad Noise module
struct PinkFilter {
	float b0, b1, b2, b3, b4, b5, b6; // Coefficients
	float y; // Out

	void process(float x) {
		b0 = 0.99886f * b0 + x * 0.0555179f;
		b1 = 0.99332f * b1 + x * 0.0750759f;
		b2 = 0.96900f * b2 + x * 0.1538520f;
		b3 = 0.86650f * b3 + x * 0.3104856f;
		b4 = 0.55000f * b4 + x * 0.5329522f;
		b5 = -0.7616f * b5 - x * 0.0168980f;
		y = b0 + b1 + b2 + b3 + b4 + b5 + b6 + x * 0.5362f;
		b6 = x * 0.115926f;
	}

	float pink() {
		return y;
	}
};


//*****************************************************************************


struct Branes : Module {
	enum ParamIds {
		ENUMS(TRIG_BYPASS_PARAMS, 2),
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(IN_INPUTS, 14),
		ENUMS(TRIG_INPUTS, 2),
		ENUMS(TRIG_BYPASS_INPUTS, 2),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(OUT_OUTPUTS, 14),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(BYPASS_CV_LIGHTS, 2 * 2),// room for white-red
		ENUMS(BYPASS_TRIG_LIGHTS, 2 * 2),// room for white-red
		NUM_LIGHTS
	};
	
	
	// Constants
	// S&H are numbered 0 to 6 in BraneA from lower left to lower right
	// S&H are numbered 7 to 13 in BraneB from top right to top left
	enum NoiseId {NONE, WHITE, PINK, RED, BLUE};//use negative value for inv phase
	int noiseSources[14] = {PINK, RED, BLUE, WHITE, -BLUE, -RED, -PINK,   -PINK, -RED, -BLUE, WHITE, BLUE, RED, PINK};
	static constexpr float nullNoise = 100.0f;// when a noise has not been generated for the current step

	// Need to save, with reset
	bool trigBypass[2];
	
	// Need to save, no reset
	int panelTheme;
	
	// No need to save, with reset
	float heldOuts[14];
	
	// No need to save, no reset
	SchmittTrigger sampleTriggers[2];
	SchmittTrigger trigBypassTriggers[2];
	NoiseGenerator whiteNoise;
	PinkFilter pinkFilter;
	RCFilter redFilter;
	RCFilter blueFilter;
	float trigLights[2];

	
	Branes() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		// Need to save, no reset
		panelTheme = 0;
		
		// No need to save, no reset		
		for (int i = 0; i < 2; i++) {
			sampleTriggers[i].reset();
			trigBypassTriggers[i].reset();
			trigLights[i] = 0.0f;
		}
		redFilter.setCutoff(441.0f / engineGetSampleRate());
		blueFilter.setCutoff(44100.0f / engineGetSampleRate());
		
		onReset();
	}

	
	// widgets are not yet created when module is created 
	// even if widgets not created yet, can use params[] and should handle 0.0f value since step may call 
	//   this before widget creation anyways
	// called from the main thread if by constructor, called by engine thread if right-click initialization
	//   when called by constructor, module is created before the first step() is called
	void onReset() override {
		// Need to save, with reset
		for (int i = 0; i < 2; i++)
			trigBypass[i] = false;
		
		// No need to save, with reset
		for (int i = 0; i < 14; i++)
			heldOuts[i] = 0.0f;
	}

	
	// widgets randomized before onRandomize() is called
	// called by engine thread if right-click randomize
	void onRandomize() override {
		// Need to save, with reset
		for (int i = 0; i < 2; i++)
			trigBypass[i] = (randomu32() % 2) > 0;
		
		// No need to save, with reset
		for (int i = 0; i < 14; i++)
			heldOuts[i] = 0.0f;
	}

	
	// called by main thread
	json_t *toJson() override {
		json_t *rootJ = json_object();
		// Need to save (reset or not)

		// trigBypass
		json_object_set_new(rootJ, "trigBypass0", json_real(trigBypass[0]));
		json_object_set_new(rootJ, "trigBypass1", json_real(trigBypass[1]));

		// panelTheme
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));

		return rootJ;
	}

	
	// widgets have their fromJson() called before this fromJson() is called
	// called by main thread
	void fromJson(json_t *rootJ) override {
		// Need to save (reset or not)

		// trigBypass
		json_t *trigBypass0J = json_object_get(rootJ, "trigBypass0");
		if (trigBypass0J)
			trigBypass[0] = json_real_value(trigBypass0J);
		json_t *trigBypass1J = json_object_get(rootJ, "trigBypass1");
		if (trigBypass1J)
			trigBypass[1] = json_real_value(trigBypass1J);

		// panelTheme
		json_t *panelThemeJ = json_object_get(rootJ, "panelTheme");
		if (panelThemeJ)
			panelTheme = json_integer_value(panelThemeJ);

		// No need to save, with reset
		for (int i = 0; i < 14; i++)
			heldOuts[i] = 0.0f;
	}

	
	// Advances the module by 1 audio frame with duration 1.0 / engineGetSampleRate()
	void step() override {		
		float stepNoises[6] = {nullNoise, nullNoise, nullNoise, nullNoise, nullNoise, 0.0f};// order is whiteBase (-1 to 1), white, pink, red, blue, pink_processed (1.0f or 0.0f)
		
		// trigBypass buttons and cv inputs
		for (int i = 0; i < 2; i++) {
			if (trigBypassTriggers[i].process(params[TRIG_BYPASS_PARAMS + i].value + inputs[TRIG_BYPASS_INPUTS + i].value)) {
				trigBypass[i] = !trigBypass[i];
			}
		}

		// trig inputs
		bool trigs[2];
		bool trigInputsActive[2];
		for (int i = 0; i < 2; i++)	{	
			trigs[i] = sampleTriggers[i].process(inputs[TRIG_INPUTS + i].value);
			if (trigs[i])
				trigLights[i] = 1.0f;
			trigInputsActive[i] = trigBypass[i] ? false : inputs[TRIG_INPUTS + i].active;
		}
		
		// sample and hold outputs
		for (int sh = 0; sh < 14; sh++) {
			if (trigInputsActive[sh / 7] || (sh == 13 && trigInputsActive[0]) || (sh == 6 && trigInputsActive[1])) {// trig connected (with crosstrigger mechanism)
				if (trigs[sh / 7] || (sh == 13 && trigs[0]) || (sh == 6 && trigs[1])) {
					if (inputs[IN_INPUTS + sh].active)// if input cable
						heldOuts[sh] = inputs[IN_INPUTS + sh].value;// sample and hold input
					else {
						int noiseIndex = prepareNoise(stepNoises, sh);// sample and hold noise
						heldOuts[sh] = stepNoises[noiseIndex] * (noiseSources[sh] > 0 ? 1.0f : -1.0f);
					}
				}
			}
			else { // no trig connected
				if (inputs[IN_INPUTS + sh].active) {
					heldOuts[sh] = inputs[IN_INPUTS + sh].value;// copy of input if no trig and no input
				}
				else {
					heldOuts[sh] = 0.0f;
					if (outputs[OUT_OUTPUTS + sh].active) {
						int noiseIndex = prepareNoise(stepNoises, sh);
						heldOuts[sh] = stepNoises[noiseIndex] * (noiseSources[sh] > 0 ? 1.0f : -1.0f);
					}
				}
			}
			outputs[OUT_OUTPUTS + sh].value = heldOuts[sh];
		}
		
		// Lights
		for (int i = 0; i < 2; i++) {
			float red = trigBypass[i] ? 1.0f : 0.0f;
			float white = !trigBypass[i] ? trigLights[i] : 0.0f;
			lights[BYPASS_CV_LIGHTS + i * 2 + 0].value = white;
			lights[BYPASS_CV_LIGHTS + i * 2 + 1].value = red;
			lights[BYPASS_TRIG_LIGHTS + i * 2 + 0].value = white;
			lights[BYPASS_TRIG_LIGHTS + i * 2 + 1].value = red;
			trigLights[i] -= (trigLights[i] / lightLambda) * (float)engineGetSampleTime();
		}
		
	}// step()
	
	int prepareNoise(float* stepNoises, int sh) {
		int noiseIndex = abs( noiseSources[sh] );
		if (stepNoises[noiseIndex] == nullNoise) {
			if (stepNoises[0] == nullNoise)
				stepNoises[0] = whiteNoise.white();
			if ((noiseIndex == PINK || noiseIndex == BLUE) && stepNoises[5] == 0.0f) {
				pinkFilter.process(stepNoises[0]);
				stepNoises[5] = 1.0f;
			}
			switch (noiseIndex) {
				// most of the code in here is from Joel Robichaud - Nohmad Noise module
				case (PINK) :
					stepNoises[noiseIndex] = 5.0f * clamp(0.18f * pinkFilter.pink(), -1.0f, 1.0f);
				break;
				case (RED) :
					redFilter.process(stepNoises[0]);
					stepNoises[noiseIndex] = 5.0f * clamp(7.8f * redFilter.lowpass(), -1.0f, 1.0f);
				break;
				case (BLUE) :
					blueFilter.process(pinkFilter.pink());
					stepNoises[noiseIndex] = 5.0f * clamp(0.64f * blueFilter.highpass(), -1.0f, 1.0f);
				break;
				default ://(WHITE)
					stepNoises[noiseIndex] = 5.0f * stepNoises[0];
				break;
			}
		}
		return noiseIndex;
	}
};


struct BranesWidget : ModuleWidget {

	struct PanelThemeItem : MenuItem {
		Branes *module;
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

		Branes *module = dynamic_cast<Branes*>(this->module);
		assert(module);

		MenuLabel *themeLabel = new MenuLabel();
		themeLabel->text = "Panel Theme";
		menu->addChild(themeLabel);

		PanelThemeItem *lightItem = new PanelThemeItem();
		lightItem->text = lightPanelID;// Geodesics.hpp
		lightItem->module = module;
		lightItem->theme = 0;
		menu->addChild(lightItem);

		PanelThemeItem *darkItem = new PanelThemeItem();
		darkItem->text = darkPanelID;// Geodesics.hpp
		darkItem->module = module;
		darkItem->theme = 1;
		//menu->addChild(darkItem);

		return menu;
	}	
	
	BranesWidget(Branes *module) : ModuleWidget(module) {
		// Main panel from Inkscape
        DynamicSVGPanel *panel = new DynamicSVGPanel();
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/BranesBG-01.svg")));
        //panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/BranesBG-02.svg")));// no dark pannel for now
        box.size = panel->box.size;
        panel->mode = &module->panelTheme;
        addChild(panel);

		// Screws 
		// part of svg panel, no code required
		
		float colRulerCenter = box.size.x / 2.0f;
		static constexpr float rowRulerHoldA = 119.5;
		static constexpr float rowRulerHoldB = 248.5f;
		static constexpr float radiusIn = 35.0f;
		static constexpr float radiusOut = 64.0f;
		static constexpr float offsetIn = 25.0f;
		static constexpr float offsetOut = 46.0f;
		
		
		// BraneA trig intput
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter, rowRulerHoldA), Port::INPUT, module, Branes::TRIG_INPUTS + 0, &module->panelTheme));
		
		// BraneA inputs
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - offsetIn, rowRulerHoldA + offsetIn), Port::INPUT, module, Branes::IN_INPUTS + 0, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - radiusIn, rowRulerHoldA), Port::INPUT, module, Branes::IN_INPUTS + 1, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - offsetIn, rowRulerHoldA - offsetIn), Port::INPUT, module, Branes::IN_INPUTS + 2, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter, rowRulerHoldA - radiusIn), Port::INPUT, module, Branes::IN_INPUTS + 3, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + offsetIn, rowRulerHoldA - offsetIn), Port::INPUT, module, Branes::IN_INPUTS + 4, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + radiusIn, rowRulerHoldA), Port::INPUT, module, Branes::IN_INPUTS + 5, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + offsetIn, rowRulerHoldA + offsetIn), Port::INPUT, module, Branes::IN_INPUTS + 6, &module->panelTheme));

		// BraneA outputs
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter - offsetOut, rowRulerHoldA + offsetOut), Port::OUTPUT, module, Branes::OUT_OUTPUTS + 0, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter - radiusOut, rowRulerHoldA), Port::OUTPUT, module, Branes::OUT_OUTPUTS + 1, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter - offsetOut, rowRulerHoldA - offsetOut), Port::OUTPUT, module, Branes::OUT_OUTPUTS + 2, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter, rowRulerHoldA - radiusOut), Port::OUTPUT, module, Branes::OUT_OUTPUTS + 3, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter + offsetOut, rowRulerHoldA - offsetOut), Port::OUTPUT, module, Branes::OUT_OUTPUTS + 4, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter + radiusOut, rowRulerHoldA), Port::OUTPUT, module, Branes::OUT_OUTPUTS + 5, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter + offsetOut, rowRulerHoldA + offsetOut), Port::OUTPUT, module, Branes::OUT_OUTPUTS + 6, &module->panelTheme));
		
		
		// BraneB trig intput
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter, rowRulerHoldB), Port::INPUT, module, Branes::TRIG_INPUTS + 1, &module->panelTheme));
		
		// BraneB inputs
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + offsetIn, rowRulerHoldB - offsetIn), Port::INPUT, module, Branes::IN_INPUTS + 7, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + radiusIn, rowRulerHoldB), Port::INPUT, module, Branes::IN_INPUTS + 8, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + offsetIn, rowRulerHoldB + offsetIn), Port::INPUT, module, Branes::IN_INPUTS + 9, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter, rowRulerHoldB + radiusIn), Port::INPUT, module, Branes::IN_INPUTS + 10, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - offsetIn, rowRulerHoldB + offsetIn), Port::INPUT, module, Branes::IN_INPUTS + 11, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - radiusIn, rowRulerHoldB), Port::INPUT, module, Branes::IN_INPUTS + 12, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - offsetIn, rowRulerHoldB - offsetIn), Port::INPUT, module, Branes::IN_INPUTS + 13, &module->panelTheme));


		// BraneB outputs
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter + offsetOut, rowRulerHoldB - offsetOut), Port::OUTPUT, module, Branes::OUT_OUTPUTS + 7, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter + radiusOut, rowRulerHoldB), Port::OUTPUT, module, Branes::OUT_OUTPUTS + 8, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter + offsetOut, rowRulerHoldB + offsetOut), Port::OUTPUT, module, Branes::OUT_OUTPUTS + 9, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter, rowRulerHoldB + radiusOut), Port::OUTPUT, module, Branes::OUT_OUTPUTS + 10, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter - offsetOut, rowRulerHoldB + offsetOut), Port::OUTPUT, module, Branes::OUT_OUTPUTS + 11, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter - radiusOut, rowRulerHoldB), Port::OUTPUT, module, Branes::OUT_OUTPUTS + 12, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter - offsetOut, rowRulerHoldB - offsetOut), Port::OUTPUT, module, Branes::OUT_OUTPUTS + 13, &module->panelTheme));
		
		
		static constexpr float rowRulerBypass = 345.5f;
		
		// Trigger bypass
		// buttons
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter - 32.0f, rowRulerBypass), module, Branes::TRIG_BYPASS_PARAMS + 0, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter + 32.0f, rowRulerBypass), module, Branes::TRIG_BYPASS_PARAMS + 1, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// cv inputs
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - 65.0f, rowRulerBypass), Port::INPUT, module, Branes::TRIG_BYPASS_INPUTS + 0, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + 65.0f, rowRulerBypass), Port::INPUT, module, Branes::TRIG_BYPASS_INPUTS + 1, &module->panelTheme));
		// LEDs bottom
		addChild(createLightCentered<SmallLight<GeoWhiteRedLight>>(Vec(colRulerCenter - 46.5f, rowRulerBypass), module, Branes::BYPASS_CV_LIGHTS + 0 * 2));
		addChild(createLightCentered<SmallLight<GeoWhiteRedLight>>(Vec(colRulerCenter + 46.5f, rowRulerBypass), module, Branes::BYPASS_CV_LIGHTS + 1 * 2));
		// LEDs top
		addChild(createLightCentered<SmallLight<GeoWhiteRedLight>>(Vec(colRulerCenter + 5.5f, rowRulerHoldA + 19.5f), module, Branes::BYPASS_TRIG_LIGHTS + 0 * 2));
		addChild(createLightCentered<SmallLight<GeoWhiteRedLight>>(Vec(colRulerCenter - 5.5f, rowRulerHoldB - 19.5f), module, Branes::BYPASS_TRIG_LIGHTS + 1 * 2));

	}
};

} // namespace rack_plugin_Geodesics

using namespace rack_plugin_Geodesics;

RACK_PLUGIN_MODEL_INIT(Geodesics, Branes) {
   Model *modelBranes = Model::create<Branes, BranesWidget>("Geodesics", "Branes", "Branes", SAMPLE_AND_HOLD_TAG);
   return modelBranes;
}

/*CHANGE LOG

0.6.0:
created

*/
