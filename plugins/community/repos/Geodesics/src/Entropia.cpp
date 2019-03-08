//***********************************************************************************************
//Thermodynamic Evolving Sequencer module for VCV Rack by Pierre Collard and Marc Boulé
//
//Based on code from the Fundamental plugins by Andrew Belt and graphics  
//  from the Component Library by Wes Milholen. 
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//
//***********************************************************************************************


#include "Geodesics.hpp"

namespace rack_plugin_Geodesics {

struct Entropia : Module {
	enum ParamIds {
		RUN_PARAM,
		STEPCLOCK_PARAM,// magnetic clock
		RESET_PARAM,
		RESETONRUN_PARAM,
		LENGTH_PARAM,
		ENUMS(CV_PARAMS, 16),// first 8 are blue, last 8 are yellow
		ENUMS(PROB_PARAMS, 8),// prob knobs
		ENUMS(OCT_PARAMS, 2),// energy (range)
		ENUMS(QUANTIZE_PARAMS, 2),// plank
		STATESWITCH_PARAM,// state switch
		SWITCHADD_PARAM,
		ENUMS(FIXEDCV_PARAMS, 2),
		ENUMS(EXTSIG_PARAMS, 2),
		ENUMS(RANDOM_PARAMS, 2),
		GPROB_PARAM,
		CLKSRC_PARAM,
		ENUMS(EXTAUDIO_PARAMS, 2),
		NUM_PARAMS
	};
	enum InputIds {
		CERTAIN_CLK_INPUT,
		UNCERTAIN_CLK_INPUT,
		LENGTH_INPUT,
		RUN_INPUT,
		RESET_INPUT,
		STATESWITCH_INPUT,// state switch
		SWITCHADD_INPUT,
		ENUMS(OCTCV_INPUTS, 2),
		ENUMS(EXTSIG_INPUTS, 2),
		ENUMS(QUANTIZE_INPUTS, 2),
		GPROB_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CV_OUTPUT,// main output
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(STEP_LIGHTS, 16),// first 8 are blue, last 8 are yellow
		ENUMS(CV_LIGHT, 3),// main output (room for Blue-Yellow-White)
		RUN_LIGHT,
		STEPCLOCK_LIGHT,
		RESET_LIGHT,
		RESETONRUN_LIGHT,
		ENUMS(LENGTH_LIGHTS, 8),// all off when len = 8, north-west turns on when len = 7, north-west and west on when len = 6, etc
		STATESWITCH_LIGHT,
		SWITCHADD_LIGHT,
		ADD_LIGHT,
		ENUMS(QUANTIZE_LIGHTS, 2),
		ENUMS(OCT_LIGHTS, 6),// first 3 are blue, last 3 are yellow (symetrical so only 3 instead of 5 declared); 0 is center, 1 is inside mirrors, 2 is outside mirrors
		ENUMS(FIXEDCV_LIGHTS, 2),
		ENUMS(EXTSIG_LIGHTS, 2),
		ENUMS(RANDOM_LIGHTS, 2),
		ENUMS(CLKSRC_LIGHTS, 2),// certain, uncertain
		ENUMS(EXTAUDIO_LIGHTS, 2),
		ENUMS(EXTCV_LIGHTS, 2),
		NUM_LIGHTS
	};
	
	
	// Constants
	enum SourceIds {SRC_CV, SRC_EXT, SRC_RND};
	
	// Need to save, with reset
	int panelTheme = 0;
	bool running;
	bool resetOnRun;
	int length;
	int quantize;// a.k.a. plank constant, bit0 = blue, bit1 = yellow
	int audio;// bit0 = blue has audio src (else is cv), bit1 = yellow has audio src (else is cv)
	int ranges[2];// [0; 2], number of extra octaves to span each side of central octave (which is C4: 0 - 1V) 
	bool addMode;
	int sources[2];// [0; ], first is blue, 2nd yellow; follows SourceIds
	int stepIndex;
	bool pipeBlue[8];
	float randomCVs[2];// used in SRC_RND
	int clkSource;// which clock to use (0 = both, 1 = certain only, 2 = uncertain only)
	
	
	// No need to save
	int stepIndexOld;// when equal to stepIndex, crossfade (antipop) is finished, when not equal, crossfade until counter 0, then set to stepIndex
	long crossFadeStepsToGo;
	long clockIgnoreOnReset;
	float resetLight;
	float cvLight;
	unsigned int lightRefreshCounter = 0;
	bool rangeInc[2] = {true, true};// true when 1-3-5 increasing, false when 5-3-1 decreasing
	Trigger runningTrigger;
	Trigger plankTriggers[2];
	Trigger lengthTrigger;
	Trigger stateSwitchTrigger;
	Trigger switchAddTrigger;
	Trigger certainClockTrigger;
	Trigger uncertainClockTrigger;
	Trigger octTriggers[2];
	Trigger stepClockTrigger;
	Trigger resetTrigger;
	Trigger resetOnRunTrigger;
	Trigger fixedSrcTriggers[2];
	Trigger rndSrcTriggers[2];
	Trigger extSrcTriggers[2];
	Trigger extAudioTriggers[2];
	Trigger clkSrcTrigger;
	float stepClockLight = 0.0f;
	float stateSwitchLight = 0.0f;
	
