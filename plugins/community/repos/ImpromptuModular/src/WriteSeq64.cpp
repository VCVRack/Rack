//***********************************************************************************************
//Four channel 64-step writable sequencer module for VCV Rack by Marc Boulé
//
//Based on code from the Fundamental and AudibleInstruments plugins by Andrew Belt 
//and graphics from the Component Library by Wes Milholen 
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//***********************************************************************************************


#include "ImpromptuModular.hpp"

namespace rack_plugin_ImpromptuModular {

struct WriteSeq64 : Module {
	enum ParamIds {
		SHARP_PARAM,
		QUANTIZE_PARAM,
		GATE_PARAM,
		CHANNEL_PARAM,
		COPY_PARAM,
		PASTE_PARAM,
		RUN_PARAM,
		WRITE_PARAM,
		STEPL_PARAM,
		MONITOR_PARAM,
		STEPR_PARAM,
		STEPS_PARAM,
		STEP_PARAM,
		AUTOSTEP_PARAM,
		RESET_PARAM,
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
		CLOCK12_INPUT,
		CLOCK34_INPUT,
		RESET_INPUT,
		// -- 0.6.2
		RUNCV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(CV_OUTPUTS, 4),
		ENUMS(GATE_OUTPUTS, 4),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(GATE_LIGHT, 2),// room for GreenRed
		RUN_LIGHT,
		RESET_LIGHT,
		ENUMS(WRITE_LIGHT, 2),// room for GreenRed
		PENDING_LIGHT,
		NUM_LIGHTS
	};

	// Need to save
	int panelTheme = 0;
	bool running;
	//int indexChannel;
	int indexStep[5];// [0;63] each
	int indexSteps[5];// [1;64] each
	float cv[5][64];
	int gates[5][64];
	bool resetOnRun;

	// No need to save
	float cvCPbuffer[64];// copy paste buffer for CVs
	int gateCPbuffer[64];// copy paste buffer for gates
	int stepsCPbuffer;
	long infoCopyPaste;// 0 when no info, positive downward step counter timer when copy, negative upward when paste
	int pendingPaste;// 0 = nothing to paste, 1 = paste on clk, 2 = paste on seq, destination channel in next msbits
	long clockIgnoreOnReset;


	unsigned int lightRefreshCounter = 0;	
	int stepKnob = 0;
	int stepsKnob = 0;
	float resetLight = 0.0f;
	Trigger clock12Trigger;
	Trigger clock34Trigger;
	Trigger resetTrigger;
	Trigger runningTrigger;
	Trigger stepLTrigger;
	Trigger stepRTrigger;
	Trigger copyTrigger;
	Trigger pasteTrigger;
	Trigger writeTrigger;
	Trigger gateTrigger;

	
	inline float quantize(float cv, bool enable) {
		return enable ? (roundf(cv * 12.0f) / 12.0f) : cv;
	}
	inline int calcChan() {
		return clamp((int)(params[CHANNEL_PARAM].value + 0.5f), 0, 4);
	}


