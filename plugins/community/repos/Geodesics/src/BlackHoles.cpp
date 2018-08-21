//***********************************************************************************************
//Gravitational Voltage Controled Amplifiers module for VCV Rack by Pierre Collard and Marc Boulé
//
//Based on code from the Fundamental plugins by Andrew Belt and graphics  
//  from the Component Library by Wes Milholen. 
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//
//***********************************************************************************************


#include "Geodesics.hpp"

namespace rack_plugin_Geodesics {

struct BlackHoles : Module {
	enum ParamIds {
		ENUMS(LEVEL_PARAMS, 8),// -1.0f to 1.0f knob, set to default (0.0f) when using CV input
		ENUMS(EXP_PARAMS, 2),// push-button
		WORMHOLE_PARAM,
		ENUMS(CVLEVEL_PARAMS, 2),// push-button
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(IN_INPUTS, 8),// -10 to 10 V 
		ENUMS(LEVELCV_INPUTS, 8),// 0 to 10V CV or -5 to 5V depeding on cvMode
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(OUT_OUTPUTS, 8),// input * [-1;1] when input connected, else [-10;10] CV when input unconnected
		ENUMS(BLACKHOLE_OUTPUTS, 2),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(EXP_LIGHTS, 2),
		ENUMS(WORMHOLE_LIGHT, 2),// room for WhiteRed
		ENUMS(CVALEVEL_LIGHTS, 2),// White, but two lights (light 0 is cvMode bit = 0, light 1 is cvMode bit = 1)
		ENUMS(CVBLEVEL_LIGHTS, 2),// White, but two lights
		NUM_LIGHTS
	};
	
	
	// Constants
	static constexpr float expBase = 50.0f;

	// Need to save, with reset
	bool isExponential[2];
	bool wormhole;
	int cvMode;// 0 is -5v to 5v, 1 is -10v to 10v; bit 0 is upper BH, bit 1 is lower BH
	
	// Need to save, no reset
	int panelTheme;
	
	// No need to save, with reset
	// none
	
	// No need to save, no reset
	SchmittTrigger expTriggers[2];
	SchmittTrigger cvLevelTriggers[2];
	SchmittTrigger wormholeTrigger;

	
	BlackHoles() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		// Need to save, no reset
		panelTheme = 0;

		// No need to save, no reset		
		expTriggers[0].reset();
		expTriggers[1].reset();
		