	inline float quantizeCV(float cv) {return roundf(cv * 12.0f) / 12.0f;}
	inline void updatePipeBlue(int step) {
		float effectiveKnob = params[PROB_PARAMS + step].value + -1.0f * (params[GPROB_PARAM].value + inputs[GPROB_INPUT].value / 5.0f);
		pipeBlue[step] = effectiveKnob > randomUniform();
	}
	inline void updateRandomCVs() {
		randomCVs[0] = randomUniform();
		randomCVs[1] = randomUniform();
		cvLight = 1.0f;// this could be elsewhere since no relevance to randomCVs, but ok here
	}
	
	
	Entropia() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		for (int i = 0; i < 8; i++)
			params[Entropia::PROB_PARAMS + i].value = 1.0f;// HACK since params not initialized properly yet, remove this in Rack 1.0
		onReset();
	}

	
	void onReset() override {
		running = true;
		resetOnRun = false;
		length = 8;
		quantize = 3;
		audio = 0;
		addMode = false;
		for (int i = 0; i < 2; i++) {
			ranges[i] = 1;	
			sources[i] = SRC_CV;
		}
		clkSource = 0;
		initRun();
		clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * engineGetSampleRate());
	}

	
	void onRandomize() override {
		initRun();
	}
	

	void initRun() {
		stepIndex = 0;
		stepIndexOld = 0;
		crossFadeStepsToGo = 0;
		for (int i = 0; i < 8; i++)
			updatePipeBlue(i);
		updateRandomCVs();
		resetLight = 0.0f;
		cvLight = 0.0f;
	}
	

	json_t *toJson() override {
		json_t *rootJ = json_object();

		// panelTheme
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));

		// running
		json_object_set_new(rootJ, "running", json_boolean(running));

		// resetOnRun
		json_object_set_new(rootJ, "resetOnRun", json_boolean(resetOnRun));

		// length
		json_object_set_new(rootJ, "length", json_integer(length));

		// quantize
		json_object_set_new(rootJ, "quantize", json_integer(quantize));

		// audio
		json_object_set_new(rootJ, "audio", json_integer(audio));

		// ranges
		json_object_set_new(rootJ, "ranges0", json_integer(ranges[0]));
		json_object_set_new(rootJ, "ranges1", json_integer(ranges[1]));

		// addMode
		json_object_set_new(rootJ, "addMode", json_boolean(addMode));

		// sources
		json_object_set_new(rootJ, "sources0", json_integer(sources[0]));
		json_object_set_new(rootJ, "sources1", json_integer(sources[1]));

		// stepIndex
		json_object_set_new(rootJ, "stepIndex", json_integer(stepIndex));

		// pipeBlue (only need to save the one corresponding to stepIndex, since others will get regenerated when moving to those steps)
		json_object_set_new(rootJ, "pipeBlue", json_boolean(pipeBlue[stepIndex]));
		
		// randomCVs (only need to save the one corresponding to stepIndex, since others will get regenerated when moving to those steps)
		json_object_set_new(rootJ, "randomCVs0", json_real(randomCVs[0]));
		json_object_set_new(rootJ, "randomCVs1", json_real(randomCVs[1]));

		// clkSource
		json_object_set_new(rootJ, "clkSource", json_integer(clkSource));

		return rootJ;
	}

	
	void fromJson(json_t *rootJ) override {
		// panelTheme
		json_t *panelThemeJ = json_object_get(rootJ, "panelTheme");
		if (panelThemeJ)
			panelTheme = json_integer_value(panelThemeJ);

		// running
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);

		// resetOnRun
		json_t *resetOnRunJ = json_object_get(rootJ, "resetOnRun");
		if (resetOnRunJ)
			resetOnRun = json_is_true(resetOnRunJ);

		// length
		json_t *lengthJ = json_object_get(rootJ, "length");
		if (lengthJ)
			length = json_integer_value(lengthJ);

		// quantize
		json_t *quantizeJ = json_object_get(rootJ, "quantize");
		if (quantizeJ)
			quantize = json_integer_value(quantizeJ);

		// audio
		json_t *audioJ = json_object_get(rootJ, "audio");
		if (audioJ)
			audio = json_integer_value(audioJ);

		// ranges
		json_t *ranges0J = json_object_get(rootJ, "ranges0");
		if (ranges0J)
			ranges[0] = json_integer_value(ranges0J);
		json_t *ranges1J = json_object_get(rootJ, "ranges1");
		if (ranges1J)
			ranges[1] = json_integer_value(ranges1J);

		// addMode
		json_t *addModeJ = json_object_get(rootJ, "addMode");
		if (addModeJ)
			addMode = json_is_true(addModeJ);

		// sources
		json_t *sources0J = json_object_get(rootJ, "sources0");
		if (sources0J)
			sources[0] = json_integer_value(sources0J);
		json_t *sources1J = json_object_get(rootJ, "sources1");
		if (sources1J)
			sources[1] = json_integer_value(sources1J);

		// stepIndex
		json_t *stepIndexJ = json_object_get(rootJ, "stepIndex");
		if (stepIndexJ)
			stepIndex = json_integer_value(stepIndexJ);

		// pipeBlue (only saved the one corresponding to stepIndex, since others will get regenerated when moving to those steps)
		json_t *pipeBlueJ = json_object_get(rootJ, "pipeBlue");
		if (pipeBlueJ)
			pipeBlue[stepIndex] = json_is_true(pipeBlueJ);

		// randomCVs (only saved the one corresponding to stepIndex, since others will get regenerated when moving to those steps)
		json_t *randomCVs0J = json_object_get(rootJ, "randomCVs0");
		if (randomCVs0J)
			randomCVs[0] = json_number_value(randomCVs0J);
		json_t *randomCVs1J = json_object_get(rootJ, "randomCVs1");
		if (randomCVs1J)
			randomCVs[1] = json_number_value(randomCVs1J);

		// clkSource
		json_t *clkSourceJ = json_object_get(rootJ, "clkSource");
		if (clkSourceJ)
			clkSource = json_integer_value(clkSourceJ);

		rangeInc[0] = true;
		rangeInc[1] = true;
		stepIndexOld = stepIndex;
	}

	
	void step() override {
		float crossFadeTime = 0.005f;
		float sampleTime = engineGetSampleTime();
	
		//********** Buttons, knobs, switches and inputs **********
	
		// Run button
		if (runningTrigger.process(params[RUN_PARAM].value + inputs[RUN_INPUT].value)) {// no input refresh here, don't want to introduce startup skew
			running = !running;
			if (running) {
				if (resetOnRun)
					initRun();
				if (resetOnRun || clockIgnoreOnRun)
					clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * engineGetSampleRate());
			}
		}
		
		if ((lightRefreshCounter & userInputsStepSkipMask) == 0) {

			// Length button and input
			bool lengthTrig = lengthTrigger.process(params[LENGTH_PARAM].value);
			if (inputs[LENGTH_INPUT].active) {
				length = clamp(8 - (int)(inputs[LENGTH_INPUT].value * 7.0f / 10.0f + 0.5f)  , 1, 8);
			}
			else if (lengthTrig) {
				if (length > 1) length--;
				else length = 8;
			}

			// Plank buttons (quantize)
			if (plankTriggers[0].process(params[QUANTIZE_PARAMS + 0].value))
				quantize ^= 0x1;
			if (plankTriggers[1].process(params[QUANTIZE_PARAMS + 1].value))
				quantize ^= 0x2;

			// Range buttons and CV inputs
			for (int i = 0; i < 2; i++) {
				bool rangeTrig = octTriggers[i].process(params[OCT_PARAMS + i].value);
				if (inputs[OCTCV_INPUTS + i].active) {
					if (inputs[OCTCV_INPUTS + i].value <= -1.0f)
						ranges[i] = 0;
					else if (inputs[OCTCV_INPUTS + i].value < 1.0f)
						ranges[i] = 1;
					else 
						ranges[i] = 2;
				}
				else if (rangeTrig) {
					if (rangeInc[i]) {
						ranges[i]++;
						if (ranges[i] >= 3) {
							ranges[i] = 1;
							rangeInc[i] = false;
						}
					}
					else {
						ranges[i]--;
						if (ranges[i] < 0) {
							ranges[i] = 1;
							rangeInc[i] = true;
						}
					}
				}
			}
			
			// Source buttons (fixedCV, random, ext)
			for (int i = 0; i < 2; i++) {
				if (rndSrcTriggers[i].process(params[RANDOM_PARAMS + i].value))
					sources[i] = SRC_RND;
				if (extSrcTriggers[i].process(params[EXTSIG_PARAMS + i].value))
					sources[i] = SRC_EXT;
				if (fixedSrcTriggers[i].process(params[FIXEDCV_PARAMS + i].value))
					sources[i] = SRC_CV;
				if (extAudioTriggers[i].process(params[EXTAUDIO_PARAMS + i].value))
					audio ^= (1 << i);
			}
			
			// addMode
			if (switchAddTrigger.process(params[SWITCHADD_PARAM].value + inputs[SWITCHADD_INPUT].value)) {
				addMode = !addMode;
			}		
		
			// StateSwitch
			if (stateSwitchTrigger.process(params[STATESWITCH_PARAM].value + inputs[STATESWITCH_INPUT].value)) {
				pipeBlue[stepIndex] = !pipeBlue[stepIndex];
				stateSwitchLight = 1.0f;
			}		
		
			// Reset on Run button
			if (resetOnRunTrigger.process(params[RESETONRUN_PARAM].value)) {
				resetOnRun = !resetOnRun;
			}	

			if (clkSrcTrigger.process(params[CLKSRC_PARAM].value)) {
				if (++clkSource > 2)
					clkSource = 0;
			}
		}// userInputs refresh
		

		//********** Clock and reset **********
		
		// External clocks
		if (running && clockIgnoreOnReset == 0l) {
			bool certainClockTrig = certainClockTrigger.process(inputs[CERTAIN_CLK_INPUT].value);
			bool uncertainClockTrig = uncertainClockTrigger.process(inputs[UNCERTAIN_CLK_INPUT].value);
			certainClockTrig &= (clkSource < 2);
			if (certainClockTrig) {
				stepIndex++;
			}
			uncertainClockTrig &= ((clkSource & 0x1) == 0);
			if (uncertainClockTrig) {
				stepIndex += getWeighted1to8random();
			}
			if (certainClockTrig || uncertainClockTrig) {
				stepIndex %= length;
				crossFadeStepsToGo = (long)(crossFadeTime * engineGetSampleRate());;
				updatePipeBlue(stepIndex);
				updateRandomCVs();
			}
		}				
		// Magnetic clock (manual step clock)
		if (stepClockTrigger.process(params[STEPCLOCK_PARAM].value)) {
			if (++stepIndex >= length) stepIndex = 0;
			crossFadeStepsToGo = (long)(crossFadeTime * engineGetSampleRate());
			updatePipeBlue(stepIndex);
			updateRandomCVs();
			stepClockLight = 1.0f;
		}
		
		// Reset
		if (resetTrigger.process(inputs[RESET_INPUT].value + params[RESET_PARAM].value)) {
			initRun();
			resetLight = 1.0f;
			clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * engineGetSampleRate());
			certainClockTrigger.reset();
			uncertainClockTrigger.reset();
		}
		
		
		//********** Outputs and lights **********

		// Output
		int crossFadeActive = audio;
		if (sources[0] != SRC_EXT) crossFadeActive &= ~0x1;
		if (sources[1] != SRC_EXT) crossFadeActive &= ~0x2;
		if (crossFadeStepsToGo > 0 && crossFadeActive != 0)
		{
			long crossFadeStepsToGoInit = (long)(crossFadeTime * engineGetSampleRate());
			float fadeRatio = ((float)crossFadeStepsToGo) / ((float)crossFadeStepsToGoInit);
			outputs[CV_OUTPUT].value = calcOutput(stepIndexOld) * fadeRatio + calcOutput(stepIndex) * (1.0f - fadeRatio);
			crossFadeStepsToGo--;
			if (crossFadeStepsToGo == 0)
				stepIndexOld = stepIndex;
		}
		else
			outputs[CV_OUTPUT].value = calcOutput(stepIndex);
		
		lightRefreshCounter++;
		if (lightRefreshCounter >= displayRefreshStepSkips) {
			lightRefreshCounter = 0;

			// Reset light
			lights[RESET_LIGHT].value =	resetLight;	
			resetLight -= (resetLight / lightLambda) * sampleTime * displayRefreshStepSkips;	
			
			// Run light
			lights[RUN_LIGHT].value = running ? 1.0f : 0.0f;
			lights[RESETONRUN_LIGHT].value = resetOnRun ? 1.0f : 0.0f;
			
			// Length lights
			for (int i = 0; i < 8; i++)
				lights[LENGTH_LIGHTS + i].value = (i < length ? 0.0f : 1.0f);
			
			// Plank
			lights[QUANTIZE_LIGHTS + 0].value = (quantize & 0x1) ? 1.0f : 0.0f;// Blue
			lights[QUANTIZE_LIGHTS + 1].value = (quantize & 0x2) ? 1.0f : 0.0f;// Yellow

			// step and main output lights (GeoBlueYellowWhiteLight)
			lights[CV_LIGHT + 0].value = (pipeBlue[stepIndex]) ? 1.0f * cvLight : 0.0f;
			lights[CV_LIGHT + 1].value = (!pipeBlue[stepIndex] && !addMode) ? 1.0f * cvLight : 0.0f;
			lights[CV_LIGHT + 2].value = (!pipeBlue[stepIndex] && addMode) ? 1.0f * cvLight : 0.0f;
			cvLight -= (cvLight / lightLambda) * sampleTime * displayRefreshStepSkips;	
			for (int i = 0; i < 8; i++) {
				lights[STEP_LIGHTS + i].value = ((pipeBlue[i] || addMode) && stepIndex == i) ? 1.0f : 0.0f;
				lights[STEP_LIGHTS + 8 + i].value = ((!pipeBlue[i]) && stepIndex == i) ? 1.0f : 0.0f;
			}
			
			// Range (energy) lights
			for (int i = 0; i < 3; i++) {
				lights[OCT_LIGHTS + i].value = (i <= ranges[0] ? 1.0f : 0.0f);
				lights[OCT_LIGHTS + 3 + i].value = (i <= ranges[1] ? 1.0f : 0.0f);
			}
				
			// Step clocks light
			lights[STEPCLOCK_LIGHT].value = stepClockLight;
			stepClockLight -= (stepClockLight / lightLambda) * sampleTime * displayRefreshStepSkips;

			// Swtich add light
			lights[SWITCHADD_LIGHT].value = (addMode ? 0.0f : 1.0f);
			lights[ADD_LIGHT].value = (addMode ? 1.0f : 0.0f);
			
			// State switch light
			lights[STATESWITCH_LIGHT].value = stateSwitchLight;
			stateSwitchLight -= (stateSwitchLight / lightLambda) * sampleTime * displayRefreshStepSkips;
			
			for (int i = 0; i < 2; i++) {
				// Sources lights
				lights[RANDOM_LIGHTS + i].value = (sources[i] == SRC_RND) ? 1.0f : 0.0f;
				lights[EXTSIG_LIGHTS + i].value = (sources[i] == SRC_EXT) ? 1.0f : 0.0f;
				lights[FIXEDCV_LIGHTS + i].value = (sources[i] == SRC_CV) ? 1.0f : 0.0f;
				
				// Audio lights
				lights[EXTAUDIO_LIGHTS + i].value = ((audio & (1 << i)) != 0) ? 1.0f : 0.0f;
				lights[EXTCV_LIGHTS + i].value = ((audio & (1 << i)) == 0) ? 1.0f : 0.0f;
			}
			
			
			
			// Clock source lights
			lights[CLKSRC_LIGHTS + 0].value = (clkSource < 2) ? 1.0f : 0.0f;
			lights[CLKSRC_LIGHTS + 1].value = ((clkSource & 0x1) == 0) ? 1.0f : 0.0f;
			
		}// lightRefreshCounter
		
		if (clockIgnoreOnReset > 0l)
			clockIgnoreOnReset--;
	}// step()
	
	inline float calcOutput(int stepIdx) {
		if (addMode) 
			return getStepCV(stepIdx, true) + (pipeBlue[stepIdx] ? 0.0f : getStepCV(stepIdx, false));
		return getStepCV(stepIdx, pipeBlue[stepIdx]);
	}
	
	float getStepCV(int step, bool blue) {
		int colorIndex = blue ? 0 : 1;
		float knobVal = params[CV_PARAMS + (colorIndex << 3) + step].value;
		float cv = 0.0f;
		
		if (sources[colorIndex] == SRC_RND) {
			cv = randomCVs[colorIndex] * (knobVal * 10.0f - 5.0f);
		}
		else if (sources[colorIndex] == SRC_EXT) {
			float extOffset = ((audio & (1 << colorIndex)) != 0) ? 0.0f : -1.0f;
			cv = clamp(inputs[EXTSIG_INPUTS + colorIndex].value * (knobVal * 2.0f + extOffset), -10.0f, 10.0f);
		}
		else {// SRC_CV
			int range = ranges[colorIndex];
			if ( (blue && (quantize & 0x1) != 0) || (!blue && (quantize > 1)) ) {
				cv = (knobVal * (float)(range * 2 + 1) - (float)range);
				cv = quantizeCV(cv);
			}
			else {
				int maxCV = (range == 0 ? 1 : (range * 5));// maxCV is [1, 5, 10]
				cv = knobVal * (float)(maxCV * 2) - (float)maxCV;
			}
		}
		
		return cv;
	}
	
};


