//***********************************************************************************************
//Blank-Panel Logo for VCV Rack by Pierre Collard and Marc Boulé
//
//Based on code from the Fundamental plugins by Andrew Belt 
//See ./LICENSE.txt for all licenses
//***********************************************************************************************

#include "Geodesics.hpp"

namespace rack_plugin_Geodesics {

// From Fundamental LFO.cpp
struct LowFrequencyOscillator {
	float phase = 0.0f;
	float freq = 1.0f;

	LowFrequencyOscillator() {}
	void setPitch(float pitch) {
		pitch = fminf(pitch, 8.0f);
		freq = powf(2.0f, pitch);
	}
	void step(float dt) {
		float deltaPhase = fminf(freq * dt, 0.5f);
		phase += deltaPhase;
		if (phase >= 1.0f)
			phase -= 1.0f;
	}
	float sqr() {
		return (phase < 0.5f) ? 2.0f : 0.0f;
	}
};


//*****************************************************************************


struct BlankLogo : Module {
	enum ParamIds {
		CLK_FREQ_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	
	
	int panelTheme = 0;
	float clkValue;
	int stepIndex;
	float song[5] = {7.0f/12.0f, 9.0f/12.0f, 5.0f/12.0f, 5.0f/12.0f - 1.0f, 0.0f/12.0f};

	LowFrequencyOscillator oscillatorClk;
	SchmittTrigger clkTrigger;
	
	
	BlankLogo() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		clkTrigger.reset();
		onReset();
	}

	void onReset() override {	
		clkValue = 0.0f;
		stepIndex = 0;
	}

	void onRandomize() override {
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();

		// panelTheme
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// panelTheme
		json_t *panelThemeJ = json_object_get(rootJ, "panelTheme");
		if (panelThemeJ)
			panelTheme = json_integer_value(panelThemeJ);
	}

	
	// Advances the module by 1 audio frame with duration 1.0 / engineGetSampleRate()
	void step() override {	
		if (outputs[OUT_OUTPUT].active) {
			// CLK
			oscillatorClk.setPitch(params[CLK_FREQ_PARAM].value);
			oscillatorClk.step(engineGetSampleTime());
			float clkValue = oscillatorClk.sqr();	
			
			if (clkTrigger.process(clkValue)) {
				stepIndex++;
				if (stepIndex >= 5)
					stepIndex = 0;
				outputs[OUT_OUTPUT].value = song[stepIndex];
			}
		}
	}
};


struct BlankLogoWidget : ModuleWidget {

	struct PanelThemeItem : MenuItem {
		BlankLogo *module;
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

		BlankLogo *module = dynamic_cast<BlankLogo*>(this->module);
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


	BlankLogoWidget(BlankLogo *module) : ModuleWidget(module) {
		// Main panel from Inkscape
        DynamicSVGPanel *panel = new DynamicSVGPanel();
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/BlankLogoBG-01.svg")));
        //panel->addPanel(SVG::load(assetPlugin(plugin, "res/dark/BlankLogo-02.svg")));// no dark pannel for now
        box.size = panel->box.size;
        panel->mode = &module->panelTheme;
        addChild(panel);

		// Screws
		// part of svg panel, no code required
		
		addParam(createParamCentered<BlankCKnob>(Vec(29.5f,74.2f), module, BlankLogo::CLK_FREQ_PARAM, -2.0f, 4.0f, 1.0f));// 120 BMP when default value
		addOutput(createOutputCentered<BlankPort>(Vec(29.5f,187.5f), module, BlankLogo::OUT_OUTPUT));
		
	}
};

} // namespace rack_plugin_Geodesics

using namespace rack_plugin_Geodesics;

RACK_PLUGIN_MODEL_INIT(Geodesics, BlankLogo) {
   Model *modelBlankLogo = Model::create<BlankLogo, BlankLogoWidget>("Geodesics", "Blank-Panel Logo", "MISC - Blank-Panel Logo", BLANK_TAG);
   return modelBlankLogo;
}
