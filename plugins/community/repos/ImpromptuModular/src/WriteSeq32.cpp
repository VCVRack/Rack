//***********************************************************************************************
//Three channel 32-step writable sequencer module for VCV Rack by Marc Boulé
//
//Based on code from the Fundamental and AudibleInstruments plugins by Andrew Belt 
//and graphics from the Component Library by Wes Milholen 
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//***********************************************************************************************


#include "ImpromptuModular.hpp"
#include "PhraseSeqUtil.hpp"

namespace rack_plugin_ImpromptuModular {

struct WriteSeq32 : Module {
	enum ParamIds {
		SHARP_PARAM,
		ENUMS(WINDOW_PARAM, 4),
		QUANTIZE_PARAM,
		ENUMS(GATE_PARAM, 8),
		CHANNEL_PARAM,
		COPY_PARAM,
		PASTE_PARAM,
		RUN_PARAM,
		WRITE_PARAM,
		STEPL_PARAM,
		MONITOR_PARAM,
		STEPR_PARAM,
		STEPS_PARAM,
		AUTOSTEP_PARAM,
		PASTESYNC_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CHANNEL_INPUT,// no longer used
		CV_INPUT,	
		GATE_INPUT,
		WRITE_INPUT,
		STEPL_INPUT,
		STEPR_INPUT,
		CLOCK_INPUT,
		RESET_INPUT,
		// -- 0.6.2
		RUNCV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(CV_OUTPUTS, 3),
		ENUMS(GATE_OUTPUTS, 3),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(WINDOW_LIGHTS, 4),
		ENUMS(STEP_LIGHTS, 8),
		ENUMS(GATE_LIGHTS, 8),
		ENUMS(CHANNEL_LIGHTS, 4),
		RUN_LIGHT,
		ENUMS(WRITE_LIGHT, 2),// room for GreenRed
		PENDING_LIGHT,
		NUM_LIGHTS
	};

	// Need to save
	int panelTheme = 0;
	bool running;
	int indexStep;
	int indexStepStage;
	int indexChannel;
	float cv[4][32];
	bool gates[4][32];
	bool resetOnRun;

	// No need to save
	int notesPos[8]; // used for rendering notes in LCD_24, 8 gate and 8 step LEDs 
	float cvCPbuffer[32];// copy paste buffer for CVs
	bool gateCPbuffer[32];// copy paste buffer for gates
	long infoCopyPaste;// 0 when no info, positive downward step counter timer when copy, negative upward when paste
	int pendingPaste;// 0 = nothing to paste, 1 = paste on clk, 2 = paste on seq, destination channel in next msbits
	long clockIgnoreOnReset;
	const float clockIgnoreOnResetDuration = 0.001f;// disable clock on powerup and reset for 1 ms (so that the first step plays)
	int lightRefreshCounter;
	
	SchmittTrigger clockTrigger;
	SchmittTrigger resetTrigger;
	SchmittTrigger runningTrigger;
	SchmittTrigger channelTrigger;
	SchmittTrigger stepLTrigger;
	SchmittTrigger stepRTrigger;
	SchmittTrigger copyTrigger;
	SchmittTrigger pasteTrigger;
	SchmittTrigger writeTrigger;
	SchmittTrigger gateTriggers[8];
	SchmittTrigger windowTriggers[4];

	
	WriteSeq32() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void onReset() override {
		running = true;
		indexStep = 0;
		indexStepStage = 0;
		indexChannel = 0;
		for (int s = 0; s < 32; s++) {
			for (int c = 0; c < 4; c++) {
				cv[c][s] = 0.0f;
				gates[c][s] = true;
			}
			cvCPbuffer[s] = 0.0f;
			gateCPbuffer[s] = true;
		}
		infoCopyPaste = 0l;
		pendingPaste = 0;
		clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * engineGetSampleRate());
		resetOnRun = false;
		lightRefreshCounter = 0;
	}

	void onRandomize() override {
		running = true;
		indexStep = 0;
		indexStepStage = 0;
		indexChannel = 0;
		for (int s = 0; s < 32; s++) {
			for (int c = 0; c < 4; c++) {
				cv[c][s] = quantize((randomUniform() *10.0f) - 4.0f, params[QUANTIZE_PARAM].value > 0.5f);
				gates[c][s] = (randomUniform() > 0.5f);
			}
			cvCPbuffer[s] = 0.0f;
			gateCPbuffer[s] = true;
		}
		infoCopyPaste = 0l;
		pendingPaste = 0;
		clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * engineGetSampleRate());
		resetOnRun = false;
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();