	WriteSeq64() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	
	void onReset() override {
		running = true;
		//indexChannel = 0;
		for (int c = 0; c < 5; c++) {
			indexStep[c] = 0;
			indexSteps[c] = 64;
			for (int s = 0; s < 64; s++) {
				cv[c][s] = 0.0f;
				gates[c][s] = 1;
			}
		}
		for (int s = 0; s < 64; s++) {
			cvCPbuffer[s] = 0.0f;
			gateCPbuffer[s] = 1;
		}
		stepsCPbuffer = 64;
		infoCopyPaste = 0l;
		pendingPaste = 0;
		clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * engineGetSampleRate());
		resetOnRun = false;
	}

	
	void onRandomize() override {
		int indexChannel = calcChan();
		// for (int c = 0; c < 5; c++) {
			// indexStep[c] = 0;
			// indexSteps[c] = 64;
			// for (int s = 0; s < 64; s++) {
				// cv[c][s] = quantize((randomUniform() *10.0f) - 4.0f, params[QUANTIZE_PARAM].value > 0.5f);
				// gates[c][s] = (randomUniform() > 0.5f) ? 1 : 0;
			// }
		// }
		// for (int s = 0; s < 64; s++) {
			// cvCPbuffer[s] = 0.0f;
			// gateCPbuffer[s] = 1;
		// }
		// stepsCPbuffer = 64;
		for (int s = 0; s < 64; s++) {
			cv[indexChannel][s] = quantize((randomUniform() *10.0f) - 4.0f, params[QUANTIZE_PARAM].value > 0.5f);
			gates[indexChannel][s] = (randomUniform() > 0.5f) ? 1 : 0;
		}		
		pendingPaste = 0;
	}

	
	json_t *toJson() override {
		json_t *rootJ = json_object();

		// panelTheme
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));

		// running
		json_object_set_new(rootJ, "running", json_boolean(running));

		// indexChannel
		//json_object_set_new(rootJ, "indexChannel", json_integer(indexChannel));
		
		// indexStep
		json_t *indexStepJ = json_array();
		for (int c = 0; c < 5; c++)
			json_array_insert_new(indexStepJ, c, json_integer(indexStep[c]));
		json_object_set_new(rootJ, "indexStep", indexStepJ);

		// indexSteps 
		json_t *indexStepsJ = json_array();
		for (int c = 0; c < 5; c++)
			json_array_insert_new(indexStepsJ, c, json_integer(indexSteps[c]));
		json_object_set_new(rootJ, "indexSteps", indexStepsJ);

		// CV
		json_t *cvJ = json_array();
		for (int c = 0; c < 5; c++)
			for (int s = 0; s < 64; s++) {
				json_array_insert_new(cvJ, s + (c<<6), json_real(cv[c][s]));
			}
		json_object_set_new(rootJ, "cv", cvJ);

		// gates
		json_t *gatesJ = json_array();
		for (int c = 0; c < 5; c++)
			for (int s = 0; s < 64; s++) {
				json_array_insert_new(gatesJ, s + (c<<6), json_integer(gates[c][s]));
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
		
		// indexChannel
		// json_t *indexChannelJ = json_object_get(rootJ, "indexChannel");
		// if (indexChannelJ)
			// indexChannel = json_integer_value(indexChannelJ);
		
		// indexStep
		json_t *indexStepJ = json_object_get(rootJ, "indexStep");
		if (indexStepJ)
			for (int c = 0; c < 5; c++)
			{
				json_t *indexStepArrayJ = json_array_get(indexStepJ, c);
				if (indexStepArrayJ)
					indexStep[c] = json_integer_value(indexStepArrayJ);
			}

		// indexSteps
		json_t *indexStepsJ = json_object_get(rootJ, "indexSteps");
		if (indexStepsJ)
			for (int c = 0; c < 5; c++)
			{
				json_t *indexStepsArrayJ = json_array_get(indexStepsJ, c);
				if (indexStepsArrayJ)
					indexSteps[c] = json_integer_value(indexStepsArrayJ);
			}

		// CV
		json_t *cvJ = json_object_get(rootJ, "cv");
		if (cvJ) {
			for (int c = 0; c < 5; c++)
				for (int i = 0; i < 64; i++) {
					json_t *cvArrayJ = json_array_get(cvJ, i + (c<<6));
					if (cvArrayJ)
						cv[c][i] = json_number_value(cvArrayJ);
				}
		}
		
		// gates
		json_t *gatesJ = json_object_get(rootJ, "gates");
		if (gatesJ) {
			for (int c = 0; c < 5; c++)
				for (int i = 0; i < 64; i++) {
					json_t *gateJ = json_array_get(gatesJ, i + (c<<6));
					if (gateJ)
						gates[c][i] = json_integer_value(gateJ);
				}
		}
		
		// resetOnRun
		json_t *resetOnRunJ = json_object_get(rootJ, "resetOnRun");
		if (resetOnRunJ)
			resetOnRun = json_is_true(resetOnRunJ);
	}
	
	
	void step() override {
		static const float copyPasteInfoTime = 0.7f;// seconds
		
		
		//********** Buttons, knobs, switches and inputs **********
		int indexChannel = calcChan();
		bool canEdit = !running || (indexChannel == 4);
		
		// Run state button
		if (runningTrigger.process(params[RUN_PARAM].value + inputs[RUNCV_INPUT].value)) {// no input refresh here, don't want to introduce startup skew
			running = !running;
			//pendingPaste = 0;// no pending pastes across run state toggles
			if (running) {
				if (resetOnRun) {
					for (int c = 0; c < 5; c++) 
						indexStep[c] = 0;
				}
				if (resetOnRun || clockIgnoreOnRun)
					clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * engineGetSampleRate());
			}
		}
	
		if ((lightRefreshCounter & userInputsStepSkipMask) == 0) {
		
			// Copy button
			if (copyTrigger.process(params[COPY_PARAM].value)) {
				infoCopyPaste = (long) (copyPasteInfoTime * engineGetSampleRate() / displayRefreshStepSkips);
				for (int s = 0; s < 64; s++) {
					cvCPbuffer[s] = cv[indexChannel][s];
					gateCPbuffer[s] = gates[indexChannel][s];
				}
				stepsCPbuffer = indexSteps[indexChannel];
				pendingPaste = 0;
			}
			// Paste button
			if (pasteTrigger.process(params[PASTE_PARAM].value)) {
				if (params[PASTESYNC_PARAM].value < 0.5f || indexChannel == 4) {
					// Paste realtime, no pending to schedule
					infoCopyPaste = (long) (-1 * copyPasteInfoTime * engineGetSampleRate() / displayRefreshStepSkips);
					for (int s = 0; s < 64; s++) {
						cv[indexChannel][s] = cvCPbuffer[s];
						gates[indexChannel][s] = gateCPbuffer[s];
					}
					indexSteps[indexChannel] = stepsCPbuffer;
					if (indexStep[indexChannel] >= stepsCPbuffer)
						indexStep[indexChannel] = stepsCPbuffer - 1;
					pendingPaste = 0;
				}
				else {
					pendingPaste = params[PASTESYNC_PARAM].value > 1.5f ? 2 : 1;
					pendingPaste |= indexChannel<<2; // add paste destination channel into pendingPaste				
				}
			}
				
			// Gate button
			if (gateTrigger.process(params[GATE_PARAM].value)) {
				if (params[GATE_PARAM].value > 1.5f) {// right button click
					gates[indexChannel][indexStep[indexChannel]] = 0;
				}
				else {
					gates[indexChannel][indexStep[indexChannel]]++;
					if (gates[indexChannel][indexStep[indexChannel]] > 2)
						gates[indexChannel][indexStep[indexChannel]] = 0;
				}
			}
			
			// Steps knob
			float stepsParamValue = params[STEPS_PARAM].value;
			int newStepsKnob = (int)roundf(stepsParamValue * 10.0f);
			if (stepsParamValue == 0.0f)// true when constructor or fromJson() occured
				stepsKnob = newStepsKnob;
			if (newStepsKnob != stepsKnob) {
				if (abs(newStepsKnob - stepsKnob) <= 3) // avoid discontinuous step (initialize for example)
					indexSteps[indexChannel] = clamp( indexSteps[indexChannel] + newStepsKnob - stepsKnob, 1, 64); 
				stepsKnob = newStepsKnob;
			}	
			// Step knob
			float stepParamValue = params[STEP_PARAM].value;
			int newStepKnob = (int)roundf(stepParamValue * 10.0f);
			if (stepParamValue == 0.0f)// true when constructor or fromJson() occured
				stepKnob = newStepKnob;
			if (newStepKnob != stepKnob) {
				if (canEdit && (abs(newStepKnob - stepKnob) <= 3) ) // avoid discontinuous step (initialize for example)
					indexStep[indexChannel] = moveIndex(indexStep[indexChannel], indexStep[indexChannel] + newStepKnob - stepKnob, indexSteps[indexChannel]);
				stepKnob = newStepKnob;// must do this step whether running or not
			}	
			// If steps knob goes down past step, step knob will not get triggered above, so reduce accordingly
			for (int c = 0; c < 5; c++)
				if (indexStep[c] >= indexSteps[c])
					indexStep[c] = indexSteps[c] - 1;
			
			// Write button and input (must be before StepL and StepR in case route gate simultaneously to Step R and Write for example)
			//  (write must be to correct step)
			if (writeTrigger.process(params[WRITE_PARAM].value + inputs[WRITE_INPUT].value)) {
				if (canEdit) {		
					// CV
					cv[indexChannel][indexStep[indexChannel]] = quantize(inputs[CV_INPUT].value, params[QUANTIZE_PARAM].value > 0.5f);
					// Gate
					if (inputs[GATE_INPUT].active)
						gates[indexChannel][indexStep[indexChannel]] = (inputs[GATE_INPUT].value >= 1.0f) ? 1 : 0;
					// Autostep
					if (params[AUTOSTEP_PARAM].value > 0.5f)
						indexStep[indexChannel] = moveIndex(indexStep[indexChannel], indexStep[indexChannel] + 1, indexSteps[indexChannel]);
				}
			}
			// Step L button
			if (stepLTrigger.process(params[STEPL_PARAM].value + inputs[STEPL_INPUT].value)) {
				if (canEdit) {		
					indexStep[indexChannel] = moveIndex(indexStep[indexChannel], indexStep[indexChannel] - 1, indexSteps[indexChannel]); 
				}
			}
			// Step R button
			if (stepRTrigger.process(params[STEPR_PARAM].value + inputs[STEPR_INPUT].value)) {
				if (canEdit) {		
					indexStep[indexChannel] = moveIndex(indexStep[indexChannel], indexStep[indexChannel] + 1, indexSteps[indexChannel]); 
				}
			}
		
		}// userInputs refresh
		
		
		
		//********** Clock and reset **********
		
		// Clock
		if (running && clockIgnoreOnReset == 0l) {
			bool clk12step = clock12Trigger.process(inputs[CLOCK12_INPUT].value);
			bool clk34step = ((!inputs[CLOCK34_INPUT].active) && clk12step) || 
							  clock34Trigger.process(inputs[CLOCK34_INPUT].value);
			if (clk12step) {
				indexStep[0] = moveIndex(indexStep[0], indexStep[0] + 1, indexSteps[0]);
				indexStep[1] = moveIndex(indexStep[1], indexStep[1] + 1, indexSteps[1]);
			}
			if (clk34step) {
				indexStep[2] = moveIndex(indexStep[2], indexStep[2] + 1, indexSteps[2]);
				indexStep[3] = moveIndex(indexStep[3], indexStep[3] + 1, indexSteps[3]);
			}	

			// Pending paste on clock or end of seq
			if ( ((pendingPaste&0x3) == 1) || ((pendingPaste&0x3) == 2 && indexStep[indexChannel] == 0) ) {
				if ( (clk12step && (indexChannel == 0 || indexChannel == 1)) ||
					 (clk34step && (indexChannel == 2 || indexChannel == 3)) ) {
					infoCopyPaste = (long) (-1 * copyPasteInfoTime * engineGetSampleRate() / displayRefreshStepSkips);
					int pasteChannel = pendingPaste>>2;
					for (int s = 0; s < 64; s++) {
						cv[pasteChannel][s] = cvCPbuffer[s];
						gates[pasteChannel][s] = gateCPbuffer[s];
					}
					indexSteps[pasteChannel] = stepsCPbuffer;
					if (indexStep[pasteChannel] >= stepsCPbuffer)
						indexStep[pasteChannel] = stepsCPbuffer - 1;
					pendingPaste = 0;
				}
			}
		}
		
		// Reset
		if (resetTrigger.process(inputs[RESET_INPUT].value + params[RESET_PARAM].value)) {
			for (int t = 0; t < 5; t++)
				indexStep[t] = 0;
			resetLight = 1.0f;
			pendingPaste = 0;
			clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * engineGetSampleRate());
			clock12Trigger.reset();
			clock34Trigger.reset();
		}
		
		
		//********** Outputs and lights **********
		
		// CV and gate outputs (staging area not used)
		if (running) {
			bool clockHigh = false;
			bool retriggingOnReset = (clockIgnoreOnReset != 0l && retrigGatesOnReset);
			for (int i = 0; i < 4; i++) {
				outputs[CV_OUTPUTS + i].value = cv[i][indexStep[i]];
				clockHigh = i < 2 ? clock12Trigger.isHigh() : clock34Trigger.isHigh();
				outputs[GATE_OUTPUTS + i].value = ( (((gates[i][indexStep[i]] == 1) && clockHigh) || gates[i][indexStep[i]] == 2) && !retriggingOnReset ) ? 10.0f : 0.0f;
			}
		}
		else {
			bool muteGate = false;// (params[WRITE_PARAM].value + params[STEPL_PARAM].value + params[STEPR_PARAM].value) > 0.5f; // set to false if don't want mute gate on button push
			for (int i = 0; i < 4; i++) {
				// CV
				if (params[MONITOR_PARAM].value > 0.5f)
					outputs[CV_OUTPUTS + i].value = cv[i][indexStep[i]];// each CV out monitors the current step CV of that channel
				else
					outputs[CV_OUTPUTS + i].value = quantize(inputs[CV_INPUT].value, params[QUANTIZE_PARAM].value > 0.5f);// all CV outs monitor the CV in (only current channel will have a gate though)
				
				// Gate
				outputs[GATE_OUTPUTS + i].value = ((i == indexChannel) && !muteGate) ? 10.0f : 0.0f;
			}
		}
		
		lightRefreshCounter++;
		if (lightRefreshCounter >= displayRefreshStepSkips) {
			lightRefreshCounter = 0;

			// Gate light
			float green = 0.0f;
			float red = 0.0f;
			if (gates[indexChannel][indexStep[indexChannel]] != 0) {
				if (gates[indexChannel][indexStep[indexChannel]] == 1) 	green = 1.0f;
				else {													green = 0.2f; red = 1.0f;}
			}	
			lights[GATE_LIGHT + 0].value = green;			
			lights[GATE_LIGHT + 1].value = red;
			
			// Reset light
			lights[RESET_LIGHT].value =	resetLight;	
			resetLight -= (resetLight / lightLambda) * engineGetSampleTime() * displayRefreshStepSkips;

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


struct WriteSeq64Widget : ModuleWidget {

	struct NoteDisplayWidget : TransparentWidget {
		WriteSeq64 *module;
		std::shared_ptr<Font> font;
		char text[7];

		NoteDisplayWidget() {
			font = Font::load(assetPlugin(plugin, "res/fonts/Segment14.ttf"));
		}
		
		void cvToStr(void) {
			int indexChannel = module->calcChan();
			float cvVal = module->cv[indexChannel][module->indexStep[indexChannel]];
			if (module->infoCopyPaste != 0l) {
				if (module->infoCopyPaste > 0l) {// if copy then display "Copy"
					snprintf(text, 7, "COPY");
				}
				else {// paste then display "Paste"
					snprintf(text, 7, "PASTE");
				}
			}
			else {			
				if (module->params[WriteSeq64::SHARP_PARAM].value > 0.5f) {// show notes
					text[0] = ' ';
					printNote(cvVal, &text[1], module->params[WriteSeq64::SHARP_PARAM].value < 1.5f);
				}
				else  {// show volts
					float cvValPrint = fabsf(cvVal);
					cvValPrint = (cvValPrint > 9.999f) ? 9.999f : cvValPrint;
					snprintf(text, 7, " %4.3f", cvValPrint);// Four-wide, three positions after the decimal, left-justified
					text[0] = (cvVal<0.0f) ? '-' : ' ';
					text[2] = ',';
				}
			}
		}

		void draw(NVGcontext *vg) override {
			NVGcolor textColor = prepareDisplay(vg, &box, 18);
			nvgFontFaceId(vg, font->handle);
			nvgTextLetterSpacing(vg, -1.5);

			Vec textPos = Vec(6, 24);
			nvgFillColor(vg, nvgTransRGBA(textColor, displayAlpha));
			nvgText(vg, textPos.x, textPos.y, "~~~~~~", NULL);
			nvgFillColor(vg, textColor);
			cvToStr();
			nvgText(vg, textPos.x, textPos.y, text, NULL);
		}
	};


	struct StepsDisplayWidget : TransparentWidget {
		WriteSeq64 *module;
		std::shared_ptr<Font> font;
		
		StepsDisplayWidget() {
			font = Font::load(assetPlugin(plugin, "res/fonts/Segment14.ttf"));
		}

		void draw(NVGcontext *vg) override {
			NVGcolor textColor = prepareDisplay(vg, &box, 18);
			nvgFontFaceId(vg, font->handle);
			//nvgTextLetterSpacing(vg, 2.5);

			Vec textPos = Vec(6, 24);
			nvgFillColor(vg, nvgTransRGBA(textColor, displayAlpha));
			nvgText(vg, textPos.x, textPos.y, "~~", NULL);
			nvgFillColor(vg, textColor);
			char displayStr[3];
			snprintf(displayStr, 3, "%2u", (unsigned) module->indexSteps[module->calcChan()]);
			nvgText(vg, textPos.x, textPos.y, displayStr, NULL);
		}
	};	
	
	
	struct StepDisplayWidget : TransparentWidget {
		WriteSeq64 *module;
		std::shared_ptr<Font> font;
		
		StepDisplayWidget() {
			font = Font::load(assetPlugin(plugin, "res/fonts/Segment14.ttf"));
		}

		void draw(NVGcontext *vg) override {
			NVGcolor textColor = prepareDisplay(vg, &box, 18);
			nvgFontFaceId(vg, font->handle);
			//nvgTextLetterSpacing(vg, 2.5);

			Vec textPos = Vec(6, 24);
			nvgFillColor(vg, nvgTransRGBA(textColor, displayAlpha));
			nvgText(vg, textPos.x, textPos.y, "~~", NULL);
			nvgFillColor(vg, textColor);
			char displayStr[3];
			snprintf(displayStr, 3, "%2u", (unsigned) module->indexStep[module->calcChan()] + 1);
			nvgText(vg, textPos.x, textPos.y, displayStr, NULL);
		}
	};
	
	
	struct ChannelDisplayWidget : TransparentWidget {
		WriteSeq64 *module;
		//int *indexTrack;
		std::shared_ptr<Font> font;
		
		ChannelDisplayWidget() {
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
			displayStr[0] = 0x30 + (char) (module->calcChan() + 1);
			displayStr[1] = 0;
			nvgText(vg, textPos.x, textPos.y, displayStr, NULL);
		}
	};

	
	struct PanelThemeItem : MenuItem {
		WriteSeq64 *module;
		int theme;
		void onAction(EventAction &e) override {
			module->panelTheme = theme;
		}
		void step() override {
			rightText = (module->panelTheme == theme) ? "✔" : "";
		}
	};
	struct ResetOnRunItem : MenuItem {
		WriteSeq64 *module;
		void onAction(EventAction &e) override {
			module->resetOnRun = !module->resetOnRun;
		}
	};
	Menu *createContextMenu() override {
		Menu *menu = ModuleWidget::createContextMenu();

		MenuLabel *spacerLabel = new MenuLabel();
		menu->addChild(spacerLabel);

		WriteSeq64 *module = dynamic_cast<WriteSeq64*>(this->module);
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
		
		ResetOnRunItem *rorItem = MenuItem::create<ResetOnRunItem>("Reset on run", CHECKMARK(module->resetOnRun));
		rorItem->module = module;
		menu->addChild(rorItem);
		
		return menu;
	}	
	
	
	WriteSeq64Widget(WriteSeq64 *module) : ModuleWidget(module) {
		// Main panel from Inkscape
        DynamicSVGPanel *panel = new DynamicSVGPanel();
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/WriteSeq64.svg")));
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/dark/WriteSeq64_dark.svg")));
        box.size = panel->box.size;
        panel->mode = &module->panelTheme;
        addChild(panel);
		
		// Screws
		addChild(createDynamicScrew<IMScrew>(Vec(15, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(box.size.x-30, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(15, 365), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(box.size.x-30, 365), &module->panelTheme));

		
		// ****** Top portion ******
		
		static const int rowRulerT0 = 56;
		static const int columnRulerT0 = 22;
		static const int columnRulerT1 = columnRulerT0 + 58;
		static const int columnRulerT2 = columnRulerT1 + 50;
		static const int columnRulerT3 = columnRulerT2 + 43;
		static const int columnRulerT4 = columnRulerT3 + 175;
		
		// Channel display
		ChannelDisplayWidget *channelTrack = new ChannelDisplayWidget();
		channelTrack->box.pos = Vec(columnRulerT0+1, rowRulerT0+vOffsetDisplay);
		channelTrack->box.size = Vec(24, 30);// 1 character
		channelTrack->module = module;
		addChild(channelTrack);
		// Step display
		StepDisplayWidget *displayStep = new StepDisplayWidget();
		displayStep->box.pos = Vec(columnRulerT1-8, rowRulerT0+vOffsetDisplay);
		displayStep->box.size = Vec(40, 30);// 2 characters
		displayStep->module = module;
		addChild(displayStep);
		// Gate LED
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(columnRulerT2+offsetLEDbutton+offsetLEDbuttonLight, rowRulerT0+offsetLEDbutton+offsetLEDbuttonLight), module, WriteSeq64::GATE_LIGHT));
		// Note display
		NoteDisplayWidget *displayNote = new NoteDisplayWidget();
		displayNote->box.pos = Vec(columnRulerT3, rowRulerT0+vOffsetDisplay);
		displayNote->box.size = Vec(98, 30);// 6 characters (ex.: "-1,234")
		displayNote->module = module;
		addChild(displayNote);
		// Volt/sharp/flat switch
		addParam(createParam<CKSSThreeInvNoRandom>(Vec(columnRulerT3+114+hOffsetCKSS, rowRulerT0+vOffsetCKSSThree), module, WriteSeq64::SHARP_PARAM, 0.0f, 2.0f, 1.0f));
		// Steps display
		StepsDisplayWidget *displaySteps = new StepsDisplayWidget();
		displaySteps->box.pos = Vec(columnRulerT4-7, rowRulerT0+vOffsetDisplay);
		displaySteps->box.size = Vec(40, 30);// 2 characters
		displaySteps->module = module;
		addChild(displaySteps);

		static const int rowRulerT1 = 105;
		
		// Channel knob
		addParam(createDynamicParam<IMFivePosSmallKnob>(Vec(columnRulerT0+offsetCKD6b+1, rowRulerT1+offsetCKD6b+1), module, WriteSeq64::CHANNEL_PARAM, 0.0f, 4.0f, 0.0f, &module->panelTheme));
		// Step knob
		addParam(createDynamicParam<IMBigKnobInf>(Vec(columnRulerT1+offsetIMBigKnob, rowRulerT1+offsetIMBigKnob), module, WriteSeq64::STEP_PARAM, -INFINITY, INFINITY, 0.0f, &module->panelTheme));		
		// Gate button
		addParam(createDynamicParam<IMBigPushButtonWithRClick>(Vec(columnRulerT2-1+offsetCKD6b, rowRulerT1+offsetCKD6b), module, WriteSeq64::GATE_PARAM , 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Autostep	
		addParam(createParam<CKSSNoRandom>(Vec(columnRulerT2+53+hOffsetCKSS, rowRulerT1+6+vOffsetCKSS), module, WriteSeq64::AUTOSTEP_PARAM, 0.0f, 1.0f, 1.0f));
		// Quantize switch
		addParam(createParam<CKSSNoRandom>(Vec(columnRulerT2+110+hOffsetCKSS, rowRulerT1+6+vOffsetCKSS), module, WriteSeq64::QUANTIZE_PARAM, 0.0f, 1.0f, 1.0f));
		// Reset LED bezel and light
		addParam(createParam<LEDBezel>(Vec(columnRulerT2+164+offsetLEDbezel, rowRulerT1+6+offsetLEDbezel), module, WriteSeq64::RESET_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(createLight<MuteLight<GreenLight>>(Vec(columnRulerT2+164+offsetLEDbezel+offsetLEDbezelLight, rowRulerT1+6+offsetLEDbezel+offsetLEDbezelLight), module, WriteSeq64::RESET_LIGHT));
		// Steps knob
		addParam(createDynamicParam<IMBigKnobInf>(Vec(columnRulerT4+offsetIMBigKnob, rowRulerT1+offsetIMBigKnob), module, WriteSeq64::STEPS_PARAM, -INFINITY, INFINITY, 0.0f, &module->panelTheme));		
	
		
		// ****** Bottom portion ******
		
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
		static const int rowRuler3 = rowRuler2 + rowRulerStep;
		
		// Column 0 
		// Copy/paste switches
		addParam(createDynamicParam<IMPushButton>(Vec(columnRuler0-10, rowRuler0+offsetTL1105), module, WriteSeq64::COPY_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParam<IMPushButton>(Vec(columnRuler0+20, rowRuler0+offsetTL1105), module, WriteSeq64::PASTE_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Paste sync (and light)
		addParam(createParam<CKSSThreeInvNoRandom>(Vec(columnRuler0+hOffsetCKSS, rowRuler1+vOffsetCKSSThree), module, WriteSeq64::PASTESYNC_PARAM, 0.0f, 2.0f, 0.0f));	
		addChild(createLight<SmallLight<RedLight>>(Vec(columnRuler0 + 41, rowRuler1 + 14), module, WriteSeq64::PENDING_LIGHT));
		// Gate input
		addInput(createDynamicPort<IMPort>(Vec(columnRuler0, rowRuler2), Port::INPUT, module, WriteSeq64::GATE_INPUT, &module->panelTheme));				
		// Run CV input
		addInput(createDynamicPort<IMPort>(Vec(columnRuler0, rowRuler3), Port::INPUT, module, WriteSeq64::RUNCV_INPUT, &module->panelTheme));
		
		
		// Column 1
		// Step L button
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRuler1+offsetCKD6b, rowRuler0+offsetCKD6b), module, WriteSeq64::STEPL_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Run LED bezel and light
		addParam(createParam<LEDBezel>(Vec(columnRuler1+offsetLEDbezel, rowRuler1+offsetLEDbezel), module, WriteSeq64::RUN_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(createLight<MuteLight<GreenLight>>(Vec(columnRuler1+offsetLEDbezel+offsetLEDbezelLight, rowRuler1+offsetLEDbezel+offsetLEDbezelLight), module, WriteSeq64::RUN_LIGHT));
		// CV input
		addInput(createDynamicPort<IMPort>(Vec(columnRuler1, rowRuler2), Port::INPUT, module, WriteSeq64::CV_INPUT, &module->panelTheme));
		// Step L input
		addInput(createDynamicPort<IMPort>(Vec(columnRuler1, rowRuler3), Port::INPUT, module, WriteSeq64::STEPL_INPUT, &module->panelTheme));
		
		
		// Column 2
		// Step R button
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRuler2+offsetCKD6b, rowRuler0+offsetCKD6b), module, WriteSeq64::STEPR_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));	
		// Write button and light
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRuler2+offsetCKD6b, rowRuler1+offsetCKD6b), module, WriteSeq64::WRITE_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLight<SmallLight<GreenRedLight>>(Vec(columnRuler2 -12, rowRuler1 - 12), module, WriteSeq64::WRITE_LIGHT));
		// Monitor
		addParam(createParam<CKSSHNoRandom>(Vec(columnRuler2+hOffsetCKSSH, rowRuler2+vOffsetCKSSH), module, WriteSeq64::MONITOR_PARAM, 0.0f, 1.0f, 0.0f));
		// Step R input
		addInput(createDynamicPort<IMPort>(Vec(columnRuler2, rowRuler3), Port::INPUT, module, WriteSeq64::STEPR_INPUT, &module->panelTheme));
		
		
		// Column 3
		// Clocks
		addInput(createDynamicPort<IMPort>(Vec(columnRuler3, rowRuler0), Port::INPUT, module, WriteSeq64::CLOCK12_INPUT, &module->panelTheme));		
		addInput(createDynamicPort<IMPort>(Vec(columnRuler3, rowRuler1), Port::INPUT, module, WriteSeq64::CLOCK34_INPUT, &module->panelTheme));		
		// Reset
		addInput(createDynamicPort<IMPort>(Vec(columnRuler3, rowRuler2), Port::INPUT, module, WriteSeq64::RESET_INPUT, &module->panelTheme));		
		// Write input
		addInput(createDynamicPort<IMPort>(Vec(columnRuler3, rowRuler3), Port::INPUT, module, WriteSeq64::WRITE_INPUT, &module->panelTheme));
		
					
		// Column 4 (CVs)
		// Outputs
		addOutput(createDynamicPort<IMPort>(Vec(columnRuler4, rowRuler0), Port::OUTPUT, module, WriteSeq64::CV_OUTPUTS + 0, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRuler4, rowRuler1), Port::OUTPUT, module, WriteSeq64::CV_OUTPUTS + 1, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRuler4, rowRuler2), Port::OUTPUT, module, WriteSeq64::CV_OUTPUTS + 2, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRuler4, rowRuler3), Port::OUTPUT, module, WriteSeq64::CV_OUTPUTS + 3, &module->panelTheme));
		
		
		// Column 5 (Gates)
		// Gates
		addOutput(createDynamicPort<IMPort>(Vec(columnRuler5, rowRuler0), Port::OUTPUT, module, WriteSeq64::GATE_OUTPUTS + 0, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRuler5, rowRuler1), Port::OUTPUT, module, WriteSeq64::GATE_OUTPUTS + 1, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRuler5, rowRuler2), Port::OUTPUT, module, WriteSeq64::GATE_OUTPUTS + 2, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRuler5, rowRuler3), Port::OUTPUT, module, WriteSeq64::GATE_OUTPUTS + 3, &module->panelTheme));
	}
};

} // namespace rack_plugin_ImpromptuModular

using namespace rack_plugin_ImpromptuModular;

RACK_PLUGIN_MODEL_INIT(ImpromptuModular, WriteSeq64) {
   Model *modelWriteSeq64 = Model::create<WriteSeq64, WriteSeq64Widget>("Impromptu Modular", "Write-Seq-64", "SEQ - Write-Seq-64", SEQUENCER_TAG);
   return modelWriteSeq64;
}

/*CHANGE LOG

0.6.16:
add 2nd gate mode for held gates (with right click to turn off)

0.6.12:
input refresh optimization

0.6.7:
no reset on run by default, with switch added in context menu
reset does not revert chan number to 1
*/