struct EntropiaWidget : ModuleWidget {

	struct PanelThemeItem : MenuItem {
		Entropia *module;
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

		Entropia *module = dynamic_cast<Entropia*>(this->module);
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
		menu->addChild(darkItem);

		return menu;
	}	
	
	EntropiaWidget(Entropia *module) : ModuleWidget(module) {
		// Main panel from Inkscape
        DynamicSVGPanel *panel = new DynamicSVGPanel();
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/WhiteLight/Entropia-WL.svg")));
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/DarkMatter/Entropia-DM.svg")));
        box.size = panel->box.size;
        panel->mode = &module->panelTheme;
        addChild(panel);

		// Screws 
		// part of svg panel, no code required
		
		static constexpr float colRulerCenter = 157.0f;//box.size.x / 2.0f;
		static constexpr float rowRulerOutput = 380.0f - 155.5f;
		static constexpr float radius1 = 50.0f;
		static constexpr float offset1 = 35.5f;
		static constexpr float radius3 = 105.0f;
		static constexpr float offset3 = 74.5f;
		static constexpr float offset2b = 74.5f;// big
		static constexpr float offset2s = 27.5f;// small
		
		
		// CV out and light 
		addOutput(createDynamicPort<GeoPort>(Vec(colRulerCenter, rowRulerOutput), Port::OUTPUT, module, Entropia::CV_OUTPUT, &module->panelTheme));		
		addChild(createLightCentered<SmallLight<GeoBlueYellowWhiteLight>>(Vec(colRulerCenter, rowRulerOutput - 21.5f), module, Entropia::CV_LIGHT));
		
		// Blue CV knobs
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter, rowRulerOutput - radius1), module, Entropia::CV_PARAMS + 0, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter + offset1, rowRulerOutput - offset1), module, Entropia::CV_PARAMS + 1, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter + radius1, rowRulerOutput), module, Entropia::CV_PARAMS + 2, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter + offset1, rowRulerOutput + offset1), module, Entropia::CV_PARAMS + 3, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter, rowRulerOutput + radius1), module, Entropia::CV_PARAMS + 4, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter - offset1, rowRulerOutput + offset1), module, Entropia::CV_PARAMS + 5, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter - radius1, rowRulerOutput), module, Entropia::CV_PARAMS + 6, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter - offset1, rowRulerOutput - offset1), module, Entropia::CV_PARAMS + 7, 0.0f, 1.0f, 0.5f, &module->panelTheme));

		// Yellow CV knobs
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter, rowRulerOutput - radius3), module, Entropia::CV_PARAMS + 8 + 0, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter + offset3, rowRulerOutput - offset3), module, Entropia::CV_PARAMS + 8 + 1, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter + radius3, rowRulerOutput), module, Entropia::CV_PARAMS + 8 + 2, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter + offset3, rowRulerOutput + offset3), module, Entropia::CV_PARAMS + 8 + 3, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter, rowRulerOutput + radius3), module, Entropia::CV_PARAMS + 8 + 4, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter - offset3, rowRulerOutput + offset3), module, Entropia::CV_PARAMS + 8 + 5, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter - radius3, rowRulerOutput), module, Entropia::CV_PARAMS + 8 + 6, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter - offset3, rowRulerOutput - offset3), module, Entropia::CV_PARAMS + 8 + 7, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		
		// Prob CV knobs
		addParam(createDynamicParam<GeoKnobRight>(Vec(colRulerCenter + offset2s, rowRulerOutput - offset2b - 3), module, Entropia::PROB_PARAMS + 0, 0.0f, 1.0f, 1.0f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnobBotRight>(Vec(colRulerCenter + offset2b, rowRulerOutput - offset2s - 8), module, Entropia::PROB_PARAMS + 1, 0.0f, 1.0f, 1.0f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnobBottom>(Vec(colRulerCenter + offset2b + 3, rowRulerOutput + offset2s), module, Entropia::PROB_PARAMS + 2, 0.0f, 1.0f, 1.0f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnobBotLeft>(Vec(colRulerCenter + offset2s + 8, rowRulerOutput + offset2b), module, Entropia::PROB_PARAMS + 3, 0.0f, 1.0f, 1.0f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnobLeft>(Vec(colRulerCenter - offset2s, rowRulerOutput + offset2b + 3), module, Entropia::PROB_PARAMS + 4, 0.0f, 1.0f, 1.0f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnobTopLeft>(Vec(colRulerCenter - offset2b, rowRulerOutput + offset2s + 8), module, Entropia::PROB_PARAMS + 5, 0.0f, 1.0f, 1.0f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter - offset2b - 3, rowRulerOutput - offset2s), module, Entropia::PROB_PARAMS + 6, 0.0f, 1.0f, 1.0f, &module->panelTheme));
		addParam(createDynamicParam<GeoKnobTopRight>(Vec(colRulerCenter - offset2s - 7.5f, rowRulerOutput - offset2b + 1.0f), module, Entropia::PROB_PARAMS + 7, 0.0f, 1.0f, 1.0f, &module->panelTheme));
		
		// Blue step lights	
		float radiusBL = 228.5f - 155.5f;// radius blue lights
		addChild(createLightCentered<SmallLight<GeoBlueLight>>(Vec(colRulerCenter, rowRulerOutput - radiusBL), module, Entropia::STEP_LIGHTS + 0));
		addChild(createLightCentered<SmallLight<GeoBlueLight>>(Vec(colRulerCenter + radiusBL * 0.707f, rowRulerOutput - radiusBL * 0.707f), module, Entropia::STEP_LIGHTS + 1));
		addChild(createLightCentered<SmallLight<GeoBlueLight>>(Vec(colRulerCenter + radiusBL, rowRulerOutput), module, Entropia::STEP_LIGHTS + 2));
		addChild(createLightCentered<SmallLight<GeoBlueLight>>(Vec(colRulerCenter + radiusBL * 0.707f, rowRulerOutput + radiusBL * 0.707f), module, Entropia::STEP_LIGHTS + 3));
		addChild(createLightCentered<SmallLight<GeoBlueLight>>(Vec(colRulerCenter, rowRulerOutput + radiusBL), module, Entropia::STEP_LIGHTS + 4));
		addChild(createLightCentered<SmallLight<GeoBlueLight>>(Vec(colRulerCenter - radiusBL * 0.707f, rowRulerOutput + radiusBL * 0.707f), module, Entropia::STEP_LIGHTS + 5));
		addChild(createLightCentered<SmallLight<GeoBlueLight>>(Vec(colRulerCenter - radiusBL, rowRulerOutput), module, Entropia::STEP_LIGHTS + 6));
		addChild(createLightCentered<SmallLight<GeoBlueLight>>(Vec(colRulerCenter - radiusBL * 0.707f, rowRulerOutput - radiusBL * 0.707f), module, Entropia::STEP_LIGHTS + 7));
		radiusBL += 9.0f;
		addChild(createLightCentered<SmallLight<GeoYellowLight>>(Vec(colRulerCenter, rowRulerOutput - radiusBL), module, Entropia::STEP_LIGHTS + 8 + 0));
		addChild(createLightCentered<SmallLight<GeoYellowLight>>(Vec(colRulerCenter + radiusBL * 0.707f, rowRulerOutput - radiusBL * 0.707f), module, Entropia::STEP_LIGHTS + 8 + 1));
		addChild(createLightCentered<SmallLight<GeoYellowLight>>(Vec(colRulerCenter + radiusBL, rowRulerOutput), module, Entropia::STEP_LIGHTS + 8 + 2));
		addChild(createLightCentered<SmallLight<GeoYellowLight>>(Vec(colRulerCenter + radiusBL * 0.707f, rowRulerOutput + radiusBL * 0.707f), module, Entropia::STEP_LIGHTS + 8 + 3));
		addChild(createLightCentered<SmallLight<GeoYellowLight>>(Vec(colRulerCenter, rowRulerOutput + radiusBL), module, Entropia::STEP_LIGHTS + 8 + 4));
		addChild(createLightCentered<SmallLight<GeoYellowLight>>(Vec(colRulerCenter - radiusBL * 0.707f, rowRulerOutput + radiusBL * 0.707f), module, Entropia::STEP_LIGHTS + 8 + 5));
		addChild(createLightCentered<SmallLight<GeoYellowLight>>(Vec(colRulerCenter - radiusBL, rowRulerOutput), module, Entropia::STEP_LIGHTS + 8 + 6));
		addChild(createLightCentered<SmallLight<GeoYellowLight>>(Vec(colRulerCenter - radiusBL * 0.707f, rowRulerOutput - radiusBL * 0.707f), module, Entropia::STEP_LIGHTS + 8 + 7));
	
	
		// Length jack, button and lights
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + 116.5f, rowRulerOutput + 70.0f), Port::INPUT, module, Entropia::LENGTH_INPUT, &module->panelTheme));
		static float lenButtonX = colRulerCenter + 130.5f;
		static float lenButtonY = rowRulerOutput + 36.5f;
		addParam(createDynamicParam<GeoPushButton>(Vec(lenButtonX, lenButtonY), module, Entropia::LENGTH_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoRedLight>>(Vec(lenButtonX        , lenButtonY - 14.5f), module, Entropia::LENGTH_LIGHTS + 0));
		addChild(createLightCentered<SmallLight<GeoRedLight>>(Vec(lenButtonX + 10.5f, lenButtonY - 10.5f), module, Entropia::LENGTH_LIGHTS + 1));
		addChild(createLightCentered<SmallLight<GeoRedLight>>(Vec(lenButtonX + 14.5f, lenButtonY        ), module, Entropia::LENGTH_LIGHTS + 2));
		addChild(createLightCentered<SmallLight<GeoRedLight>>(Vec(lenButtonX + 10.5f, lenButtonY + 10.5f), module, Entropia::LENGTH_LIGHTS + 3));
		addChild(createLightCentered<SmallLight<GeoRedLight>>(Vec(lenButtonX        , lenButtonY + 14.5f), module, Entropia::LENGTH_LIGHTS + 4));
		addChild(createLightCentered<SmallLight<GeoRedLight>>(Vec(lenButtonX - 10.5f, lenButtonY + 10.5f), module, Entropia::LENGTH_LIGHTS + 5));
		addChild(createLightCentered<SmallLight<GeoRedLight>>(Vec(lenButtonX - 14.5f, lenButtonY        ), module, Entropia::LENGTH_LIGHTS + 6));
		addChild(createLightCentered<SmallLight<GeoRedLight>>(Vec(lenButtonX - 10.5f, lenButtonY - 10.5f), module, Entropia::LENGTH_LIGHTS + 7));
		
		// Clock inputs
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - 130.5f, rowRulerOutput + 36.5f), Port::INPUT, module, Entropia::CERTAIN_CLK_INPUT, &module->panelTheme));
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - 116.5f, rowRulerOutput + 70.0f), Port::INPUT, module, Entropia::UNCERTAIN_CLK_INPUT, &module->panelTheme));
		// Clock source button and LEDs
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(43.0f, 256.5f), module, Entropia::CLKSRC_LIGHTS + 0));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(55.0f, 284.5f), module, Entropia::CLKSRC_LIGHTS + 1));
		addParam(createDynamicParam<GeoPushButton>(Vec(46.0f, 272.5f), module, Entropia::CLKSRC_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		
		
		// Switch, add, state (jacks, buttons, ligths)
		// left side
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - 130.5f, rowRulerOutput - 36.0f), Port::INPUT, module, Entropia::SWITCHADD_INPUT, &module->panelTheme));
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter - 115.5f, rowRulerOutput - 69.0f), module, Entropia::SWITCHADD_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(colRulerCenter - 115.5f - 7.0f, rowRulerOutput - 69.0f + 13.0f), module, Entropia::SWITCHADD_LIGHT));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(colRulerCenter - 115.5f + 3.0f, rowRulerOutput - 69.0f + 14.0f), module, Entropia::ADD_LIGHT));
		// right side
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + 130.5f, rowRulerOutput - 36.0f), Port::INPUT, module, Entropia::STATESWITCH_INPUT, &module->panelTheme));
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter + 115.5f, rowRulerOutput - 69.0f), module, Entropia::STATESWITCH_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(colRulerCenter + 115.5f + 7.0f, rowRulerOutput - 69.0f + 13.0f), module, Entropia::STATESWITCH_LIGHT));
		
		// Plank constant (jack, light and button)
		// left side (blue)
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - 96.0f, rowRulerOutput - 96.0f), Port::INPUT, module, Entropia::OCTCV_INPUTS + 0, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoBlueLight>>(Vec(colRulerCenter - 96.0f - 13.0f, rowRulerOutput - 96.0f - 13.0f), module, Entropia::QUANTIZE_LIGHTS + 0));
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter - 96.0f - 23.0f, rowRulerOutput - 96.0f - 23.0f), module, Entropia::QUANTIZE_PARAMS + 0, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// right side (yellow)
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + 96.0f, rowRulerOutput - 96.0f), Port::INPUT, module, Entropia::OCTCV_INPUTS + 1, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoYellowLight>>(Vec(colRulerCenter + 96.0f + 13.0f, rowRulerOutput - 96.0f - 13.0f), module, Entropia::QUANTIZE_LIGHTS + 1));
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter + 96.0f + 23.0f, rowRulerOutput - 96.0f - 23.0f), module, Entropia::QUANTIZE_PARAMS + 1, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		
		// Energy (button and lights)
		// left side (blue)
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter - 69.5f, rowRulerOutput - 116.0f), module, Entropia::OCT_PARAMS + 0, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoBlueLight>>(Vec(colRulerCenter - 69.5f - 12.0f, rowRulerOutput - 116.0f + 9.0f), module, Entropia::OCT_LIGHTS + 0));
		addChild(createLightCentered<SmallLight<GeoBlueLight>>(Vec(colRulerCenter - 69.5f - 15.0f, rowRulerOutput - 116.0f - 1.0f), module, Entropia::OCT_LIGHTS + 1));
		addChild(createLightCentered<SmallLight<GeoBlueLight>>(Vec(colRulerCenter - 69.5f - 3.0f, rowRulerOutput - 116.0f + 14.0f), module, Entropia::OCT_LIGHTS + 1));
		addChild(createLightCentered<SmallLight<GeoBlueLight>>(Vec(colRulerCenter - 69.5f - 10.0f, rowRulerOutput - 116.0f - 11.0f), module, Entropia::OCT_LIGHTS + 2));
		addChild(createLightCentered<SmallLight<GeoBlueLight>>(Vec(colRulerCenter - 69.5f + 7.0f, rowRulerOutput - 116.0f + 12.0f), module, Entropia::OCT_LIGHTS + 2));
		// right side (yellow)
		// left side (blue)
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter + 69.5f, rowRulerOutput - 116.0f), module, Entropia::OCT_PARAMS + 1, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoYellowLight>>(Vec(colRulerCenter + 69.5f + 12.0f, rowRulerOutput - 116.0f + 9.0f), module, Entropia::OCT_LIGHTS + 3 + 0));
		addChild(createLightCentered<SmallLight<GeoYellowLight>>(Vec(colRulerCenter + 69.5f + 15.0f, rowRulerOutput - 116.0f - 1.0f), module, Entropia::OCT_LIGHTS + 3 + 1));
		addChild(createLightCentered<SmallLight<GeoYellowLight>>(Vec(colRulerCenter + 69.5f + 3.0f, rowRulerOutput - 116.0f + 14.0f), module, Entropia::OCT_LIGHTS + 3 + 1));
		addChild(createLightCentered<SmallLight<GeoYellowLight>>(Vec(colRulerCenter + 69.5f + 10.0f, rowRulerOutput - 116.0f - 11.0f), module, Entropia::OCT_LIGHTS + 3 + 2));
		addChild(createLightCentered<SmallLight<GeoYellowLight>>(Vec(colRulerCenter + 69.5f - 7.0f, rowRulerOutput - 116.0f + 12.0f), module, Entropia::OCT_LIGHTS + 3 + 2));
		
		
		// Top portion
		static constexpr float rowRulerTop = rowRulerOutput - 150.0f;
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter, rowRulerTop - 30.5f), Port::INPUT, module, Entropia::GPROB_INPUT, &module->panelTheme));
		addParam(createDynamicParam<GeoKnob>(Vec(colRulerCenter, rowRulerTop), module, Entropia::GPROB_PARAM, -1.0f, 1.0f, 0.0f, &module->panelTheme));
		
		// Left side top
		// ext
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - 77.5f, rowRulerTop), Port::INPUT, module, Entropia::EXTSIG_INPUTS + 0, &module->panelTheme));
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter - 41.5f, rowRulerTop), module, Entropia::EXTSIG_PARAMS + 0, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoBlueLight>>(Vec(colRulerCenter - 26.5f, rowRulerTop), module, Entropia::EXTSIG_LIGHTS + 0));
		// random
		static constexpr float buttonOffsetX = 35.5f;// button
		static constexpr float buttonOffsetY = 20.5f;// button
		static constexpr float lightOffsetX = 22.5f;// light
		static constexpr float lightOffsetY = 12.5f;// light
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter - buttonOffsetX, rowRulerTop - buttonOffsetY), module, Entropia::RANDOM_PARAMS + 0, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoBlueLight>>(Vec(colRulerCenter - lightOffsetX, rowRulerTop - lightOffsetY), module, Entropia::RANDOM_LIGHTS + 0));
		// fixed cv
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter - buttonOffsetX, rowRulerTop + buttonOffsetY), module, Entropia::FIXEDCV_PARAMS + 0, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoBlueLight>>(Vec(colRulerCenter - lightOffsetX, rowRulerTop + lightOffsetY), module, Entropia::FIXEDCV_LIGHTS + 0));
		// audio
		addParam(createDynamicParam<GeoPushButton>(Vec(38.5f, 380.0f - 325.5f), module, Entropia::EXTAUDIO_PARAMS + 0, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(40.0f, 380.0f - 311.5f), module, Entropia::EXTAUDIO_LIGHTS + 0));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(48.5f, 380.0f - 315.5f), module, Entropia::EXTCV_LIGHTS + 0));
		
		
		// Right side top
		// ext
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + 77.5f, rowRulerTop), Port::INPUT, module, Entropia::EXTSIG_INPUTS + 1, &module->panelTheme));
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter + 41.5f, rowRulerTop), module, Entropia::EXTSIG_PARAMS + 1, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoYellowLight>>(Vec(colRulerCenter + 26.5f, rowRulerTop), module, Entropia::EXTSIG_LIGHTS + 1));
		// random
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter + buttonOffsetX, rowRulerTop - buttonOffsetY), module, Entropia::RANDOM_PARAMS + 1, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoYellowLight>>(Vec(colRulerCenter + lightOffsetX, rowRulerTop - lightOffsetY), module, Entropia::RANDOM_LIGHTS + 1));
		// fixed cv
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter + buttonOffsetX, rowRulerTop + buttonOffsetY), module, Entropia::FIXEDCV_PARAMS + 1, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoYellowLight>>(Vec(colRulerCenter + lightOffsetX, rowRulerTop + lightOffsetY), module, Entropia::FIXEDCV_LIGHTS + 1));
		// audio
		addParam(createDynamicParam<GeoPushButton>(Vec(315.0f - 38.5f, 380.0f - 325.5f), module, Entropia::EXTAUDIO_PARAMS + 1, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(315.0f - 40.0f, 380.0f - 311.5f), module, Entropia::EXTAUDIO_LIGHTS + 1));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(315.0f - 48.5f, 380.0f - 315.5f), module, Entropia::EXTCV_LIGHTS + 1));
		
		
		
		// Bottom row
		
		// Run jack, light and button
		static constexpr float rowRulerRunJack = 380.0f - 32.5f;
		static constexpr float offsetRunJackX = 119.5f;
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter - offsetRunJackX, rowRulerRunJack), Port::INPUT, module, Entropia::RUN_INPUT, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(colRulerCenter - offsetRunJackX + 18.0f, rowRulerRunJack), module, Entropia::RUN_LIGHT));
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter - offsetRunJackX + 33.0f, rowRulerRunJack), module, Entropia::RUN_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));	
		
		// Reset jack, light and button
		addInput(createDynamicPort<GeoPort>(Vec(colRulerCenter + offsetRunJackX, rowRulerRunJack), Port::INPUT, module, Entropia::RESET_INPUT, &module->panelTheme));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(colRulerCenter + offsetRunJackX - 18.0f, rowRulerRunJack), module, Entropia::RESET_LIGHT));
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter + offsetRunJackX - 33.0f, rowRulerRunJack), module, Entropia::RESET_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));	
	
		static constexpr float offsetMagneticButton = 42.5f;
		// Magnetic clock (step clocks)
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(colRulerCenter - offsetMagneticButton - 15.0f, rowRulerRunJack), module, Entropia::STEPCLOCK_LIGHT));
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter - offsetMagneticButton, rowRulerRunJack), module, Entropia::STEPCLOCK_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));			
		// Reset on Run light and button
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(Vec(colRulerCenter + offsetMagneticButton + 15.0f, rowRulerRunJack), module, Entropia::RESETONRUN_LIGHT));
		addParam(createDynamicParam<GeoPushButton>(Vec(colRulerCenter + offsetMagneticButton, rowRulerRunJack), module, Entropia::RESETONRUN_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));	
		
		
	}
};

} // namespace rack_plugin_Geodesics

using namespace rack_plugin_Geodesics;

RACK_PLUGIN_MODEL_INIT(Geodesics, Entropia) {
   Model *modelEntropia = Model::create<Entropia, EntropiaWidget>("Geodesics", "Entropia", "Entropia", SEQUENCER_TAG);
   return modelEntropia;
}

/*CHANGE LOG

0.6.6:
add audio/cv switch for ext source, and activate anti-pop when at least one channel has audio input
make step knobs select a gain rather that attenuverter for ext sources

0.6.5:
created

*/