		onReset();
	}

	
	// widgets are not yet created when module is created 
	// even if widgets not created yet, can use params[] and should handle 0.0f value since step may call 
	//   this before widget creation anyways
	// called from the main thread if by constructor, called by engine thread if right-click initialization
	//   when called by constructor, module is created before the first step() is called
	void onReset() override {
		// Need to save, with reset
		cvMode = 0x3;
		isExponential[0] = false;
		isExponential[1] = false;
		wormhole = true;
		
		// No need to save, with reset
		// none
	}

	
	// widgets randomized before onRandomize() is called
	// called by engine thread if right-click randomize
	void onRandomize() override {
		// Need to save, with reset
		for (int i = 0; i < 2; i++) {
			isExponential[i] = (randomu32() % 2) > 0;
		}
		wormhole = (randomu32() % 2) > 0;
		
		// No need to save, with reset
		// none
	}

	
	// called by main thread
	json_t *toJson() override {
		json_t *rootJ = json_object();
		// Need to save (reset or not)

		// isExponential
		json_object_set_new(rootJ, "isExponential0", json_real(isExponential[0]));
		json_object_set_new(rootJ, "isExponential1", json_real(isExponential[1]));
		
		// wormhole
		json_object_set_new(rootJ, "wormhole", json_boolean(wormhole));

		// panelTheme
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));

		// cvMode
		json_object_set_new(rootJ, "cvMode", json_integer(cvMode));

		return rootJ;
	}

	
	// widgets have their fromJson() called before this fromJson() is called
	// called by main thread
	void fromJson(json_t *rootJ) override {
		// Need to save (reset or not)

		// isExponential
		json_t *isExponential0J = json_object_get(rootJ, "isExponential0");
		if (isExponential0J)
			isExponential[0] = json_real_value(isExponential0J);
		json_t *isExponential1J = json_object_get(rootJ, "isExponential1");
		if (isExponential1J)
			isExponential[1] = json_real_value(isExponential1J);

		// wormhole
		json_t *wormholeJ = json_object_get(rootJ, "wormhole");
		if (wormholeJ)
			wormhole = json_is_true(wormholeJ);

		// panelTheme
		json_t *panelThemeJ = json_object_get(rootJ, "panelTheme");
		if (panelThemeJ)
			panelTheme = json_integer_value(panelThemeJ);

		// cvMode
		json_t *cvModeJ = json_object_get(rootJ, "cvMode");
		if (cvModeJ)
			cvMode = json_integer_value(cvModeJ);

		// No need to save, with reset
		// none
	}

	
	// Advances the module by 1 audio frame with duration 1.0 / engineGetSampleRate()
	void step() override {		
		// Exponential buttons
		for (int i = 0; i < 2; i++)
			if (expTriggers[i].process(params[EXP_PARAMS + i].value)) {
				isExponential[i] = !isExponential[i];
		}
		
		// Wormhole buttons
		if (wormholeTrigger.process(params[WORMHOLE_PARAM].value)) {
			wormhole = ! wormhole;
		}

		// CV Level buttons
		for (int i = 0; i < 2; i++) {
			if (cvLevelTriggers[i].process(params[CVLEVEL_PARAMS + i].value))
				cvMode ^= (0x1 << i);
		}
		
		
		// BlackHole 0 all outputs
		float blackHole0 = 0.0f;
		float inputs0[4] = {10.0f, 10.0f, 10.0f, 10.0f};// default to generate CV when no input connected
		for (int i = 0; i < 4; i++) 
			if (inputs[IN_INPUTS + i].active)
				inputs0[i] = inputs[IN_INPUTS + i].value;
		for (int i = 0; i < 4; i++) {
			float chanVal = calcChannel(inputs0[i], params[LEVEL_PARAMS + i], inputs[LEVELCV_INPUTS + i], isExponential[0], cvMode & 0x1);
			outputs[OUT_OUTPUTS + i].value = chanVal;
			blackHole0 += chanVal;
		}
		outputs[BLACKHOLE_OUTPUTS + 0].value = clamp(blackHole0, -10.0f, 10.0f);
			
			
		// BlackHole 1 all outputs
		float blackHole1 = 0.0f;
		float inputs1[4] = {10.0f, 10.0f, 10.0f, 10.0f};// default to generate CV when no input connected
		bool allUnconnected = true;
		for (int i = 0; i < 4; i++) 
			if (inputs[IN_INPUTS + i + 4].active) {
				inputs1[i] = inputs[IN_INPUTS + i + 4].value;
				allUnconnected = false;
			}
		if (allUnconnected && wormhole)
			for (int i = 0; i < 4; i++)
				inputs1[i] = blackHole0;
		for (int i = 0; i < 4; i++) {
			float chanVal = calcChannel(inputs1[i], params[LEVEL_PARAMS + i + 4], inputs[LEVELCV_INPUTS + i + 4], isExponential[1], cvMode >> 1);
			outputs[OUT_OUTPUTS + i + 4].value = chanVal;
			blackHole1 += chanVal;
		}
		outputs[BLACKHOLE_OUTPUTS + 1].value = clamp(blackHole1, -10.0f, 10.0f);

		// Wormhole light
		lights[WORMHOLE_LIGHT + 0].value = ((wormhole && allUnconnected) ? 1.0f : 0.0f);
		lights[WORMHOLE_LIGHT + 1].value = ((wormhole && !allUnconnected) ? 1.0f : 0.0f);
				
		// isExponential lights
		for (int i = 0; i < 2; i++)
			lights[EXP_LIGHTS + i].value = isExponential[i] ? 1.0f : 0.0f;
		
		// CV Level lights
		lights[CVALEVEL_LIGHTS + 0].value = (cvMode & 0x1) == 0 ? 1.0f : 0.0f;
		lights[CVALEVEL_LIGHTS + 1].value = 1.0f - lights[CVALEVEL_LIGHTS + 0].value;
		lights[CVBLEVEL_LIGHTS + 0].value = (cvMode & 0x2) == 0 ? 1.0f : 0.0f;
		lights[CVBLEVEL_LIGHTS + 1].value = 1.0f - lights[CVBLEVEL_LIGHTS + 0].value;
	
	}// step()
	
	float calcChannel(float in, Param &level, Input &levelCV, bool isExp, int cvMode) {
		float levCv = levelCV.active ? (levelCV.value / (cvMode != 0 ? 10.0f : 5.0f)) : 0.0f;
		float lev = clamp(level.value + levCv, -1.0f, 1.0f);
		if (isExp) {
			float newlev = rescale(powf(expBase, fabs(lev)), 1.0f, expBase, 0.0f, 1.0f);
			if (lev < 0.0f)
				newlev *= -1.0f;
			lev = newlev;
		}
		float ret = lev * in;
		return ret;
	}	
};


struct BlackHolesWidget : ModuleWidget {

	struct PanelThemeItem : MenuItem {
		BlackHoles *module;
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

		BlackHoles *module = dynamic_cast<BlackHoles*>(this->module);
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
	
