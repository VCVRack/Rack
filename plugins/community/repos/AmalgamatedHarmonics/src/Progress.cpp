#include "AH.hpp"
#include "Core.hpp"
#include "UI.hpp"
#include "dsp/digital.hpp"

#include <iostream>

namespace rack_plugin_AmalgamatedHarmonics {

struct Progress : AHModule {

	const static int NUM_PITCHES = 6;
	
	enum ParamIds {
		CLOCK_PARAM,
		RUN_PARAM,
		RESET_PARAM,
		STEPS_PARAM,
		ENUMS(ROOT_PARAM,8),
		ENUMS(CHORD_PARAM,8),
		ENUMS(INV_PARAM,8),
		ENUMS(GATE_PARAM,8),
		NUM_PARAMS
	};
	enum InputIds {
		KEY_INPUT,
		MODE_INPUT,
		CLOCK_INPUT,
		EXT_CLOCK_INPUT,
		RESET_INPUT,
		STEPS_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		GATES_OUTPUT,
		ENUMS(PITCH_OUTPUT,6),
		ENUMS(GATE_OUTPUT,8),
		NUM_OUTPUTS
	};
	enum LightIds {
		RUNNING_LIGHT,
		RESET_LIGHT,
		GATES_LIGHT,
		ENUMS(GATE_LIGHTS,16),
		NUM_LIGHTS
	};

	Progress() : AHModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) { }
	
	void step() override;
	
	enum ParamType {
		ROOT_TYPE,
		CHORD_TYPE,
		INV_TYPE
	};

	void receiveEvent(ParamEvent e) override {
		if (receiveEvents && e.pType != -1) { // AHParamWidgets that are no config through set<>() have a pType of -1
			if (modeMode) {
				paramState = "> " + 
					CoreUtil().noteNames[currRoot[e.pId]] + 
					CoreUtil().ChordTable[currChord[e.pId]].quality + " " +  
					CoreUtil().inversionNames[currInv[e.pId]] + " " + "[" + 
					CoreUtil().degreeNames[currDegree[e.pId] * 3 + currQuality[e.pId]] + "]"; 
			} else {
				paramState = "> " + 
					CoreUtil().noteNames[currRoot[e.pId]] + 
					CoreUtil().ChordTable[currChord[e.pId]].quality + " " +  
					CoreUtil().inversionNames[currInv[e.pId]];
			}
		}
		keepStateDisplay = 0;
	}
	
		
	json_t *toJson() override {
		json_t *rootJ = json_object();

		// running
		json_object_set_new(rootJ, "running", json_boolean(running));

		// gates
		json_t *gatesJ = json_array();
		for (int i = 0; i < 8; i++) {
			json_t *gateJ = json_integer((int) gates[i]);
			json_array_append_new(gatesJ, gateJ);
		}
		json_object_set_new(rootJ, "gates", gatesJ);

		// gateMode
		json_t *gateModeJ = json_integer((int) gateMode);
		json_object_set_new(rootJ, "gateMode", gateModeJ);

		return rootJ;
	}
	
	void fromJson(json_t *rootJ) override {
		// running
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);

		// gates
		json_t *gatesJ = json_object_get(rootJ, "gates");
		if (gatesJ) {
			for (int i = 0; i < 8; i++) {
				json_t *gateJ = json_array_get(gatesJ, i);
				if (gateJ)
					gates[i] = !!json_integer_value(gateJ);
			}
		}