		// panelTheme
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));

		// running
		json_object_set_new(rootJ, "running", json_boolean(running));

		// indexStep
		json_object_set_new(rootJ, "indexStep", json_integer(indexStep));

		// indexStepStage
		json_object_set_new(rootJ, "indexStepStage", json_integer(indexStepStage));

		// indexChannel
		json_object_set_new(rootJ, "indexChannel", json_integer(indexChannel));

		// CV
		json_t *cvJ = json_array();
		for (int c = 0; c < 4; c++)
			for (int s = 0; s < 32; s++) {
				json_array_insert_new(cvJ, s + (c<<5), json_real(cv[c][s]));
			}
		json_object_set_new(rootJ, "cv", cvJ);

		// Gates
		json_t *gatesJ = json_array();
		for (int c = 0; c < 4; c++)
			for (int s = 0; s < 32; s++) {
				json_array_insert_new(gatesJ, s + (c<<5), json_integer((int) gates[c][s]));// json_boolean wil break patches
			}
		json_object_set_new(rootJ, "gates", gatesJ);

		// resetOnRun
		json_object_set_new(rootJ, "resetOnRun", json_boolean(resetOnRun));
		
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
		
		// indexStep
		json_t *indexStepJ = json_object_get(rootJ, "indexStep");
		if (indexStepJ)
			indexStep = json_integer_value(indexStepJ);

		// indexStepStage
		json_t *indexStepStageJ = json_object_get(rootJ, "indexStepStage");
		if (indexStepStageJ)
			indexStepStage = json_integer_value(indexStepStageJ);

		// indexChannel
		json_t *indexChannelJ = json_object_get(rootJ, "indexChannel");
		if (indexChannelJ)
			indexChannel = json_integer_value(indexChannelJ);

		// CV
		json_t *cvJ = json_object_get(rootJ, "cv");
		if (cvJ) {
			for (int c = 0; c < 4; c++)
				for (int s = 0; s < 32; s++) {
					json_t *cvArrayJ = json_array_get(cvJ, s + (c<<5));
					if (cvArrayJ)
						cv[c][s] = json_real_value(cvArrayJ);
				}
		}
		
		// Gates
		json_t *gatesJ = json_object_get(rootJ, "gates");
		if (gatesJ) {
			for (int c = 0; c < 4; c++)
				for (int s = 0; s < 32; s++) {
					json_t *gateJ = json_array_get(gatesJ, s + (c<<5));
					if (gateJ)
						gates[c][s] = !!json_integer_value(gateJ);// json_is_true() will break patches
				}
		}
		
		// resetOnRun
		json_t *resetOnRunJ = json_object_get(rootJ, "resetOnRun");
		if (resetOnRunJ)
			resetOnRun = json_is_true(resetOnRunJ);
	}

	
	inline float quantize(float cv, bool enable) {
		return enable ? (roundf(cv * 12.0f) / 12.0f) : cv;
	}
	
	
	// Advances the module by 1 audio frame with duration 1.0 / engineGetSampleRate()
	void step() override {
		static const float copyPasteInfoTime = 0.5f;// seconds
		
		
		//********** Buttons, knobs, switches and inputs **********
		
		int numSteps = (int) clamp(roundf(params[STEPS_PARAM].value), 1.0f, 32.0f);	
		
		// Run state button
		if (runningTrigger.process(params[RUN_PARAM].value + inputs[RUNCV_INPUT].value)) {
			running = !running;
			//pendingPaste = 0;// no pending pastes across run state toggles
			if (running && resetOnRun) {
				indexStep = 0;
				indexStepStage = 0;
			}
			clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * engineGetSampleRate());
		}
		
		// Copy button
		if (copyTrigger.process(params[COPY_PARAM].value)) {
			infoCopyPaste = (long) (copyPasteInfoTime * engineGetSampleRate() / displayRefreshStepSkips);
			for (int s = 0; s < 32; s++) {
				cvCPbuffer[s] = cv[indexChannel][s];
				gateCPbuffer[s] = gates[indexChannel][s];
			}
			pendingPaste = 0;
		}
		// Paste button
		if (pasteTrigger.process(params[PASTE_PARAM].value)) {
			if (params[PASTESYNC_PARAM].value < 0.5f || indexChannel == 3) {
				// Paste realtime, no pending to schedule
				infoCopyPaste = (long) (-1 * copyPasteInfoTime * engineGetSampleRate() / displayRefreshStepSkips);
				for (int s = 0; s < 32; s++) {
					cv[indexChannel][s] = cvCPbuffer[s];
					gates[indexChannel][s] = gateCPbuffer[s];
				}
				pendingPaste = 0;
			}
			else {
				pendingPaste = params[PASTESYNC_PARAM].value > 1.5f ? 2 : 1;
				pendingPaste |= indexChannel<<2; // add paste destination channel into pendingPaste
			}
		}
		
		// Channel selection button
		if (channelTrigger.process(params[CHANNEL_PARAM].value)) {
			indexChannel++;
			if (indexChannel >= 4)
				indexChannel = 0;
		}
		
		// Gate buttons
		for (int index8 = 0, iGate = 0; index8 < 8; index8++) {
			if (gateTriggers[index8].process(params[GATE_PARAM + index8].value)) {
				iGate = ( (indexChannel == 3 ? indexStepStage : indexStep) & 0x18) | index8;
				if (iGate < numSteps)// don't toggle gates beyond steps
					gates[indexChannel][iGate] = !gates[indexChannel][iGate];
			}
		}

		bool canEdit = !running || (indexChannel == 3);
			
		// Steps knob will not trigger anything in step(), and if user goes lower than current step, lower the index accordingly
		if (indexStep >= numSteps)
			indexStep = numSteps - 1;
		if (indexStepStage >= numSteps)
			indexStepStage = numSteps - 1;
		
		// Write button (must be before StepL and StepR in case route gate simultaneously to Step R and Write for example)
		//  (write must be to correct step)
		if (writeTrigger.process(params[WRITE_PARAM].value + inputs[WRITE_INPUT].value)) {
			if (canEdit) {		
				int index = (indexChannel == 3 ? indexStepStage : indexStep);
				// CV
				cv[indexChannel][index] = quantize(inputs[CV_INPUT].value, params[QUANTIZE_PARAM].value > 0.5f);
				// Gate
				if (inputs[GATE_INPUT].active)
					gates[indexChannel][index] = (inputs[GATE_INPUT].value >= 1.0f) ? true : false;
				// Autostep
				if (params[AUTOSTEP_PARAM].value > 0.5f) {
					if (indexChannel == 3)
						indexStepStage = moveIndex(indexStepStage, indexStepStage + 1, numSteps);
					else 
						indexStep = moveIndex(indexStep, indexStep + 1, numSteps);
				}
			}
		}		
		// Step L button
		if (stepLTrigger.process(params[STEPL_PARAM].value + inputs[STEPL_INPUT].value)) {
			if (canEdit) {		
				if (indexChannel == 3)
					indexStepStage = moveIndex(indexStepStage, indexStepStage - 1, numSteps);
				else 
					indexStep = moveIndex(indexStep, indexStep - 1, numSteps);
			}
		}
		// Step R button
		if (stepRTrigger.process(params[STEPR_PARAM].value + inputs[STEPR_INPUT].value)) {
			if (canEdit) {		
				if (indexChannel == 3)
					indexStepStage = moveIndex(indexStepStage, indexStepStage + 1, numSteps);
				else 
					indexStep = moveIndex(indexStep, indexStep + 1, numSteps);
			}
		}
		
		// Window buttons
		for (int i = 0; i < 4; i++) {
			if (windowTriggers[i].process(params[WINDOW_PARAM+i].value)) {
				if (canEdit) {		
					if (indexChannel == 3)
						indexStepStage = (i<<3) | (indexStepStage&0x7);
					else 
						indexStep = (i<<3) | (indexStep&0x7);
				}
			}
		}
		
		
		//********** Clock and reset **********
		
		// Clock
		if (clockTrigger.process(inputs[CLOCK_INPUT].value)) {
			if (running && clockIgnoreOnReset == 0l) {
				indexStep = moveIndex(indexStep, indexStep + 1, numSteps);
				
				// Pending paste on clock or end of seq
				if ( ((pendingPaste&0x3) == 1) || ((pendingPaste&0x3) == 2 && indexStep == 0) ) {
					int pasteChannel = pendingPaste>>2;
					infoCopyPaste = (long) (-1 * copyPasteInfoTime * engineGetSampleRate() / displayRefreshStepSkips);
					for (int s = 0; s < 32; s++) {
						cv[pasteChannel][s] = cvCPbuffer[s];
						gates[pasteChannel][s] = gateCPbuffer[s];
					}
					pendingPaste = 0;
				}
			}
		}
		
		// Reset
		if (resetTrigger.process(inputs[RESET_INPUT].value)) {
			indexStep = 0;
			indexStepStage = 0;	
			pendingPaste = 0;
			//indexChannel = 0;
			clockTrigger.reset();
			clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * engineGetSampleRate());
		}		
		
		
		//********** Outputs and lights **********
		
		// CV and gate outputs (staging area not used)
		if (running) {
			for (int i = 0; i < 3; i++) {
				outputs[CV_OUTPUTS + i].value = cv[i][indexStep];
				outputs[GATE_OUTPUTS + i].value = (clockTrigger.isHigh() && gates[i][indexStep]) ? 10.0f : 0.0f;
			}
		}
		else {			
			bool muteGate = false;// (params[WRITE_PARAM].value + params[STEPL_PARAM].value + params[STEPR_PARAM].value) > 0.5f; // set to false if don't want mute gate on button push
			for (int i = 0; i < 3; i++) {
				// CV
				if (params[MONITOR_PARAM].value > 0.5f)
					outputs[CV_OUTPUTS + i].value = cv[i][indexStep];// each CV out monitors the current step CV of that channel
				else
					outputs[CV_OUTPUTS + i].value = quantize(inputs[CV_INPUT].value, params[QUANTIZE_PARAM].value > 0.5f);// all CV outs monitor the CV in (only current channel will have a gate though)
				
				// Gate
				outputs[GATE_OUTPUTS + i].value = ((i == indexChannel) && !muteGate) ? 10.0f : 0.0f;
			}
		}

		lightRefreshCounter++;
		if (lightRefreshCounter > displayRefreshStepSkips) {
			lightRefreshCounter = 0;

			int index = (indexChannel == 3 ? indexStepStage : indexStep);
			// Window lights
			for (int i = 0; i < 4; i++) {
				lights[WINDOW_LIGHTS + i].value = ((i == (index >> 3))?1.0f:0.0f);
			}
			// Step and gate lights
			for (int index8 = 0, iGate = 0; index8 < 8; index8++) {
				lights[STEP_LIGHTS + index8].value = (index8 == (index&0x7)) ? 1.0f : 0.0f;
				iGate = (index&0x18) | index8;
				lights[GATE_LIGHTS + index8].value = (gates[indexChannel][iGate] && iGate < numSteps) ? 1.0f : 0.0f;
			}
				
			// Channel lights		
			lights[CHANNEL_LIGHTS + 0].value = (indexChannel == 0) ? 1.0f : 0.0f;// green
			lights[CHANNEL_LIGHTS + 1].value = (indexChannel == 1) ? 1.0f : 0.0f;// yellow
			lights[CHANNEL_LIGHTS + 2].value = (indexChannel == 2) ? 1.0f : 0.0f;// orange
			lights[CHANNEL_LIGHTS + 3].value = (indexChannel == 3) ? 1.0f : 0.0f;// blue
			
			// Run light
			lights[RUN_LIGHT].value = running ? 1.0f : 0.0f;
			
			// Write allowed light
			lights[WRITE_LIGHT + 0].value = (canEdit)?1.0f:0.0f;
			lights[WRITE_LIGHT + 1].value = (canEdit)?0.0f:1.0f;
			
			// Pending paste light
			lights[PENDING_LIGHT].value = (pendingPaste == 0 ? 0.0f : 1.0f);
			
			if (infoCopyPaste != 0l) {
				if (infoCopyPaste > 0l)
					infoCopyPaste --;
				if (infoCopyPaste < 0l)
					infoCopyPaste ++;
			}
		}// lightRefreshCounter
		
		if (clockIgnoreOnReset > 0l)
			clockIgnoreOnReset--;
	}
};