	BlackHolesWidget(BlackHoles *module) : ModuleWidget(module) {
		// Main panel from Inkscape
        DynamicSVGPanel *panel = new DynamicSVGPanel();
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/BlackHolesBG-01.svg")));
        //panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/BlackHolesBG-02.svg")));// no dark pannel for now
        box.size = panel->box.size;
        panel->mode = &module->panelTheme;
        addChild(panel);

		// Screws 
		// part of svg panel, no code required
		
		float colRulerCenter = box.size.x / 2.0f;
		static constexpr float rowRulerBlack0 = 108.5f;
		static constexpr float rowRulerBlack1 = 272.5f;
		static constexpr float radiusIn = 30.0f;
		static constexpr float radiusOut = 61.0f;
		static constexpr float offsetL = 53.0f;
		static constexpr float offsetS = 30.0f;
		
		
		// BlackHole0 knobs
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter, rowRulerBlack0 - radiusOut), module, BlackHoles::LEVEL_PARAMS + 0, -1.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnobRight>(Vec(colRulerCenter + radiusOut, rowRulerBlack0), module, BlackHoles::LEVEL_PARAMS + 1, -1.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnobBottom>(Vec(colRulerCenter, rowRulerBlack0 + radiusOut), module, BlackHoles::LEVEL_PARAMS + 2, -1.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnobLeft>(Vec(colRulerCenter - radiusOut, rowRulerBlack0), module, BlackHoles::LEVEL_PARAMS + 3, -1.0f, 1.0f, 0.0f, &module->panelTheme));

		// BlackHole0 level CV inputs
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter, rowRulerBlack0 - radiusIn), Port::INPUT, module, BlackHoles::LEVELCV_INPUTS + 0, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + radiusIn, rowRulerBlack0), Port::INPUT, module, BlackHoles::LEVELCV_INPUTS + 1, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter, rowRulerBlack0 + radiusIn), Port::INPUT, module, BlackHoles::LEVELCV_INPUTS + 2, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - radiusIn, rowRulerBlack0), Port::INPUT, module, BlackHoles::LEVELCV_INPUTS + 3, &module->panelTheme));

		// BlackHole0 inputs
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - offsetS, rowRulerBlack0 - offsetL), Port::INPUT, module, BlackHoles::IN_INPUTS + 0, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + offsetL, rowRulerBlack0 - offsetS), Port::INPUT, module, BlackHoles::IN_INPUTS + 1, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + offsetS, rowRulerBlack0 + offsetL), Port::INPUT, module, BlackHoles::IN_INPUTS + 2, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - offsetL, rowRulerBlack0 + offsetS), Port::INPUT, module, BlackHoles::IN_INPUTS + 3, &module->panelTheme));
		
		// BlackHole0 outputs
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter + offsetS, rowRulerBlack0 - offsetL), Port::OUTPUT, module, BlackHoles::OUT_OUTPUTS + 0, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter + offsetL, rowRulerBlack0 + offsetS), Port::OUTPUT, module, BlackHoles::OUT_OUTPUTS + 1, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter - offsetS, rowRulerBlack0 + offsetL), Port::OUTPUT, module, BlackHoles::OUT_OUTPUTS + 2, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter - offsetL, rowRulerBlack0 - offsetS), Port::OUTPUT, module, BlackHoles::OUT_OUTPUTS + 3, &module->panelTheme));
		// BlackHole0 center output
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter, rowRulerBlack0), Port::OUTPUT, module, BlackHoles::BLACKHOLE_OUTPUTS + 0, &module->panelTheme));

				
		// BlackHole1 knobs
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter, rowRulerBlack1 - radiusOut), module, BlackHoles::LEVEL_PARAMS + 4, -1.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnobRight>(Vec(colRulerCenter + radiusOut, rowRulerBlack1), module, BlackHoles::LEVEL_PARAMS + 5, -1.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnobBottom>(Vec(colRulerCenter, rowRulerBlack1 + radiusOut), module, BlackHoles::LEVEL_PARAMS + 6, -1.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnobLeft>(Vec(colRulerCenter - radiusOut, rowRulerBlack1), module, BlackHoles::LEVEL_PARAMS + 7, -1.0f, 1.0f, 0.0f, &module->panelTheme));
		
		// BlackHole1 level CV inputs
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter, rowRulerBlack1 - radiusIn), Port::INPUT, module, BlackHoles::LEVELCV_INPUTS + 4, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + radiusIn, rowRulerBlack1), Port::INPUT, module, BlackHoles::LEVELCV_INPUTS + 5, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter, rowRulerBlack1 + radiusIn), Port::INPUT, module, BlackHoles::LEVELCV_INPUTS + 6, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - radiusIn, rowRulerBlack1), Port::INPUT, module, BlackHoles::LEVELCV_INPUTS + 7, &module->panelTheme));

		// BlackHole1 inputs
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - offsetS, rowRulerBlack1 - offsetL), Port::INPUT, module, BlackHoles::IN_INPUTS + 4, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + offsetL, rowRulerBlack1 - offsetS), Port::INPUT, module, BlackHoles::IN_INPUTS + 5, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + offsetS, rowRulerBlack1 + offsetL), Port::INPUT, module, BlackHoles::IN_INPUTS + 6, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - offsetL, rowRulerBlack1 + offsetS), Port::INPUT, module, BlackHoles::IN_INPUTS + 7, &module->panelTheme));
		
		// BlackHole1 outputs
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter + offsetS, rowRulerBlack1 - offsetL), Port::OUTPUT, module, BlackHoles::OUT_OUTPUTS + 4, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter + offsetL, rowRulerBlack1 + offsetS), Port::OUTPUT, module, BlackHoles::OUT_OUTPUTS + 5, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter - offsetS, rowRulerBlack1 + offsetL), Port::OUTPUT, module, BlackHoles::OUT_OUTPUTS + 6, &module->panelTheme));
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter - offsetL, rowRulerBlack1 - offsetS), Port::OUTPUT, module, BlackHoles::OUT_OUTPUTS + 7, &module->panelTheme));
		// BlackHole1 center output
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter, rowRulerBlack1), Port::OUTPUT, module, BlackHoles::BLACKHOLE_OUTPUTS + 1, &module->panelTheme));
		
		
		static constexpr float offsetButtonsX = 62.0f;
		static constexpr float offsetButtonsY = 64.0f;
		static constexpr float offsetLedVsBut = 9.0f;
		static constexpr float offsetLedVsButS = 5.0f;// small
		static constexpr float offsetLedVsButL = 12.0f;// large
		
		
		// BlackHole0 Exp button and light
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter - offsetButtonsX, rowRulerBlack0 + offsetButtonsY), module, BlackHoles::EXP_PARAMS + 0, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(colRulerCenter - offsetButtonsX + offsetLedVsBut, rowRulerBlack0 + offsetButtonsY - offsetLedVsBut - 1.0f), module, BlackHoles::EXP_LIGHTS + 0));
		
		// BlackHole1 Exp button and light
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter - offsetButtonsX, rowRulerBlack1 + offsetButtonsY), module, BlackHoles::EXP_PARAMS + 1, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(colRulerCenter - offsetButtonsX + offsetLedVsBut, rowRulerBlack1 + offsetButtonsY - offsetLedVsBut -1.0f), module, BlackHoles::EXP_LIGHTS + 1));
		
		// Wormhole button and light
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter - offsetButtonsX, rowRulerBlack1 - offsetButtonsY), module, BlackHoles::WORMHOLE_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoWhiteRedLight>>(Vec(colRulerCenter - offsetButtonsX + offsetLedVsBut, rowRulerBlack1 - offsetButtonsY + offsetLedVsBut), module, BlackHoles::WORMHOLE_LIGHT));
		
		
		// CV Level A button and light
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter + offsetButtonsX, rowRulerBlack0 + offsetButtonsY), module, BlackHoles::CVLEVEL_PARAMS + 0, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(colRulerCenter + offsetButtonsX + offsetLedVsButL, rowRulerBlack0 + offsetButtonsY + offsetLedVsButS), module, BlackHoles::CVALEVEL_LIGHTS + 0));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(colRulerCenter + offsetButtonsX + offsetLedVsButS, rowRulerBlack0 + offsetButtonsY + offsetLedVsButL), module, BlackHoles::CVALEVEL_LIGHTS + 1));
		
		// CV Level B button and light
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter + offsetButtonsX, rowRulerBlack1 + offsetButtonsY), module, BlackHoles::CVLEVEL_PARAMS + 1, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(colRulerCenter + offsetButtonsX + offsetLedVsButL, rowRulerBlack1 + offsetButtonsY + offsetLedVsButS), module, BlackHoles::CVBLEVEL_LIGHTS + 0));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(colRulerCenter + offsetButtonsX + offsetLedVsButS, rowRulerBlack1 + offsetButtonsY + offsetLedVsButL), module, BlackHoles::CVBLEVEL_LIGHTS + 1));


	}
};

} // namespace rack_plugin_Geodesics

using namespace rack_plugin_Geodesics;

RACK_PLUGIN_MODEL_INIT(Geodesics, BlackHoles) {
   Model *modelBlackHoles = Model::create<BlackHoles, BlackHolesWidget>("Geodesics", "BlackHoles", "BlackHoles", AMPLIFIER_TAG);
   return modelBlackHoles;
}

/*CHANGE LOG

0.6.1:
add CV level modes buttons and lights
change CV level behavior

0.6.0:
created

*/