		// gateMode
		json_t *gateModeJ = json_object_get(rootJ, "gateMode");
		if (gateModeJ)
			gateMode = (GateMode)json_integer_value(gateModeJ);
	}
	
	bool running = true;
	
	// for external clock
	SchmittTrigger clockTrigger; 
	
	// For buttons
	SchmittTrigger runningTrigger;
	SchmittTrigger resetTrigger;
	SchmittTrigger gateTriggers[8];
		
	PulseGenerator gatePulse;

	/** Phase of internal LFO */
	float phase = 0.0f;

	// Step index
	int index = 0;
	bool gates[8] = {true,true,true,true,true,true,true,true};

	float resetLight = 0.0f;
	float gateLight = 0.0f;
	float stepLights[8] = {};

	enum GateMode {
		TRIGGER,
		RETRIGGER,
		CONTINUOUS,
	};
	GateMode gateMode = CONTINUOUS;
		
	bool modeMode = false;
	bool prevModeMode = false;
	
	int offset = 24; 	// Repeated notes in chord and expressed in the chord definition as being transposed 2 octaves lower. 
						// When played this offset needs to be removed (or the notes removed, or the notes transposed to an octave higher)
	
	float prevRootInput[8] = {-100.0, -100.0, -100.0, -100.0, -100.0, -100.0, -100.0, -100.0};
	float prevChrInput[8] = {-100.0, -100.0, -100.0, -100.0, -100.0, -100.0, -100.0, -100.0};

	float prevDegreeInput[8] = {-100.0, -100.0, -100.0, -100.0, -100.0, -100.0, -100.0, -100.0};
	float prevQualityInput[8] = {-100.0, -100.0, -100.0, -100.0, -100.0, -100.0, -100.0, -100.0};

	float prevInvInput[8] = {-100.0, -100.0, -100.0, -100.0, -100.0, -100.0, -100.0, -100.0};

	float currRootInput[8];
	float currChrInput[8];

	float currDegreeInput[8];
	float currQualityInput[8];

	float currInvInput[8];
	
	int currMode;
	int currKey;
	int prevMode = -1;
	int prevKey = -1;
	
	int currRoot[8];
	int currChord[8];
	int currInv[8];	

	int currDegree[8];
	int currQuality[8];
	
	float pitches[8][6];
	float oldPitches[6];
			
	void reset() override {
		for (int i = 0; i < 8; i++) {
			gates[i] = true;
		}
	}
	
	void setIndex(int index, int nSteps) {
		phase = 0.0f;
		this->index = index;
		if (this->index >= nSteps) {
			this->index = 0;
		}
		this->gatePulse.trigger(Core::TRIGGER);
	}
	
};