struct WriteSeq32Widget : ModuleWidget {

	struct NotesDisplayWidget : TransparentWidget {
		WriteSeq32 *module;
		std::shared_ptr<Font> font;
		char text[4];

		NotesDisplayWidget() {
			font = Font::load(assetPlugin(plugin, "res/fonts/Segment14.ttf"));
		}
		
		void cvToStr(int index8) {
		if (module->infoCopyPaste != 0l) {
			if (index8 == 0) {
				if (module->infoCopyPaste > 0l)
					snprintf(text, 4, "COP");			
				else 
					snprintf(text, 4, "PAS");
			}
			else if (index8 == 1) {
				if (module->infoCopyPaste > 0l)
					snprintf(text, 4, "Y  ");			
				else 
					snprintf(text, 4, "TE ");
			}
			else {
				snprintf(text, 4, "   ");
			}
		}
		else {
			int index = (module->indexChannel == 3 ? module->indexStepStage : module->indexStep);
			if ( ( (index&0x18) |index8) >= (int) clamp(roundf(module->params[WriteSeq32::STEPS_PARAM].value), 1.0f, 32.0f) ) {
				text[0] = ' ';
				text[1] = ' ';
				text[2] = ' ';
			}
			else {
				float cvVal = module->cv[module->indexChannel][index8|(index&0x18)];
				float cvValOffset = cvVal +10.0f;//to properly handle negative note voltages
				int indexNote = (int) clamp(  roundf( (cvValOffset-floor(cvValOffset)) * 12.0f ),  0.0f,  11.0f);
				bool sharp = (module->params[WriteSeq32::SHARP_PARAM].value > 0.5f) ? true : false;
				
				// note letter
				text[0] = sharp ? noteLettersSharp[indexNote] : noteLettersFlat[indexNote];
				
				// octave number
				int octave = (int) roundf(floorf(cvVal)+4.0f);
				if (octave < 0 || octave > 9)
					text[1] = (octave > 9) ? ':' : '_';
				else
					text[1] = (char) ( 0x30 + octave);
				
				// sharp/flat
				text[2] = ' ';
				if (isBlackKey[indexNote] == 1)
					text[2] = (sharp ? '\"' : '^' );
			}
		}
			// end of string
			text[3] = 0;
		}