void Progress::step() {
	
	AHModule::step();
	
	// Run
	if (runningTrigger.process(params[RUN_PARAM].value)) {
		running = !running;
	}

	int numSteps = (int) clamp(roundf(params[STEPS_PARAM].value + inputs[STEPS_INPUT].value), 1.0f, 8.0f);

	if (running) {
		if (inputs[EXT_CLOCK_INPUT].active) {
			// External clock
			if (clockTrigger.process(inputs[EXT_CLOCK_INPUT].value)) {
				setIndex(index + 1, numSteps);
			}
		}
		else {
			// Internal clock
			float clockTime = powf(2.0f, params[CLOCK_PARAM].value + inputs[CLOCK_INPUT].value);
			phase += clockTime * delta;
			if (phase >= 1.0f) {
				setIndex(index + 1, numSteps);
			}
		}
	}

	// Reset
	if (resetTrigger.process(params[RESET_PARAM].value + inputs[RESET_INPUT].value)) {
		setIndex(0, numSteps);
	}

	bool haveRoot = false;
	bool haveMode = false;

	// index is our current step
	if (inputs[KEY_INPUT].active) {
		float fRoot = inputs[KEY_INPUT].value;
		currKey = CoreUtil().getKeyFromVolts(fRoot);
		haveRoot = true;
	}

	if (inputs[MODE_INPUT].active) {
		float fMode = inputs[MODE_INPUT].value;
		currMode = CoreUtil().getModeFromVolts(fMode);	
		haveMode = true;
	}
	
	modeMode = haveRoot && haveMode;
	
	 if (modeMode && ((prevMode != currMode) || (prevKey != currKey))) { // Input changes so force re-read
	 	for (int step = 0; step < 8; step++) {
			prevDegreeInput[step]    = -100.0;
			prevQualityInput[step]  = -100.0;
		}
		
		prevMode = currMode;
		prevKey = currKey;
		
	}
	
	// Read inputs
	for (int step = 0; step < 8; step++) {
		if (modeMode) {
			currDegreeInput[step]  = params[CHORD_PARAM + step].value;
			currQualityInput[step] = params[ROOT_PARAM + step].value;
			if (prevModeMode != modeMode) { // Switching mode, so reset history to ensure re-read on return
				prevChrInput[step]  = -100.0;
				prevRootInput[step]  = -100.0;
			}
		} else {
			currChrInput[step]  = params[CHORD_PARAM + step].value;
			currRootInput[step] = params[ROOT_PARAM + step].value;
			if (prevModeMode != modeMode) { // Switching mode, so reset history to ensure re-read on return
				prevDegreeInput[step]  = -100.0;
				prevQualityInput[step]  = -100.0;
			}
		}
		currInvInput[step]  = params[INV_PARAM + step].value;
	}
	
	// Remember mode
	prevModeMode = modeMode;
	
	// Check for changes on all steps
	for (int step = 0; step < 8; step++) {
		
		bool update = false;
		
		if (modeMode) {			
		
			currDegreeInput[step]   = params[ROOT_PARAM + step].value;
			currQualityInput[step] = params[CHORD_PARAM + step].value;
							
			if (prevDegreeInput[step] != currDegreeInput[step]) {
				prevDegreeInput[step] = currDegreeInput[step];
				update = true;
			}
		
			if (prevQualityInput[step] != currQualityInput[step]) {
				prevQualityInput[step]  = currQualityInput[step]; 
				update = true;
			}
			
			if (update) {
				
				// Get Degree (I- VII)
				currDegree[step] = round(rescale(fabs(currDegreeInput[step]), 0.0f, 10.0f, 0.0f, Core::NUM_DEGREES - 1)); 

				// From the input root, mode and degree, we can get the root chord note and quality (Major,Minor,Diminshed)
				CoreUtil().getRootFromMode(currMode,currKey,currDegree[step],&currRoot[step],&currQuality[step]);

				// Now get the actual chord from the main list
				switch(currQuality[step]) {
					case Core::MAJ: 
						currChord[step] = round(rescale(fabs(currQualityInput[step]), 0.0f, 10.0f, 1.0f, 70.0f)); 
						break;
					case Core::MIN: 
						currChord[step] = round(rescale(fabs(currQualityInput[step]), 0.0f, 10.0f, 71.0f, 90.0f));
						break;
					case Core::DIM: 
						currChord[step] = round(rescale(fabs(currQualityInput[step]), 0.0f, 10.0f, 91.0f, 98.0f));
						break;		
				}
			
			}

		} else {
			
			// Chord Mode
			
			// If anything has changed, recalculate output for that step
			if (prevRootInput[step] != currRootInput[step]) {
				prevRootInput[step] = currRootInput[step];
				currRoot[step] = round(rescale(fabs(currRootInput[step]), 0.0f, 10.0f, 0.0f, Core::NUM_NOTES - 1)); // Param range is 0 to 10, mapped to 0 to 11
				update = true;
			}
		
			if (prevChrInput[step] != currChrInput[step]) {
				prevChrInput[step]  = currChrInput[step]; 
				currChord[step] = round(rescale(fabs(currChrInput[step]), 0.0f, 10.0f, 1.0f, 98.0f)); // Param range is 0 to 10		
				update = true;
			}
			
		}
		
		// Inversions remain the same between Chord and Mode mode
		if (prevInvInput[step] != currInvInput[step]) {
			prevInvInput[step]  = currInvInput[step];		
			currInv[step] = currInvInput[step];
			update = true;
		}
		
		// So, after all that, we calculate the pitch output
		if (update) {
					
			int *chordArray;
	
			// Get the array of pitches based on the inversion
			switch(currInv[step]) {
				case Core::ROOT:  		chordArray = CoreUtil().ChordTable[currChord[step]].root; 	break;
				case Core::FIRST_INV:  	chordArray = CoreUtil().ChordTable[currChord[step]].first; 	break;
				case Core::SECOND_INV:  chordArray = CoreUtil().ChordTable[currChord[step]].second;	break;
				default: chordArray = CoreUtil().ChordTable[currChord[step]].root;
			}
			
			for (int j = 0; j < NUM_PITCHES; j++) {
	
				// Set the pitches for this step. If the chord has less than 6 notes, the empty slots are
				// filled with repeated notes. These notes are identified by a  24 semi-tome negative
				// offset. We correct for that offset now, pitching thaem back into the original octave.
				// They could be pitched into the octave above (or below)
				if (chordArray[j] < 0) {
					pitches[step][j] = CoreUtil().getVoltsFromPitch(chordArray[j] + offset,currRoot[step]);			
				} else {
					pitches[step][j] = CoreUtil().getVoltsFromPitch(chordArray[j],currRoot[step]);			
				}	
			}
		}	
	}
	
	bool pulse = gatePulse.process(delta);
	
	// Gate buttons
	for (int i = 0; i < 8; i++) {
		if (gateTriggers[i].process(params[GATE_PARAM + i].value)) {
			gates[i] = !gates[i];
		}
		
		bool gateOn = (running && i == index && gates[i]);
		if (gateMode == TRIGGER) {
			gateOn = gateOn && pulse;
		} else if (gateMode == RETRIGGER) {
			gateOn = gateOn && !pulse;
		}
		
		outputs[GATE_OUTPUT + i].value = gateOn ? 10.0f : 0.0f;	
		
		if (i == index) {
			if (gates[i]) {
				// Gate is on and active = flash green
				lights[GATE_LIGHTS + i * 2].setBrightnessSmooth(1.0f);
				lights[GATE_LIGHTS + i * 2 + 1].setBrightnessSmooth(0.0f);
			} else {
				// Gate is off and active = flash dull yellow
				lights[GATE_LIGHTS + i * 2].setBrightnessSmooth(0.20f);
				lights[GATE_LIGHTS + i * 2 + 1].setBrightnessSmooth(0.20f);
			}
		} else {
			if (gates[i]) {
				// Gate is on and not active = red
				lights[GATE_LIGHTS + i * 2].setBrightnessSmooth(0.0f);
				lights[GATE_LIGHTS + i * 2 + 1].setBrightnessSmooth(1.0f);
			} else {
				// Gate is off and not active = black
				lights[GATE_LIGHTS + i * 2].setBrightnessSmooth(0.0f);
				lights[GATE_LIGHTS + i * 2 + 1].setBrightnessSmooth(0.0f);
			}			
		}
		
	}

	bool gatesOn = (running && gates[index]);
	if (gateMode == TRIGGER) {
		gatesOn = gatesOn && pulse;
	} else if (gateMode == RETRIGGER) {
		gatesOn = gatesOn && !pulse;
	}

	// Outputs
	outputs[GATES_OUTPUT].value = gatesOn ? 10.0f : 0.0f;
	lights[RUNNING_LIGHT].value = (running);
	lights[RESET_LIGHT].setBrightnessSmooth(resetTrigger.isHigh());
	lights[GATES_LIGHT].setBrightnessSmooth(pulse);

	for (int i = 0; i < NUM_PITCHES; i++) {
		outputs[PITCH_OUTPUT + i].value = pitches[index][i];
	}
	
}

struct ProgressWidget : ModuleWidget {
	ProgressWidget(Progress *module);
	Menu *createContextMenu() override;
};

ProgressWidget::ProgressWidget(Progress *module) : ModuleWidget(module) {
	
	UI ui;
		
	box.size = Vec(15*26, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Progress.svg")));
		addChild(panel);
	}

	{
		StateDisplay *display = new StateDisplay();
		display->module = module;
		display->box.pos = Vec(0, 135);
		display->box.size = Vec(100, 140);
		addChild(display);
	}
	
	addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 0, 0, true, false), module, Progress::CLOCK_PARAM, -2.0, 6.0, 2.0));
	addParam(ParamWidget::create<AHButton>(ui.getPosition(UI::BUTTON, 1, 0, true, false), module, Progress::RUN_PARAM, 0.0, 1.0, 0.0));
	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(ui.getPosition(UI::LIGHT, 1, 0, true, false), module, Progress::RUNNING_LIGHT));
	addParam(ParamWidget::create<AHButton>(ui.getPosition(UI::BUTTON, 2, 0, true, false), module, Progress::RESET_PARAM, 0.0, 1.0, 0.0));
	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(ui.getPosition(UI::LIGHT, 2, 0, true, false), module, Progress::RESET_LIGHT));
	addParam(ParamWidget::create<AHKnobSnap>(ui.getPosition(UI::KNOB, 3, 0, true, false), module, Progress::STEPS_PARAM, 1.0, 8.0, 8.0));
	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(ui.getPosition(UI::LIGHT, 4, 0, true, false), module, Progress::GATES_LIGHT));