		void draw(NVGcontext *vg) override {
			NVGcolor textColor = prepareDisplay(vg, &box);
			nvgFontFaceId(vg, font->handle);
			nvgTextLetterSpacing(vg, -1.5);

			for (int i = 0; i < 8; i++) {
				Vec textPos = Vec(module->notesPos[i], 24);
				nvgFillColor(vg, nvgTransRGBA(textColor, 16));
				nvgText(vg, textPos.x, textPos.y, "~~~", NULL);
				nvgFillColor(vg, textColor);
				cvToStr(i);
				nvgText(vg, textPos.x, textPos.y, text, NULL);
			}
		}
	};


	struct StepsDisplayWidget : TransparentWidget {
		float *valueKnob;
		std::shared_ptr<Font> font;
		
		StepsDisplayWidget() {
			font = Font::load(assetPlugin(plugin, "res/fonts/Segment14.ttf"));
		}

		void draw(NVGcontext *vg) override {
			NVGcolor textColor = prepareDisplay(vg, &box);
			nvgFontFaceId(vg, font->handle);
			//nvgTextLetterSpacing(vg, 2.5);

			Vec textPos = Vec(6, 24);
			nvgFillColor(vg, nvgTransRGBA(textColor, 16));
			nvgText(vg, textPos.x, textPos.y, "~~", NULL);
			nvgFillColor(vg, textColor);
			char displayStr[3];
			snprintf(displayStr, 3, "%2u", (unsigned) clamp(roundf(*valueKnob), 1.0f, 32.0f) );
			nvgText(vg, textPos.x, textPos.y, displayStr, NULL);
		}
	};

	
	struct PanelThemeItem : MenuItem {
		WriteSeq32 *module;
		int theme;
		void onAction(EventAction &e) override {
			module->panelTheme = theme;
		}
		void step() override {
			rightText = (module->panelTheme == theme) ? "✔" : "";
		}
	};
	struct ResetOnRunItem : MenuItem {
		WriteSeq32 *module;
		void onAction(EventAction &e) override {
			module->resetOnRun = !module->resetOnRun;
		}
	};
	Menu *createContextMenu() override {
		Menu *menu = ModuleWidget::createContextMenu();

		MenuLabel *spacerLabel = new MenuLabel();
		menu->addChild(spacerLabel);

		WriteSeq32 *module = dynamic_cast<WriteSeq32*>(this->module);
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

		menu->addChild(new MenuLabel());// empty line
		
		MenuLabel *settingsLabel = new MenuLabel();
		settingsLabel->text = "Settings";
		menu->addChild(settingsLabel);
		
		ResetOnRunItem *rorItem = MenuItem::create<ResetOnRunItem>("Reset on Run", CHECKMARK(module->resetOnRun));
		rorItem->module = module;
		menu->addChild(rorItem);
		
		return menu;
	}	
	
	
	WriteSeq32Widget(WriteSeq32 *module) : ModuleWidget(module) {
		// Main panel from Inkscape
        DynamicSVGPanel *panel = new DynamicSVGPanel();
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/WriteSeq32.svg")));
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/dark/WriteSeq32_dark.svg")));
        box.size = panel->box.size;
        panel->mode = &module->panelTheme;
        addChild(panel);

		// Screws
		addChild(createDynamicScrew<IMScrew>(Vec(15, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(box.size.x-30, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(15, 365), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(box.size.x-30, 365), &module->panelTheme));

		// Column rulers (horizontal positions)
		static const int columnRuler0 = 25;
		static const int columnnRulerStep = 69;
		static const int columnRuler1 = columnRuler0 + columnnRulerStep;
		static const int columnRuler2 = columnRuler1 + columnnRulerStep;
		static const int columnRuler3 = columnRuler2 + columnnRulerStep;
		static const int columnRuler4 = columnRuler3 + columnnRulerStep;
		static const int columnRuler5 = columnRuler4 + columnnRulerStep - 15;
		
		// Row rulers (vertical positions)
		static const int rowRuler0 = 172;
		static const int rowRulerStep = 49;
		static const int rowRuler1 = rowRuler0 + rowRulerStep;
		static const int rowRuler2 = rowRuler1 + rowRulerStep;
		static const int rowRuler3 = rowRuler2 + rowRulerStep + 4;


		// ****** Top portion ******
		
		static const int yRulerTopLEDs = 42;
		static const int yRulerTopSwitches = yRulerTopLEDs-11;
		
		// Autostep, sharp/flat and quantize switches
		// Autostep	
		addParam(ParamWidget::create<CKSS>(Vec(columnRuler0+3+hOffsetCKSS, yRulerTopSwitches+vOffsetCKSS), module, WriteSeq32::AUTOSTEP_PARAM, 0.0f, 1.0f, 1.0f));
		// Sharp/flat
		addParam(ParamWidget::create<CKSS>(Vec(columnRuler4+hOffsetCKSS, yRulerTopSwitches+vOffsetCKSS), module, WriteSeq32::SHARP_PARAM, 0.0f, 1.0f, 1.0f));
		// Quantize
		addParam(ParamWidget::create<CKSS>(Vec(columnRuler5+hOffsetCKSS, yRulerTopSwitches+vOffsetCKSS), module, WriteSeq32::QUANTIZE_PARAM, 0.0f, 1.0f, 1.0f));

		// Window LED buttons
		static const float wLightsPosX = 140.0f;
		static const float wLightsIntX = 35.0f;
		for (int i = 0; i < 4; i++) {
			addParam(ParamWidget::create<LEDButton>(Vec(wLightsPosX + i * wLightsIntX, yRulerTopLEDs - 4.4f), module, WriteSeq32::WINDOW_PARAM + i, 0.0f, 1.0f, 0.0f));
			addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(wLightsPosX + 4.4f + i * wLightsIntX, yRulerTopLEDs), module, WriteSeq32::WINDOW_LIGHTS + i));
		}
		
		// Prepare 8 positions for step lights, gate lights and notes display
		module->notesPos[0] = 9;// this is also used to help line up LCD digits with LEDbuttons and avoid bad horizontal scaling with long str in display  
		for (int i = 1; i < 8; i++) {
			module->notesPos[i] = module->notesPos[i-1] + 46;
		}

		// Notes display
		NotesDisplayWidget *displayNotes = new NotesDisplayWidget();
		displayNotes->box.pos = Vec(12, 76);
		displayNotes->box.size = Vec(381, 30);
		displayNotes->module = module;
		addChild(displayNotes);

		// Step LEDs (must be done after Notes display such that LED glow will overlay the notes display
		static const int yRulerStepLEDs = 65;
		for (int i = 0; i < 8; i++) {
			addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(module->notesPos[i]+25.0f+1.5f, yRulerStepLEDs), module, WriteSeq32::STEP_LIGHTS + i));
		}

		// Gates LED buttons
		static const int yRulerT2 = 119.0f;
		for (int i = 0; i < 8; i++) {
			addParam(ParamWidget::create<LEDButton>(Vec(module->notesPos[i]+25.0f-4.4f, yRulerT2-4.4f), module, WriteSeq32::GATE_PARAM + i, 0.0f, 1.0f, 0.0f));
			addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(module->notesPos[i]+25.0f, yRulerT2), module, WriteSeq32::GATE_LIGHTS + i));
		}
		
		
		// ****** Bottom portion ******
		
		// Column 0
		// Channel button
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRuler0+offsetCKD6b, rowRuler0+offsetCKD6b), module, WriteSeq32::CHANNEL_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Channel LEDS
		static const int chanLEDoffsetX = 25;
		static const int chanLEDoffsetY[4] = {-20, -8, 4, 16};
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(columnRuler0 + chanLEDoffsetX + offsetMediumLight, rowRuler0 - 4 + chanLEDoffsetY[0] + offsetMediumLight), module, WriteSeq32::CHANNEL_LIGHTS + 0));
		addChild(ModuleLightWidget::create<MediumLight<YellowLight>>(Vec(columnRuler0 + chanLEDoffsetX + offsetMediumLight, rowRuler0 - 4 + chanLEDoffsetY[1] + offsetMediumLight), module, WriteSeq32::CHANNEL_LIGHTS + 1));
		addChild(ModuleLightWidget::create<MediumLight<OrangeLight>>(Vec(columnRuler0 + chanLEDoffsetX + offsetMediumLight, rowRuler0 - 4 + chanLEDoffsetY[2] + offsetMediumLight), module, WriteSeq32::CHANNEL_LIGHTS + 2));
		addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(columnRuler0 + chanLEDoffsetX + offsetMediumLight, rowRuler0 - 4 + chanLEDoffsetY[3] + offsetMediumLight), module, WriteSeq32::CHANNEL_LIGHTS + 3));
		// Copy/paste switches
		addParam(ParamWidget::create<TL1105>(Vec(columnRuler0-10, rowRuler1+offsetTL1105), module, WriteSeq32::COPY_PARAM, 0.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<TL1105>(Vec(columnRuler0+20, rowRuler1+offsetTL1105), module, WriteSeq32::PASTE_PARAM, 0.0f, 1.0f, 0.0f));
		// Paste sync (and light)
		addParam(ParamWidget::create<CKSSThreeInv>(Vec(columnRuler0+hOffsetCKSS, rowRuler2+vOffsetCKSSThree), module, WriteSeq32::PASTESYNC_PARAM, 0.0f, 2.0f, 0.0f));	
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(columnRuler0 + 41, rowRuler2 + 14), module, WriteSeq32::PENDING_LIGHT));		
		// Run CV input
		addInput(createDynamicPort<IMPort>(Vec(columnRuler0, rowRuler3), Port::INPUT, module, WriteSeq32::RUNCV_INPUT, &module->panelTheme));
		
		
		// Column 1
		// Step L button
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRuler1+offsetCKD6b, rowRuler0+offsetCKD6b), module, WriteSeq32::STEPL_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Run LED bezel and light
		addParam(ParamWidget::create<LEDBezel>(Vec(columnRuler1+offsetLEDbezel, rowRuler1+offsetLEDbezel), module, WriteSeq32::RUN_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(Vec(columnRuler1+offsetLEDbezel+offsetLEDbezelLight, rowRuler1+offsetLEDbezel+offsetLEDbezelLight), module, WriteSeq32::RUN_LIGHT));
		// Gate input
		addInput(createDynamicPort<IMPort>(Vec(columnRuler1, rowRuler2), Port::INPUT, module, WriteSeq32::GATE_INPUT, &module->panelTheme));		
		// Step L input
		addInput(createDynamicPort<IMPort>(Vec(columnRuler1, rowRuler3), Port::INPUT, module, WriteSeq32::STEPL_INPUT, &module->panelTheme));
		
		
		// Column 2
		// Step R button
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRuler2+offsetCKD6b, rowRuler0+offsetCKD6b), module, WriteSeq32::STEPR_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));	
		// Write button and light
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRuler2+offsetCKD6b, rowRuler1+offsetCKD6b), module, WriteSeq32::WRITE_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(columnRuler2 -12, rowRuler1 - 12), module, WriteSeq32::WRITE_LIGHT));
		// CV input
		addInput(createDynamicPort<IMPort>(Vec(columnRuler2, rowRuler2), Port::INPUT, module, WriteSeq32::CV_INPUT, &module->panelTheme));		
		// Step R input
		addInput(createDynamicPort<IMPort>(Vec(columnRuler2, rowRuler3), Port::INPUT, module, WriteSeq32::STEPR_INPUT, &module->panelTheme));
		
		
		// Column 3
		// Steps display
		StepsDisplayWidget *displaySteps = new StepsDisplayWidget();
		displaySteps->box.pos = Vec(columnRuler3-7, rowRuler0+vOffsetDisplay);
		displaySteps->box.size = Vec(40, 30);// 2 characters
		displaySteps->valueKnob = &module->params[WriteSeq32::STEPS_PARAM].value;
		addChild(displaySteps);
		// Steps knob
		addParam(createDynamicParam<IMBigSnapKnob>(Vec(columnRuler3+offsetIMBigKnob, rowRuler1+offsetIMBigKnob), module, WriteSeq32::STEPS_PARAM, 1.0f, 32.0f, 32.0f, &module->panelTheme));		
		// Monitor
		addParam(ParamWidget::create<CKSSH>(Vec(columnRuler3+hOffsetCKSSH, rowRuler2+vOffsetCKSSH), module, WriteSeq32::MONITOR_PARAM, 0.0f, 1.0f, 0.0f));		
		// Write input
		addInput(createDynamicPort<IMPort>(Vec(columnRuler3, rowRuler3), Port::INPUT, module, WriteSeq32::WRITE_INPUT, &module->panelTheme));
		
		
		// Column 4
		// Outputs
		addOutput(createDynamicPort<IMPort>(Vec(columnRuler4, rowRuler0), Port::OUTPUT, module, WriteSeq32::CV_OUTPUTS + 0, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRuler4, rowRuler1), Port::OUTPUT, module, WriteSeq32::CV_OUTPUTS + 1, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRuler4, rowRuler2), Port::OUTPUT, module, WriteSeq32::CV_OUTPUTS + 2, &module->panelTheme));
		// Reset
		addInput(createDynamicPort<IMPort>(Vec(columnRuler4, rowRuler3), Port::INPUT, module, WriteSeq32::RESET_INPUT, &module->panelTheme));		

		
		// Column 5
		// Gates
		addOutput(createDynamicPort<IMPort>(Vec(columnRuler5, rowRuler0), Port::OUTPUT, module, WriteSeq32::GATE_OUTPUTS + 0, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRuler5, rowRuler1), Port::OUTPUT, module, WriteSeq32::GATE_OUTPUTS + 1, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRuler5, rowRuler2), Port::OUTPUT, module, WriteSeq32::GATE_OUTPUTS + 2, &module->panelTheme));
		// Clock
		addInput(createDynamicPort<IMPort>(Vec(columnRuler5, rowRuler3), Port::INPUT, module, WriteSeq32::CLOCK_INPUT, &module->panelTheme));			
	}
};

} // namespace rack_plugin_ImpromptuModular

using namespace rack_plugin_ImpromptuModular;

RACK_PLUGIN_MODEL_INIT(ImpromptuModular, WriteSeq32) {
   Model *modelWriteSeq32 = Model::create<WriteSeq32, WriteSeq32Widget>("Impromptu Modular", "Write-Seq-32", "SEQ - Write-Seq-32", SEQUENCER_TAG);
   return modelWriteSeq32;
}

/*CHANGE LOG

0.6.7:
no reset on run by default, with switch added in context menu
reset does not revert chan number to 1
*/