//	static const float portX[13] = {20, 58, 96, 135, 173, 212, 250, 288, 326, 364, 402, 440, 478};
	addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 0, 1, true, false), Port::INPUT, module, Progress::CLOCK_INPUT));
	addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 1, 1, true, false), Port::INPUT, module, Progress::EXT_CLOCK_INPUT));
	addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 2, 1, true, false), Port::INPUT, module, Progress::RESET_INPUT));
	addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 3, 1, true, false), Port::INPUT, module, Progress::STEPS_INPUT));

	addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 4, 1, true, false), Port::INPUT, module, Progress::KEY_INPUT));
	addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 5, 1, true, false), Port::INPUT, module, Progress::MODE_INPUT));

	
	for (int i = 0; i < 3; i++) {
		addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 7 + i, 0, true, false), Port::OUTPUT, module, Progress::PITCH_OUTPUT + i));
	}	

	for (int i = 0; i < 3; i++) {
		addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 7 + i, 1, true, false), Port::OUTPUT, module, Progress::PITCH_OUTPUT + 3 + i));
	}

	for (int i = 0; i < 8; i++) {
		AHKnobNoSnap *rootW = ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, i + 1, 4, true, true), module, Progress::ROOT_PARAM + i, 0.0, 10.0, 0.0);
		AHParamWidget::set<AHKnobNoSnap>(rootW, Progress::ROOT_TYPE, i);
		addParam(rootW);
		
		AHKnobNoSnap *chordW = ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, i + 1, 5, true, true), module, Progress::CHORD_PARAM + i, 0.0, 10.0, 0.0);
		AHParamWidget::set<AHKnobNoSnap>(chordW, Progress::CHORD_TYPE, i);
		addParam(chordW);

		AHKnobSnap *invW = ParamWidget::create<AHKnobSnap>(ui.getPosition(UI::KNOB, i + 1, 6, true, true), module, Progress::INV_PARAM + i, 0.0, 2.0, 0.0);
		AHParamWidget::set<AHKnobSnap>(invW, Progress::INV_TYPE, i);
		addParam(invW);

		addParam(ParamWidget::create<AHButton>(ui.getPosition(UI::BUTTON, i + 1, 7, true, true), module, Progress::GATE_PARAM + i, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(ui.getPosition(UI::LIGHT, i + 1, 7, true, true), module, Progress::GATE_LIGHTS + i * 2));
				
		addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, i + 1, 5, true, false), Port::OUTPUT, module, Progress::GATE_OUTPUT + i));
	}

	addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 9, 5, true, false), Port::OUTPUT, module, Progress::GATES_OUTPUT));
	
}

struct ProgressGateModeItem : MenuItem {
	Progress *progress;
	Progress::GateMode gateMode;
	void onAction(EventAction &e) override {
		progress->gateMode = gateMode;
	}
	void step() override {
		rightText = (progress->gateMode == gateMode) ? "✔" : "";
	}
};

Menu *ProgressWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	Progress *progress = dynamic_cast<Progress*>(module);
	assert(progress);

	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Gate Mode";
	menu->addChild(modeLabel);

	ProgressGateModeItem *triggerItem = new ProgressGateModeItem();
	triggerItem->text = "Trigger";
	triggerItem->progress = progress;
	triggerItem->gateMode = Progress::TRIGGER;
	menu->addChild(triggerItem);

	ProgressGateModeItem *retriggerItem = new ProgressGateModeItem();
	retriggerItem->text = "Retrigger";
	retriggerItem->progress = progress;
	retriggerItem->gateMode = Progress::RETRIGGER;
	menu->addChild(retriggerItem);

	ProgressGateModeItem *continuousItem = new ProgressGateModeItem();
	continuousItem->text = "Continuous";
	continuousItem->progress = progress;
	continuousItem->gateMode = Progress::CONTINUOUS;
	menu->addChild(continuousItem);

	return menu;
}

} // namespace rack_plugin_AmalgamatedHarmonics

using namespace rack_plugin_AmalgamatedHarmonics;

RACK_PLUGIN_MODEL_INIT(AmalgamatedHarmonics, Progress) {
   Model *modelProgress = Model::create<Progress, ProgressWidget>( "Amalgamated Harmonics", "Progress", "Progress", SEQUENCER_TAG);
   return modelProgress;
}
