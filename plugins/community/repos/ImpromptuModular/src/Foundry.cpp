//***********************************************************************************************
//Multi-track sequencer module for VCV Rack by Marc Boulé
//
//Based on code from the Fundamental and AudibleInstruments plugins by Andrew Belt 
//and graphics from the Component Library by Wes Milholen 
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//
//Acknowledgements: please see README.md
//***********************************************************************************************

#include <algorithm>
#include "FoundrySequencer.hpp"

namespace rack_plugin_ImpromptuModular {

struct Foundry : Module {	
	enum ParamIds {
		EDIT_PARAM,
		PHRASE_PARAM,
		SEQUENCE_PARAM,
		RUN_PARAM,
		COPY_PARAM,
		PASTE_PARAM,
		RESET_PARAM,
		ENUMS(OCTAVE_PARAM, 7),
		GATE_PARAM,
		SLIDE_BTN_PARAM,
		AUTOSTEP_PARAM,
		ENUMS(KEY_PARAMS, 12),
		MODE_PARAM,
		CLKRES_PARAM,
		TRAN_ROT_PARAM,
		GATE_PROB_PARAM,
		TIE_PARAM,// Legato
		CPMODE_PARAM,
		ENUMS(STEP_PHRASE_PARAMS, SequencerKernel::MAX_STEPS),
		TRACKDOWN_PARAM,
		TRACKUP_PARAM,
		VEL_KNOB_PARAM,
		SEL_PARAM,
		ALLTRACKS_PARAM,
		REP_LEN_PARAM,
		BEGIN_PARAM,
		END_PARAM,
		KEY_GATE_PARAM,
		ATTACH_PARAM,
		VEL_EDIT_PARAM,
		WRITEMODE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		WRITE_INPUT,
		ENUMS(CV_INPUTS, Sequencer::NUM_TRACKS),
		RESET_INPUT,
		ENUMS(CLOCK_INPUTS, Sequencer::NUM_TRACKS),
		LEFTCV_INPUT,
		RIGHTCV_INPUT,
		RUNCV_INPUT,
		GATECV_INPUT,
		GATEPCV_INPUT,
		TIEDCV_INPUT,
		SLIDECV_INPUT,
		ENUMS(VEL_INPUTS, Sequencer::NUM_TRACKS),
		SEQCV_INPUT,
		TRKCV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(CV_OUTPUTS, Sequencer::NUM_TRACKS),
		ENUMS(VEL_OUTPUTS, Sequencer::NUM_TRACKS),
		ENUMS(GATE_OUTPUTS, Sequencer::NUM_TRACKS),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(STEP_PHRASE_LIGHTS, SequencerKernel::MAX_STEPS * 3),// room for GreenRedWhite
		ENUMS(OCTAVE_LIGHTS, 7),// octaves 1 to 7
		ENUMS(KEY_LIGHTS, 12 * 2),// room for GreenRed
		RUN_LIGHT,
		RESET_LIGHT,
		ENUMS(GATE_LIGHT, 2),// room for GreenRed
		SLIDE_LIGHT,
		ENUMS(GATE_PROB_LIGHT, 2),// room for GreenRed
		TIE_LIGHT,
		ATTACH_LIGHT,
		ENUMS(VEL_PROB_LIGHT, 2),// room for GreenRed
		VEL_SLIDE_LIGHT,
		ENUMS(WRITECV_LIGHTS, Sequencer::NUM_TRACKS),
		ENUMS(WRITECV2_LIGHTS, Sequencer::NUM_TRACKS),
		NUM_LIGHTS
	};
	
	// Constants
	enum EditPSDisplayStateIds {DISP_NORMAL, DISP_MODE_SEQ, DISP_MODE_SONG, DISP_LEN, DISP_REPS, DISP_TRANSPOSE, DISP_ROTATE, DISP_PPQN, DISP_DELAY, DISP_COPY_SEQ, DISP_PASTE_SEQ, DISP_COPY_SONG, DISP_PASTE_SONG, DISP_COPY_SONG_CUST};
	static constexpr float warningTime = 0.7f;// seconds

	// Need to save
	int panelTheme = 0;
	int expansion = 0;
	int velocityMode;
	bool velocityBipol;
	bool holdTiedNotes;
	bool autoseq;
	bool autostepLen;
	bool showSharp;
	bool multiTracks;
	int seqCVmethod;// 0 is 0-10V, 1 is C2-D7#, 2 is TrigIncr
	bool running;
	bool resetOnRun;
	bool attached;
	int velEditMode;// 0 is velocity, 1 is gate-prob, 2 is slide-rate
	int writeMode;// 0 is both, 1 is CV only, 2 is CV2 only
	Sequencer seq;

	// No need to save
	int displayState;
	long clockIgnoreOnReset;
	long tiedWarning;// 0 when no warning, positive downward step counter timer when warning
	long attachedWarning;// 0 when no warning, positive downward step counter timer when warning
	long revertDisplay;
	long showLenInSteps;
	bool multiSteps;
	int clkInSources[Sequencer::NUM_TRACKS];// first index is always 0 and will never change
	int cpSeqLength;
	int cpSongStart;// no need to initialize
	
	

	unsigned int lightRefreshCounter = 0;
	float resetLight = 0.0f;
	int sequenceKnob = 0;
	int velocityKnob = 0;
	int phraseKnob = 0;
	Trigger resetTrigger;
	Trigger leftTrigger;
	Trigger rightTrigger;
	Trigger runningTrigger;
	Trigger clockTriggers[SequencerKernel::MAX_STEPS];
	Trigger keyTriggers[12];
	Trigger octTriggers[7];
	Trigger gate1Trigger;
	Trigger tiedTrigger;
	Trigger gateProbTrigger;
	Trigger slideTrigger;
	Trigger writeTrigger;
	Trigger copyTrigger;
	Trigger pasteTrigger;
	Trigger modeTrigger;
	Trigger rotateTrigger;
	Trigger transposeTrigger;
	Trigger stepTriggers[SequencerKernel::MAX_STEPS];
	Trigger clkResTrigger;
	Trigger trackIncTrigger;
	Trigger trackDeccTrigger;	
	Trigger beginTrigger;
	Trigger endTrigger;
	Trigger repLenTrigger;
	Trigger attachedTrigger;
	Trigger seqCVTrigger;
	Trigger selTrigger;
	Trigger allTrigger;
	Trigger velEditTrigger;
	Trigger writeModeTrigger;

	
	inline bool isEditingSequence(void) {return params[EDIT_PARAM].value < 0.5f;}
	inline bool isEditingGates(void) {return params[KEY_GATE_PARAM].value < 0.5f;}
	inline int getCPMode(void) {
		if (params[CPMODE_PARAM].value > 1.5f) return 2000;// this means end, and code should never loop up to this count. This value should be bigger than max(MAX_STEPS, MAX_PHRASES)
		if (params[CPMODE_PARAM].value < 0.5f) return 4;
		return 8;
	}

	
	Foundry() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		seq.construct(&holdTiedNotes, &velocityMode);
		onReset();
	}

	
	// widgets are not yet created when module is created (and when onReset() is called by constructor)
	// onReset() is also called when right-click initialization of module
	void onReset() override {
		autoseq = false;
		autostepLen = false;
		showSharp = true;
		running = true;
		velocityMode = 0;
		velocityBipol = false;
		holdTiedNotes = true;
		displayState = DISP_NORMAL;
		tiedWarning = 0l;
		attachedWarning = 0l;
		revertDisplay = 0l;
		showLenInSteps = 0l;
		resetOnRun = false;
		attached = false;
		multiSteps = false;
		multiTracks = false;
		seqCVmethod = 0;
		cpSeqLength = getCPMode();
		for (int trkn = 0; trkn < Sequencer::NUM_TRACKS; trkn++) {
			clkInSources[trkn] = 0;
		}
		velEditMode = 0;
		writeMode = 0;
		seq.reset();
		clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * engineGetSampleRate());
	}
	
	
	void onRandomize() override {
		cpSeqLength = getCPMode();
		if (isEditingSequence() && !attached)
			seq.randomize();
		else if (attached)
			attachedWarning = (long) (warningTime * engineGetSampleRate() / displayRefreshStepSkips);
	}
	
	
	json_t *toJson() override {
		json_t *rootJ = json_object();

		// panelTheme
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));

		// expansion
		json_object_set_new(rootJ, "expansion", json_integer(expansion));

		// velocityMode
		json_object_set_new(rootJ, "velocityMode", json_integer(velocityMode));

		// velocityBipol
		json_object_set_new(rootJ, "velocityBipol", json_integer(velocityBipol));

		// autostepLen
		json_object_set_new(rootJ, "autostepLen", json_boolean(autostepLen));
		
		// multiTracks
		json_object_set_new(rootJ, "multiTracks", json_boolean(multiTracks));
		
		// autoseq
		json_object_set_new(rootJ, "autoseq", json_boolean(autoseq));
		
		// holdTiedNotes
		json_object_set_new(rootJ, "holdTiedNotes", json_boolean(holdTiedNotes));
		
		// showSharp
		json_object_set_new(rootJ, "showSharp", json_boolean(showSharp));
		
		// seqCVmethod
		json_object_set_new(rootJ, "seqCVmethod", json_integer(seqCVmethod));

		// running
		json_object_set_new(rootJ, "running", json_boolean(running));
		
		// resetOnRun
		json_object_set_new(rootJ, "resetOnRun", json_boolean(resetOnRun));
		
		// attached
		json_object_set_new(rootJ, "attached", json_boolean(attached));

		// velEditMode
		json_object_set_new(rootJ, "velEditMode", json_integer(velEditMode));

		// writeMode
		json_object_set_new(rootJ, "writeMode", json_integer(writeMode));

		seq.toJson(rootJ);
		
		return rootJ;
	}

	
	void fromJson(json_t *rootJ) override {
		// panelTheme
		json_t *panelThemeJ = json_object_get(rootJ, "panelTheme");
		if (panelThemeJ)
			panelTheme = json_integer_value(panelThemeJ);

		// expansion
		json_t *expansionJ = json_object_get(rootJ, "expansion");
		if (expansionJ)
			expansion = json_integer_value(expansionJ);

		// velocityMode
		json_t *velocityModeJ = json_object_get(rootJ, "velocityMode");
		if (velocityModeJ)
			velocityMode = json_integer_value(velocityModeJ);

		// velocityBipol
		json_t *velocityBipolJ = json_object_get(rootJ, "velocityBipol");
		if (velocityBipolJ)
			velocityBipol = json_integer_value(velocityBipolJ);

		// autostepLen
		json_t *autostepLenJ = json_object_get(rootJ, "autostepLen");
		if (autostepLenJ)
			autostepLen = json_is_true(autostepLenJ);

		// multiTracks
		json_t *multiTracksJ = json_object_get(rootJ, "multiTracks");
		if (multiTracksJ)
			multiTracks = json_is_true(multiTracksJ);

		// autoseq
		json_t *autoseqJ = json_object_get(rootJ, "autoseq");
		if (autoseqJ)
			autoseq = json_is_true(autoseqJ);

		// holdTiedNotes
		json_t *holdTiedNotesJ = json_object_get(rootJ, "holdTiedNotes");
		if (holdTiedNotesJ)
			holdTiedNotes = json_is_true(holdTiedNotesJ);
		
		// showSharp
		json_t *showSharpJ = json_object_get(rootJ, "showSharp");
		if (showSharpJ)
			showSharp = json_is_true(showSharpJ);
		
		// seqCVmethod
		json_t *seqCVmethodJ = json_object_get(rootJ, "seqCVmethod");
		if (seqCVmethodJ)
			seqCVmethod = json_integer_value(seqCVmethodJ);

		// running
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);
		
		// resetOnRun
		json_t *resetOnRunJ = json_object_get(rootJ, "resetOnRun");
		if (resetOnRunJ)
			resetOnRun = json_is_true(resetOnRunJ);

		// attached
		json_t *attachedJ = json_object_get(rootJ, "attached");
		if (attachedJ)
			attached = json_is_true(attachedJ);
		
		// velEditMode
		json_t *velEditModeJ = json_object_get(rootJ, "velEditMode");
		if (velEditModeJ)
			velEditMode = json_integer_value(velEditModeJ);

		// writeMode
		json_t *writeModeJ = json_object_get(rootJ, "writeMode");
		if (writeModeJ)
			writeMode = json_integer_value(writeModeJ);

		seq.fromJson(rootJ);
		
		// Initialize dependants after everything loaded
		cpSeqLength = getCPMode();
		seq.initRun();
	}


	void step() override {
		const float sampleRate = engineGetSampleRate();
		static const float revertDisplayTime = 0.7f;// seconds
		static const float showLenInStepsTime = 2.0f;// seconds
		
		
		//********** Buttons, knobs, switches and inputs **********
		
		bool editingSequence = isEditingSequence();
		
		// Run button
		if (runningTrigger.process(params[RUN_PARAM].value + inputs[RUNCV_INPUT].value)) {// no input refresh here, don't want to introduce startup skew
			running = !running;
			if (running) {
				if (resetOnRun)
					seq.initRun();
				if (resetOnRun || clockIgnoreOnRun)
					clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * sampleRate);
			}
			displayState = DISP_NORMAL;
			//multiSteps = false;
			//multiTracks = false;
		}

		if ((lightRefreshCounter & userInputsStepSkipMask) == 0) {
			
			// Seq CV input
			if (inputs[SEQCV_INPUT].active) {
				if (seqCVmethod == 0) {// 0-10 V
					int newSeq = (int)( inputs[SEQCV_INPUT].value * ((float)SequencerKernel::MAX_SEQS - 1.0f) / 10.0f + 0.5f );
					seq.setSeqIndexEdit(clamp(newSeq, 0, SequencerKernel::MAX_SEQS - 1));
				}
				else if (seqCVmethod == 1) {// C2-D7#
					int newSeq = (int)( (inputs[SEQCV_INPUT].value + 2.0f) * 12.0f + 0.5f );
					seq.setSeqIndexEdit(clamp(newSeq, 0, SequencerKernel::MAX_SEQS - 1));
				}
				else {// TrigIncr
					if (seqCVTrigger.process(inputs[SEQCV_INPUT].value))
						seq.setSeqIndexEdit(clamp(seq.getSeqIndexEdit() + 1, 0, SequencerKernel::MAX_SEQS - 1));
				}	
			}
			
			// Track CV input
			if (inputs[TRKCV_INPUT].active) {
				int newTrk = (int)( inputs[TRKCV_INPUT].value * (2.0f * (float)Sequencer::NUM_TRACKS - 1.0f) / 10.0f + 0.5f );
				seq.setTrackIndexEdit(abs(newTrk) % Sequencer::NUM_TRACKS);
				if (newTrk > 3)
					multiTracks = true;
				else
					multiTracks = false;
			}
			
			// Attach button
			if (attachedTrigger.process(params[ATTACH_PARAM].value)) {
				attached = !attached;
				displayState = DISP_NORMAL;			
				multiSteps = false;
				multiTracks = false;
			}
			if (attached) {//if (running && attached) {
				seq.attach();
			}
			
	
			// Copy 
			if (copyTrigger.process(params[COPY_PARAM].value)) {
				if (!attached) {
					multiTracks = false;
					if (editingSequence) {
						seq.copySequence(cpSeqLength);					
						displayState = DISP_COPY_SEQ;
					}
					else {
						int cpMode = getCPMode();
						if (cpMode == 2000) {
							if (displayState != DISP_COPY_SONG_CUST) {// first click to set cpSongStart
								cpSongStart = seq.getPhraseIndexEdit();
								displayState = DISP_COPY_SONG_CUST;
							}
							else {// second click do the copy 
								seq.copySong(cpSongStart, max(1, seq.getPhraseIndexEdit() - cpSongStart + 1));
								displayState = DISP_COPY_SONG;
							}
						}
						else {
							seq.copySong(seq.getPhraseIndexEdit(), cpMode);
							displayState = DISP_COPY_SONG;
						}
					}
					if (displayState != DISP_COPY_SONG_CUST)
						revertDisplay = (long) (revertDisplayTime * sampleRate / displayRefreshStepSkips);
				}
				else
					attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
			}
			// Paste 
			if (pasteTrigger.process(params[PASTE_PARAM].value)) {
				if (!attached) {
					if (editingSequence) {
						seq.pasteSequence(multiTracks);
						displayState = DISP_PASTE_SEQ;
					}
					else {
						if (displayState != DISP_COPY_SONG_CUST) {
							seq.pasteSong(multiTracks);
							displayState = DISP_PASTE_SONG;
						}
					}
					if (displayState != DISP_COPY_SONG_CUST)
						revertDisplay = (long) (revertDisplayTime * sampleRate / displayRefreshStepSkips);
				}
				else
					attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
			}			
			

			// Write input (must be before Left and Right in case route gate simultaneously to Right and Write for example)
			//  (write must be to correct step)
			bool writeTrig = writeTrigger.process(inputs[WRITE_INPUT].value);
			if (writeTrig) {
				if (editingSequence && !attached) {
					int multiStepsCount = multiSteps ? cpSeqLength : 1;
					for (int trkn = 0; trkn < Sequencer::NUM_TRACKS; trkn++) {
						if (trkn == seq.getTrackIndexEdit() || multiTracks) {
							if (inputs[VEL_INPUTS + trkn].active && ((writeMode & 0x1) == 0)) {	// must be before seq.writeCV() below, so that editing CV2 can be grabbed
								float maxVel = (velocityMode > 0 ? 127.0f : 200.0f);
								float capturedCV = inputs[VEL_INPUTS + trkn].value + (velocityBipol ? 5.0f : 0.0f);
								int intVel = (int)(capturedCV * maxVel / 10.0f + 0.5f);
								seq.setVelocityVal(trkn, clamp(intVel, 0, 200), multiStepsCount, false);
							}
							if (inputs[CV_INPUTS + trkn].active && ((writeMode & 0x2) == 0)) {
								seq.writeCV(trkn, clamp(inputs[CV_INPUTS + trkn].value, -10.0f, 10.0f), multiStepsCount, sampleRate, false);
							}
						}
					}
					seq.setEditingGateKeyLight(-1);
					if (params[AUTOSTEP_PARAM].value > 0.5f)
						seq.autostep(autoseq && !inputs[SEQCV_INPUT].active, autostepLen);
				}
				else if (attached)
					attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
			}
			// Left and right CV inputs
			int delta = 0;
			if (leftTrigger.process(inputs[LEFTCV_INPUT].value)) { 
				delta = -1;
			}
			if (rightTrigger.process(inputs[RIGHTCV_INPUT].value)) {
				delta = +1;
			}
			if (delta != 0) {
				if (editingSequence && !attached) {
					if (displayState == DISP_NORMAL) {
						seq.moveStepIndexEditWithEditingGate(delta, writeTrig, sampleRate);
					}
				}
				else if (attached)
					attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
			}

			// Step button presses
			int stepPressed = -1;
			for (int i = 0; i < SequencerKernel::MAX_STEPS; i++) {
				if (stepTriggers[i].process(params[STEP_PHRASE_PARAMS + i].value))
					stepPressed = i;
			}
			if (stepPressed != -1) {
				if (editingSequence && !attached) {
					if (displayState == DISP_LEN) {
						seq.setLength(stepPressed + 1, multiTracks);
						revertDisplay = (long) (revertDisplayTime * sampleRate / displayRefreshStepSkips);
					}
					else {
						if (multiSteps && (getCPMode() == 2000) && (stepPressed >= seq.getStepIndexEdit())) {
							cpSeqLength = stepPressed - seq.getStepIndexEdit() + 1;
						}
						else {
							showLenInSteps = (long) (showLenInStepsTime * sampleRate / displayRefreshStepSkips);
							seq.setStepIndexEdit(stepPressed, sampleRate);
							displayState = DISP_NORMAL; // leave this here, the if has it also, but through the revert mechanism
							if (multiSteps && (getCPMode() == 2000)) {
								multiSteps = false;
								cpSeqLength = 2000;
							}
						}
					}
				}
				else if (attached)
					attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
			} 
			
			// Mode button
			if (modeTrigger.process(params[MODE_PARAM].value)) {
				if (!attached) {
					if (displayState != DISP_MODE_SEQ && displayState != DISP_MODE_SONG)
						displayState = editingSequence ? DISP_MODE_SEQ : DISP_MODE_SONG;
					else
						displayState = DISP_NORMAL;
				}
				else
					attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
			}
			
			// Clk res/delay button
			if (clkResTrigger.process(params[CLKRES_PARAM].value)) {
				if (!attached) {
					if (displayState != DISP_PPQN && displayState != DISP_DELAY)	
						displayState = DISP_PPQN;
					else if (displayState == DISP_PPQN)
						displayState = DISP_DELAY;
					else
						displayState = DISP_NORMAL;
				}
				else
					attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
			}
			
			// Transpose/Rotate button
			if (transposeTrigger.process(params[TRAN_ROT_PARAM].value)) {
				if (editingSequence && !attached) {
					if (displayState != DISP_TRANSPOSE && displayState != DISP_ROTATE) {
						displayState = DISP_TRANSPOSE;
					}
					else if (displayState == DISP_TRANSPOSE) {
						displayState = DISP_ROTATE;
					}
					else 
						displayState = DISP_NORMAL;
				}
				else if (attached)
					attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
			}			

			// Begin/End buttons
			if (beginTrigger.process(params[BEGIN_PARAM].value)) {
				if (!editingSequence && !attached) {
					seq.setBegin(multiTracks);
					displayState = DISP_NORMAL;
				}
				else if (attached)
					attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
			}	
			if (endTrigger.process(params[END_PARAM].value)) {
				if (!editingSequence && !attached) {
					seq.setEnd(multiTracks);
					displayState = DISP_NORMAL;
				}
				else if (attached)
					attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
			}	

			// Rep/Len button
			if (repLenTrigger.process(params[REP_LEN_PARAM].value)) {
				if (!attached) {
					if (displayState != DISP_LEN && displayState != DISP_REPS)
						displayState = editingSequence ? DISP_LEN : DISP_REPS;
					else
						displayState = DISP_NORMAL;
				}
				else
					attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
			}	

			// Track Inc/Dec buttons
			if (trackIncTrigger.process(params[TRACKUP_PARAM].value)) {
				if (!inputs[TRKCV_INPUT].active) {
					seq.incTrackIndexEdit();
				}
			}
			if (trackDeccTrigger.process(params[TRACKDOWN_PARAM].value)) {
				if (!inputs[TRKCV_INPUT].active) {
					seq.decTrackIndexEdit();
				}
			}
			// All button
			if (allTrigger.process(params[ALLTRACKS_PARAM].value)) {
				if (!inputs[TRKCV_INPUT].active) {
					if (!attached) {
						multiTracks = !multiTracks;
					}
					else {
						multiTracks = false;
						attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
					}
				}
			}	
			
			// Sel button
			if (selTrigger.process(params[SEL_PARAM].value)) {
				if (!attached && editingSequence)
					multiSteps = !multiSteps;
				else if (attached) {
					multiSteps = false;
					attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
				}
			}	
			
			// Vel mode button
			if (velEditTrigger.process(params[VEL_EDIT_PARAM].value)) {
				if (attached || editingSequence) {
					if (velEditMode < 2)
						velEditMode++;
					else {
						velEditMode = 0;
					}
				}
			}
			
			// Write mode button
			if (writeModeTrigger.process(params[WRITEMODE_PARAM].value)) {
				if (attached || editingSequence) {
					if (++writeMode > 2)
						writeMode =0;
				}
			}
		
			// Velocity edit knob 
			float velParamValue = params[VEL_KNOB_PARAM].value;
			int newVelocityKnob = (int)roundf(velParamValue * 30.0f);
			if (velParamValue == 0.0f)// true when constructor or fromJson() occured
				velocityKnob = newVelocityKnob;
			int deltaVelKnob = newVelocityKnob - velocityKnob;
			if (deltaVelKnob != 0) {
				if (abs(deltaVelKnob) <= 3) {// avoid discontinuous step (initialize for example)
					// any changes in here should may also require right click behavior to be updated in the knob's onMouseDown()
					if (editingSequence && !attached) {
						int mutliStepsCount = multiSteps ? cpSeqLength : 1;
						if (velEditMode == 2) {
							seq.modSlideVal(deltaVelKnob, mutliStepsCount, multiTracks);
						}
						else if (velEditMode == 1) {
							seq.modGatePVal(deltaVelKnob, mutliStepsCount, multiTracks);
						}
						else {
							seq.modVelocityVal(deltaVelKnob, mutliStepsCount, multiTracks);
						}
						displayState = DISP_NORMAL;
					}
					else if (attached)
						attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
				}
				velocityKnob = newVelocityKnob;
			}	

						
			// Sequence edit knob 
			float seqParamValue = params[SEQUENCE_PARAM].value;
			int newSequenceKnob = (int)roundf(seqParamValue * 7.0f);
			if (seqParamValue == 0.0f)// true when constructor or fromJson() occured
				sequenceKnob = newSequenceKnob;
			int deltaSeqKnob = newSequenceKnob - sequenceKnob;
			if (deltaSeqKnob != 0) {
				if (abs(deltaSeqKnob) <= 3) {// avoid discontinuous step (initialize for example)
					// any changes in here should may also require right click behavior to be updated in the knob's onMouseDown()
					if (displayState == DISP_LEN) {
						seq.modLength(deltaSeqKnob, multiTracks);
					}
					else if (displayState == DISP_TRANSPOSE) {
						seq.transposeSeq(deltaSeqKnob, multiTracks);
					}
					else if (displayState == DISP_ROTATE) {
						seq.rotateSeq(deltaSeqKnob, multiTracks);
					}							
					else if (displayState == DISP_REPS) {
						seq.modPhraseReps(deltaSeqKnob, multiTracks);
					}
					else if (displayState == DISP_PPQN || displayState == DISP_DELAY) {
					}
					else if (!attached) {
						if (editingSequence) {
							if (!inputs[SEQCV_INPUT].active) {
								seq.moveSeqIndexEdit(deltaSeqKnob);
								if (displayState != DISP_MODE_SEQ)
									displayState = DISP_NORMAL;
							}
						}
						else {// editing song
							seq.modPhraseSeqNum(deltaSeqKnob, multiTracks);
							displayState = DISP_NORMAL;
						}
					}
					else if (attached)
						attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
				}
				sequenceKnob = newSequenceKnob;
			}	
		

			// Phrase edit knob 
			float phraseParamValue = params[PHRASE_PARAM].value;
			int newPhraseKnob = (int)roundf(phraseParamValue * 7.0f);
			if (phraseParamValue == 0.0f)// true when constructor or fromJson() occured
				phraseKnob = newPhraseKnob;
			int deltaPhrKnob = newPhraseKnob - phraseKnob;
			if (deltaPhrKnob != 0) {
				if (abs(deltaPhrKnob) <= 3) {// avoid discontinuous step (initialize for example)
					// any changes in here should may also require right click behavior to be updated in the knob's onMouseDown()
					if (displayState == DISP_MODE_SEQ) {
						seq.modRunModeSeq(deltaPhrKnob, multiTracks);
					}
					else if (displayState == DISP_PPQN) {
						seq.modPulsesPerStep(deltaPhrKnob, multiTracks);
					}
					else if (displayState == DISP_DELAY) {
						seq.modDelay(deltaPhrKnob, multiTracks);
					}
					else if (displayState == DISP_MODE_SONG) {
						seq.modRunModeSong(deltaPhrKnob, multiTracks);
					}
					else if (!editingSequence && !attached && displayState != DISP_PPQN && displayState != DISP_DELAY) {
						seq.movePhraseIndexEdit(deltaPhrKnob);
						if (displayState != DISP_REPS && displayState != DISP_COPY_SONG_CUST)
							displayState = DISP_NORMAL;
					}
					else if (attached)
						attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
				}
				phraseKnob = newPhraseKnob;
			}	
				
	
			// Octave buttons
			for (int octn = 0; octn < 7; octn++) {
				if (octTriggers[octn].process(params[OCTAVE_PARAM + octn].value)) {
					if (editingSequence && !attached && displayState != DISP_PPQN) {
						if (seq.applyNewOctave(6 - octn, multiSteps ? cpSeqLength : 1, sampleRate, multiTracks))
							tiedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
					}
					else if (attached)
						attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
					displayState = DISP_NORMAL;
				}
			}
			
			// Keyboard buttons
			for (int keyn = 0; keyn < 12; keyn++) {
				if (keyTriggers[keyn].process(params[KEY_PARAMS + keyn].value)) {
					displayState = DISP_NORMAL;
					if (editingSequence && !attached && displayState != DISP_PPQN) {
						bool autostepClick = params[KEY_PARAMS + keyn].value > 1.5f;// if right-click
						if (isEditingGates()) {
							if (!seq.setGateType(keyn, multiSteps ? cpSeqLength : 1, sampleRate, autostepClick, multiTracks))
								displayState = DISP_PPQN;
						}
						else {
							if (seq.applyNewKey(keyn, multiSteps ? cpSeqLength : 1, sampleRate, autostepClick, multiTracks))
								tiedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
						}							
					}
					else if (attached)
						attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
				}
			}
			
			// Gate, GateProb, Slide and Tied buttons
			if (gate1Trigger.process(params[GATE_PARAM].value + inputs[GATECV_INPUT].value)) {
				if (editingSequence && !attached) {
					seq.toggleGate(multiSteps ? cpSeqLength : 1, multiTracks);
				}
				else if (attached)
					attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
				displayState = DISP_NORMAL;
			}		
			if (gateProbTrigger.process(params[GATE_PROB_PARAM].value + inputs[GATEPCV_INPUT].value)) {
				if (editingSequence && !attached) {
					if (seq.toggleGateP(multiSteps ? cpSeqLength : 1, multiTracks)) 
						tiedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
					else if (seq.getAttribute().getGateP())
						velEditMode = 1;
				}
				else if (attached)
					attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
				displayState = DISP_NORMAL;
			}		
			if (slideTrigger.process(params[SLIDE_BTN_PARAM].value + inputs[SLIDECV_INPUT].value)) {
				if (editingSequence && !attached) {
					if (seq.toggleSlide(multiSteps ? cpSeqLength : 1, multiTracks))
						tiedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
					else if (seq.getAttribute().getSlide())
						velEditMode = 2;
				}
				else if (attached)
					attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
				displayState = DISP_NORMAL;
			}		
			if (tiedTrigger.process(params[TIE_PARAM].value + inputs[TIEDCV_INPUT].value)) {
				if (editingSequence && !attached) {
					seq.toggleTied(multiSteps ? cpSeqLength : 1, multiTracks);// will clear other attribs if new state is on
				}
				else if (attached)
					attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
				displayState = DISP_NORMAL;
			}		
			
			calcClkInSources();
			
		}// userInputs refresh
		
		
		
		//********** Clock and reset **********
		
		// Clock
		if (running && clockIgnoreOnReset == 0l) {
			bool clockTrigged[Sequencer::NUM_TRACKS];
			for (int trkn = 0; trkn < Sequencer::NUM_TRACKS; trkn++) {
				clockTrigged[trkn] = clockTriggers[trkn].process(inputs[CLOCK_INPUTS + trkn].value);
				if (clockTrigged[clkInSources[trkn]]) {
					seq.clockStep(trkn);
				}
			}
			seq.step();
		}
				
		// Reset
		if (resetTrigger.process(inputs[RESET_INPUT].value + params[RESET_PARAM].value)) {
			seq.initRun();
			resetLight = 1.0f;
			displayState = DISP_NORMAL;
			//multiSteps = false;
			//multiTracks = false;
			clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * sampleRate);
			for (int trkn = 0; trkn < Sequencer::NUM_TRACKS; trkn++)
				clockTriggers[trkn].reset();		
			if (inputs[SEQCV_INPUT].active && seqCVmethod == 2)
				seq.setSeqIndexEdit(0);
		}
		
		
		//********** Outputs and lights **********
				
		
		
		// CV, gate and velocity outputs
		for (int trkn = 0; trkn < Sequencer::NUM_TRACKS; trkn++) {
			outputs[CV_OUTPUTS + trkn].value = seq.calcCvOutputAndDecSlideStepsRemain(trkn, running);
			bool retriggingOnReset = (clockIgnoreOnReset != 0l && retrigGatesOnReset);
			outputs[GATE_OUTPUTS + trkn].value = seq.calcGateOutput(trkn, running && !retriggingOnReset, clockTriggers[clkInSources[trkn]], sampleRate);
			outputs[VEL_OUTPUTS + trkn].value = seq.calcVelOutput(trkn, running && !retriggingOnReset) - (velocityBipol ? 5.0f : 0.0f);			
		}

		// lights
		lightRefreshCounter++;
		if (lightRefreshCounter >= displayRefreshStepSkips) {
			lightRefreshCounter = 0;
		
			// Prepare values to visualize
			StepAttributes attributesVisual;
			float cvVisual;
			if (editingSequence || attached) {
				attributesVisual = seq.getAttribute();
				cvVisual = seq.getCV();
			}
			else {
				attributesVisual.clear();// clears everything, but just buttons used below
				cvVisual = 0.0f;// not used
			}
			bool editingGates = isEditingGates();
			
			// Step lights
			for (int stepn = 0; stepn < SequencerKernel::MAX_STEPS; stepn++) {
				float red = 0.0f;
				float green = 0.0f;	
				float white = 0.0f;
				if ((displayState == DISP_COPY_SEQ) || (displayState == DISP_PASTE_SEQ)) {
					int startCP = seq.getStepIndexEdit();
					if (stepn >= startCP && stepn < (startCP + seq.getLengthSeqCPbug()))
						green = 0.5f;
				}
				else if (displayState == DISP_TRANSPOSE) {
					red = 0.5f;
				}
				else if (displayState == DISP_ROTATE) {
					red = (stepn == seq.getStepIndexEdit() ? 1.0f : (stepn < seq.getLength() ? 0.2f : 0.0f));
				}
				else if (displayState == DISP_LEN) {
					int seqEnd = seq.getLength() - 1;
					if (stepn < seqEnd)
						green = 0.1f;
					else if (stepn == seqEnd)
						green =  1.0f;
				}				
				else if (editingSequence && !attached) {
					if (multiSteps) {
						if (stepn >= seq.getStepIndexEdit() && stepn < (seq.getStepIndexEdit() + cpSeqLength))
							red = 0.2f;
					}
					else if (!running && showLenInSteps > 0l && stepn < seq.getLength()) {
						green = 0.01f;
					}
					if (stepn == seq.getStepIndexEdit()) {
						if (red == 0.0f) // don't overwrite light multistep with full red
							red = 1.0f;
						green = 0.0f;
					}
					if (running && red != 0.2f && stepn == seq.getStepIndexRun(seq.getTrackIndexEdit())) {
						green = 1.0f;
					}
					white = 1.0f;// signal for override below
				}
				else if (attached) {
					// all active light green, current track is bright yellow
					for (int trkn = 0; trkn < Sequencer::NUM_TRACKS; trkn++) {
						bool trknIsUsed = outputs[CV_OUTPUTS + trkn].active || outputs[GATE_OUTPUTS + trkn].active || outputs[VEL_OUTPUTS + trkn].active;
						if (stepn == seq.getStepIndexRun(trkn) && trknIsUsed) 
							green = 0.1f;	
					}
					if (green > 0.2f) 
						green = 0.2f;
					if (stepn == seq.getStepIndexEdit()) {
						green = 1.0f;
						red = 1.0f;
					}
					white = 1.0f;// signal for override below
				}
				if (white == 1.0f)
					white = ((green == 0.0f && red == 0.0f && (editingSequence || attached) && seq.getAttribute(seq.getTrackIndexEdit(), stepn).getGate() && displayState != DISP_MODE_SEQ && displayState != DISP_PPQN && displayState != DISP_DELAY) ? 0.04f : 0.0f);
				if (editingSequence && white != 0.0f) {
					green = 0.02f; white = 0.0f;
				}				
				setGreenRed(STEP_PHRASE_LIGHTS + stepn * 3, green, red);
				//if (white != 0.0f && seq.getAttribute(seq.getTrackIndexEdit(), stepn).getGateP()) white = 0.01f;
				lights[STEP_PHRASE_LIGHTS + stepn * 3 + 2].value = white;
			}
			
			
			// Octave lights
			int octLightIndex = (int) floor(cvVisual + 3.0f);
			for (int i = 0; i < 7; i++) {
				float red = 0.0f;
				if (editingSequence || attached) {
					if (tiedWarning > 0l) {
						bool warningFlashState = calcWarningFlash(tiedWarning, (long) (warningTime * sampleRate / displayRefreshStepSkips));
						red = (warningFlashState && (i == (6 - octLightIndex))) ? 1.0f : 0.0f;
					}
					else				
						red = (i == (6 - octLightIndex) ? 1.0f : 0.0f);// no lights when outside of range
				}
				lights[OCTAVE_LIGHTS + i].value = red;
			}
			
			// Keyboard lights
			float keyCV = cvVisual + 10.0f;// to properly handle negative note voltages
			int keyLightIndex = clamp( (int)((keyCV - floor(keyCV)) * 12.0f + 0.5f),  0,  11);
			for (int i = 0; i < 12; i++) {
				float green = 0.0f;
				float red = 0.0f;
				if (displayState == DISP_PPQN) {
					if (seq.keyIndexToGateTypeEx(i) != -1) {
						green =	1.0f;
						red = 1.0f;
					}
				}
				else if (editingSequence || attached) {			
					if (editingGates) {
						green = 1.0f;
						red = 0.2f;
						unsigned long editingType = seq.getEditingType();
						if (editingType > 0ul) {
							if (i == seq.getEditingGateKeyLight()) {
								float dimMult = ((float) editingType / (float)(Sequencer::gateTime * sampleRate / displayRefreshStepSkips));
								green *= dimMult;
								red *= dimMult;
							}
							else {
								green = 0.0f; 
								red = 0.0f;
							}
						}
						else {
							int modeLightIndex = seq.getGateType();
							if (i != modeLightIndex) {// show dim note if gatetype is different than note
								green = 0.0f;
								red = (i == keyLightIndex ? 0.1f : 0.0f);
							}
						}
					}
					else {
						if (tiedWarning > 0l) {
							bool warningFlashState = calcWarningFlash(tiedWarning, (long) (warningTime * sampleRate / displayRefreshStepSkips));
							red = (warningFlashState && i == keyLightIndex) ? 1.0f : 0.0f;
						}
						else {
							red = seq.calcKeyLightWithEditing(i, keyLightIndex, sampleRate);
						}
					}
				}
				setGreenRed(KEY_LIGHTS + i * 2, green, red);
			}

			// Gate, Tied, GateProb, and Slide lights 
			if (!attributesVisual.getGate())
				setGreenRed(GATE_LIGHT, 0.0f, 0.0f);
			else 
				setGreenRed(GATE_LIGHT, editingGates ? 1.0f : 0.0f, editingGates ? 0.2f : 1.0f);
			if (tiedWarning > 0l) {
				bool warningFlashState = calcWarningFlash(tiedWarning, (long) (warningTime * sampleRate / displayRefreshStepSkips));
				lights[TIE_LIGHT].value = (warningFlashState) ? 1.0f : 0.0f;
			}
			else
				lights[TIE_LIGHT].value = attributesVisual.getTied() ? 1.0f : 0.0f;			
			if (attributesVisual.getGateP())
				setGreenRed(GATE_PROB_LIGHT, 1.0f, 1.0f);
			else 
				setGreenRed(GATE_PROB_LIGHT, 0.0f, 0.0f);
			lights[SLIDE_LIGHT].value = attributesVisual.getSlide() ? 1.0f : 0.0f;
			
			// Reset light
			lights[RESET_LIGHT].value =	resetLight;
			resetLight -= (resetLight / lightLambda) * engineGetSampleTime() * displayRefreshStepSkips;
			
			// Run light
			lights[RUN_LIGHT].value = (running ? 1.0f : 0.0f);

			// Attach light
			if (attachedWarning > 0l) {
				bool warningFlashState = calcWarningFlash(attachedWarning, (long) (warningTime * sampleRate / displayRefreshStepSkips));
				lights[ATTACH_LIGHT].value = (warningFlashState) ? 1.0f : 0.0f;
			}
			else
				lights[ATTACH_LIGHT].value = (attached ? 1.0f : 0.0f);
				
			// Velocity edit mode lights
			if (editingSequence || attached) {
				setGreenRed(VEL_PROB_LIGHT, velEditMode == 1 ? 1.0f : 0.0f, velEditMode == 1 ? 1.0f : 0.0f);
				lights[VEL_SLIDE_LIGHT].value = (velEditMode == 2 ? 1.0f : 0.0f);
			}
			else {
				setGreenRed(VEL_PROB_LIGHT, 0.0f, 0.0f);
				lights[VEL_SLIDE_LIGHT].value = 0.0f;
			}
			
			// CV writing lights
			for (int trkn = 0; trkn < Sequencer::NUM_TRACKS; trkn++) {
				if (editingSequence && !attached) {
					lights[WRITECV_LIGHTS + trkn].value = (((writeMode & 0x2) == 0) && (multiTracks || seq.getTrackIndexEdit() == trkn)) ? 1.0f : 0.0f;
					lights[WRITECV2_LIGHTS + trkn].value = (((writeMode & 0x1) == 0) && (multiTracks || seq.getTrackIndexEdit() == trkn)) ? 1.0f : 0.0f;
				}
				else {
					lights[WRITECV_LIGHTS + trkn].value = 0.0f;
					lights[WRITECV2_LIGHTS + trkn].value = 0.0f;
				}
			}				
				
			seq.stepEditingGate();// also steps editingType
			if (tiedWarning > 0l)
				tiedWarning--;
			if (attachedWarning > 0l)
				attachedWarning--;
			if (showLenInSteps > 0l)
				showLenInSteps--;
			if (revertDisplay > 0l) {
				if (revertDisplay == 1)
					displayState = DISP_NORMAL;
				revertDisplay--;
			}
		}// lightRefreshCounter
				
		if (clockIgnoreOnReset > 0l)
			clockIgnoreOnReset--;
		
	}// step()
	

	inline void setGreenRed(int id, float green, float red) {
		lights[id + 0].value = green;
		lights[id + 1].value = red;
	}
	
	inline void calcClkInSources() {
		// index 0 is always 0 so nothing to do for it
		for (int trkn = 1; trkn < Sequencer::NUM_TRACKS; trkn++) {
			if (inputs[CLOCK_INPUTS + trkn].active)
				clkInSources[trkn] = trkn;
			else 
				clkInSources[trkn] = clkInSources[trkn - 1];
		}
	}
	
	
};



struct FoundryWidget : ModuleWidget {
	Foundry *module;
	DynamicSVGPanel *panel;
	int oldExpansion;
	int expWidth = 105;
	IMPort* expPorts[12];
	
	template <int NUMCHAR>
	struct DisplayWidget : TransparentWidget {// a centered display, must derive from this
		Foundry *module;
		std::shared_ptr<Font> font;
		char displayStr[NUMCHAR + 1];
		static const int textFontSize = 15;
		static constexpr float textOffsetY = 19.9f; // 18.2f for 14 pt, 19.7f for 15pt
		
		void runModeToStr(int num) {
			if (num >= 0 && num < SequencerKernel::NUM_MODES)
				snprintf(displayStr, 4, "%s", SequencerKernel::modeLabels[num].c_str());
		}

		DisplayWidget(Vec _pos, Vec _size, Foundry *_module) {
			box.size = _size;
			box.pos = _pos.minus(_size.div(2));
			module = _module;
			font = Font::load(assetPlugin(plugin, "res/fonts/Segment14.ttf"));
		}
		
		void draw(NVGcontext *vg) override {
			NVGcolor textColor = prepareDisplay(vg, &box, textFontSize);
			nvgFontFaceId(vg, font->handle);
			nvgTextLetterSpacing(vg, -0.4);

			Vec textPos = Vec(5.7f, textOffsetY);
			nvgFillColor(vg, nvgTransRGBA(textColor, displayAlpha));
			std::string initString(NUMCHAR,'~');
			nvgText(vg, textPos.x, textPos.y, initString.c_str(), NULL);
			nvgFillColor(vg, textColor);
			char overlayChar = printText();
			nvgText(vg, textPos.x, textPos.y, displayStr, NULL);
			if (overlayChar != 0) {
				displayStr[0] = overlayChar;
				displayStr[1] = 0;
				nvgText(vg, textPos.x, textPos.y, displayStr, NULL);
			}
		}
		
		virtual char printText() = 0;
	};
	
	struct VelocityDisplayWidget : DisplayWidget<4> {
		VelocityDisplayWidget(Vec _pos, Vec _size, Foundry *_module) : DisplayWidget(_pos, _size, _module) {};

		void draw(NVGcontext *vg) override {
			static const float offsetXfrac = 3.5f;
			NVGcolor textColor = prepareDisplay(vg, &box, textFontSize);
			nvgFontFaceId(vg, font->handle);
			nvgTextLetterSpacing(vg, -0.4);

			Vec textPos = Vec(6.3f, textOffsetY);
			char useRed = printText();
			if (useRed == 1)
				textColor = nvgRGB(0xE0, 0xD0, 0x30);
			nvgFillColor(vg, nvgTransRGBA(textColor, displayAlpha));
			nvgText(vg, textPos.x, textPos.y, "~", NULL);
			std::string initString(".~~");
			nvgText(vg, textPos.x + offsetXfrac, textPos.y, initString.c_str(), NULL);
			if (useRed == 1)
				textColor = nvgRGB(0xFF, 0x2C, 0x20);
			nvgFillColor(vg, textColor);
			nvgText(vg, textPos.x + offsetXfrac, textPos.y, &displayStr[1], NULL);
			displayStr[1] = 0;
			nvgText(vg, textPos.x, textPos.y, displayStr, NULL);
		}

		char printText() override {
			char ret = 0;// used for a color instead of overlay char. 0 = default (green), 1 = red
			if (module->isEditingSequence() || module->attached) {
				StepAttributes attributesVal = module->seq.getAttribute();
				if (module->velEditMode == 2) {
					int slide = attributesVal.getSlideVal();						
					if ( slide >= 100)
						snprintf(displayStr, 5, "   1");
					else if (slide >= 1)
						snprintf(displayStr, 5, "0.%02u", (unsigned) slide);
					else
						snprintf(displayStr, 5, "   0");
				}
				else if (module->velEditMode == 1) {
					int prob = attributesVal.getGatePVal();
					if ( prob >= 100)
						snprintf(displayStr, 5, "   1");
					else if (prob >= 1)
						snprintf(displayStr, 5, "0.%02u", (unsigned) prob);
					else
						snprintf(displayStr, 5, "   0");
				}
				else {
					unsigned int velocityDisp = (unsigned)(attributesVal.getVelocityVal());
					if (module->velocityMode > 0) {// velocity is 0-127 or semitone
						if (module->velocityMode == 2)// semitone
							printNote(((float)velocityDisp)/12.0f - (module->velocityBipol ? 5.0f : 0.0f), &displayStr[1], true);// given str pointer must be 4 chars (3 display and one end of string)
						else// 0-127
							snprintf(displayStr, 5, " %3u", min(velocityDisp, 127));
						displayStr[0] = displayStr[1];
						displayStr[1] = ' ';
					}
					else {// velocity is 0-10V
						float cvValPrint = (float)velocityDisp;
						cvValPrint /= 20.0f;
						if (module->velocityBipol) {						
							if (cvValPrint < 5.0f)
								ret = 1;
							cvValPrint = fabsf(cvValPrint - 5.0f);
						}
						if (cvValPrint > 9.975f)
							snprintf(displayStr, 5, "  10");
						else if (cvValPrint < 0.025f)
							snprintf(displayStr, 5, "   0");
						else {
							snprintf(displayStr, 5, "%3.2f", cvValPrint);// Three-wide, two positions after the decimal, left-justified
							displayStr[1] = '.';// in case locals in printf
						}							
					}
				}
			}
			else 				
				snprintf(displayStr, 5, "  - ");
			return ret;
		}
	};
	

	struct SeqEditDisplayWidget : DisplayWidget<3> {
		SeqEditDisplayWidget(Vec _pos, Vec _size, Foundry *_module) : DisplayWidget(_pos, _size, _module) {};
		
		char printText() override {
			switch (module->displayState) {
			
				case Foundry::DISP_PPQN :
				case Foundry::DISP_DELAY :
					snprintf(displayStr, 4, " - ");
				break;
				case Foundry::DISP_REPS :
					snprintf(displayStr, 4, "R%2u", (unsigned) module->seq.getPhraseReps());
				break;
				case Foundry::DISP_COPY_SEQ :
					snprintf(displayStr, 4, "CPY");
				break;
				case Foundry::DISP_PASTE_SEQ :
					snprintf(displayStr, 4, "PST");
				break;
				case Foundry::DISP_LEN :
					snprintf(displayStr, 4, "L%2u", (unsigned) module->seq.getLength());
				break;
				case Foundry::DISP_TRANSPOSE :
				{
					int tranOffset = module->seq.getTransposeOffset();
					snprintf(displayStr, 4, "+%2u", (unsigned) abs(tranOffset));
					if (tranOffset < 0)
						displayStr[0] = '-';
				}
				break;
				case Foundry::DISP_ROTATE :
				{
					int rotOffset = module->seq.getRotateOffset();
					snprintf(displayStr, 4, ")%2u", (unsigned) abs(rotOffset));
					if (rotOffset < 0)
						displayStr[0] = '(';
				}
				break;
				default :
					// two paths below are equivalent when attached, so no need to check attached
					if (module->isEditingSequence()) {
						snprintf(displayStr, 4, " %2u", (unsigned)(module->seq.getSeqIndexEdit() + 1) );
						// if (module->displayState == Foundry::DISP_MODE_SEQ) {// Arrow
							// displayStr[0] = displayStr[1];
							// displayStr[1] = displayStr[2];
							// displayStr[2] = ')';
						// }
				break;

					}
					else
						snprintf(displayStr, 4, " %2u", (unsigned)(module->seq.getPhraseSeq() + 1) );
			}
			return 0;
		}
	};
	

	struct PhrEditDisplayWidget : DisplayWidget<3> {
		PhrEditDisplayWidget(Vec _pos, Vec _size, Foundry *_module) : DisplayWidget(_pos, _size, _module) {};

		char printText() override {
			char overlayChar = 0;// extra char to print an end symbol overlaped (begin symbol done in here)

			if (module->displayState == Foundry::DISP_COPY_SONG) {
				snprintf(displayStr, 4, "CPY");
			}
			else if (module->displayState == Foundry::DISP_PASTE_SONG) {
				snprintf(displayStr, 4, "PST");
			}
			else if (module->displayState == Foundry::DISP_MODE_SONG) {
				runModeToStr(module->seq.getRunModeSong());
			}
			else if (module->displayState == Foundry::DISP_PPQN) {
				snprintf(displayStr, 4, "x%2u", (unsigned) module->seq.getPulsesPerStep());
			}
			else if (module->displayState == Foundry::DISP_DELAY) {
				snprintf(displayStr, 4, "D%2u", (unsigned) module->seq.getDelay());
			}
			else if (module->displayState == Foundry::DISP_MODE_SEQ) {
				runModeToStr(module->seq.getRunModeSeq());
			}
			else { 
				if (module->isEditingSequence()) {
					snprintf(displayStr, 4, " - ");
				}
				else { // editing song
					int phrn = module->seq.getPhraseIndexEdit(); // good whether attached or not
					int phrBeg = module->seq.getBegin();
					int phrEnd = module->seq.getEnd();
					snprintf(displayStr, 4, " %2u", (unsigned)(phrn + 1));
					bool begHere = (phrn == phrBeg);
					bool endHere = (phrn == phrEnd);
					if (begHere) {
						displayStr[0] = '{';
						if (endHere)
							overlayChar = '}';
					}
					else if (endHere) {
						displayStr[0] = '}';
						overlayChar = '_';
					}
					else if (phrn < phrEnd && phrn > phrBeg)
						displayStr[0] = '_';
					if (module->displayState == Foundry::DISP_COPY_SONG_CUST) {
						overlayChar = 0;
						displayStr[0] = (time(0) & 0x1) ? 'C' : ' ';
					}
				}
			}
			return overlayChar;
		}
	};
	
	
	struct TrackDisplayWidget : DisplayWidget<2> {
		TrackDisplayWidget(Vec _pos, Vec _size, Foundry *_module) : DisplayWidget(_pos, _size, _module) {};
		char printText() override {
			int trkn = module->seq.getTrackIndexEdit();
			if (module->multiTracks)
				snprintf(displayStr, 3, "%c%c", (unsigned)(trkn + 0x41), ((module->multiTracks && (time(0) & 0x1)) ? '*' : ' '));
			else {
				snprintf(displayStr, 3, " %c", (unsigned)(trkn + 0x41));
				// if (module->displayState == Foundry::DISP_MODE_SONG || // Arrow
					// module->displayState == Foundry::DISP_PPQN || module->displayState == Foundry::DISP_DELAY)
					// displayStr[0] = '(';
			}
			return 0;
		}
	};
	

	struct PanelThemeItem : MenuItem {
		Foundry *module;
		int theme;
		void onAction(EventAction &e) override {
			module->panelTheme = theme;
		}
		void step() override {
			rightText = (module->panelTheme == theme) ? "✔" : "";
		}
	};
	struct ExpansionItem : MenuItem {
		Foundry *module;
		void onAction(EventAction &e) override {
			module->expansion = module->expansion == 1 ? 0 : 1;
		}
	};
	struct ResetOnRunItem : MenuItem {
		Foundry *module;
		void onAction(EventAction &e) override {
			module->resetOnRun = !module->resetOnRun;
		}
	};
	struct AutoStepLenItem : MenuItem {
		Foundry *module;
		void onAction(EventAction &e) override {
			module->autostepLen = !module->autostepLen;
		}
	};
	struct AutoseqItem : MenuItem {
		Foundry *module;
		void onAction(EventAction &e) override {
			module->autoseq = !module->autoseq;
		}
	};
	struct SeqCVmethodItem : MenuItem {
		Foundry *module;
		void onAction(EventAction &e) override {
			module->seqCVmethod++;
			if (module->seqCVmethod > 2)
				module->seqCVmethod = 0;
		}
		void step() override {
			if (module->seqCVmethod == 0)
				text = "Seq CV in: <0-10V>,  C2-D7#,  Trig-Incr";
			else if (module->seqCVmethod == 1)
				text = "Seq CV in: 0-10V,  <C2-D7#>,  Trig-Incr";
			else
				text = "Seq CV in: 0-10V,  C2-D7#,  <Trig-Incr>";
		}	
	};
	struct VelModeItem : MenuItem {
		Foundry *module;
		void onAction(EventAction &e) override {
			module->velocityMode++;
			if (module->velocityMode > 2)
				module->velocityMode = 0;
		}
		void step() override {
			if (module->velocityMode == 0)
				text = "CV2 mode: <Volts>,  0-127,  Notes";
			else if (module->velocityMode == 1)
				text = "CV2 mode: Volts,  <0-127>,  Notes";
			else
				text = "CV2 mode: Volts,  0-127,  <Notes>";
		}	
	};
	struct VelBipolItem : MenuItem {
		Foundry *module;
		void onAction(EventAction &e) override {
			module->velocityBipol = !module->velocityBipol;
		}
	};
	struct HoldTiedItem : MenuItem {
		Foundry *module;
		void onAction(EventAction &e) override {
			module->holdTiedNotes = !module->holdTiedNotes;
		}
	};
	Menu *createContextMenu() override {
		Menu *menu = ModuleWidget::createContextMenu();

		MenuLabel *spacerLabel = new MenuLabel();
		menu->addChild(spacerLabel);

		Foundry *module = dynamic_cast<Foundry*>(this->module);
		assert(module);

		MenuLabel *themeLabel = new MenuLabel();
		themeLabel->text = "Panel Theme";
		menu->addChild(themeLabel);

		PanelThemeItem *lightItem = new PanelThemeItem();
		lightItem->text = lightPanelID;// ImpromptuModular.hpp
		lightItem->module = module;
		lightItem->theme = 0;
		menu->addChild(lightItem);

		PanelThemeItem *metalItem = new PanelThemeItem();
		metalItem->text = "Liquid metal";// ImpromptuModular.hpp
		metalItem->module = module;
		metalItem->theme = 1;
		menu->addChild(metalItem);

		PanelThemeItem *darkItem = new PanelThemeItem();
		darkItem->text = darkPanelID;// ImpromptuModular.hpp
		darkItem->module = module;
		darkItem->theme = 2;
		menu->addChild(darkItem);

		menu->addChild(new MenuLabel());// empty line
		
		MenuLabel *settingsLabel = new MenuLabel();
		settingsLabel->text = "Settings";
		menu->addChild(settingsLabel);
		
		ResetOnRunItem *rorItem = MenuItem::create<ResetOnRunItem>("Reset on run", CHECKMARK(module->resetOnRun));
		rorItem->module = module;
		menu->addChild(rorItem);

		AutoStepLenItem *astlItem = MenuItem::create<AutoStepLenItem>("AutoStep write bounded by seq length", CHECKMARK(module->autostepLen));
		astlItem->module = module;
		menu->addChild(astlItem);

		AutoseqItem *aseqItem = MenuItem::create<AutoseqItem>("AutoSeq when writing via CV inputs", CHECKMARK(module->autoseq));
		aseqItem->module = module;
		menu->addChild(aseqItem);

		HoldTiedItem *holdItem = MenuItem::create<HoldTiedItem>("Hold tied notes", CHECKMARK(module->holdTiedNotes));
		holdItem->module = module;
		menu->addChild(holdItem);

		VelBipolItem *bipolItem = MenuItem::create<VelBipolItem>("CV2 bipolar", CHECKMARK(module->velocityBipol));
		bipolItem->module = module;
		menu->addChild(bipolItem);
		
		VelModeItem *velItem = MenuItem::create<VelModeItem>("CV2 mode: ", "");
		velItem->module = module;
		menu->addChild(velItem);
		
		SeqCVmethodItem *seqcvItem = MenuItem::create<SeqCVmethodItem>("Seq CV in: ", "");
		seqcvItem->module = module;
		menu->addChild(seqcvItem);
		
		menu->addChild(new MenuLabel());// empty line
		
		MenuLabel *expansionLabel = new MenuLabel();
		expansionLabel->text = "Expansion module";
		menu->addChild(expansionLabel);

		std::string expansionMenuLabel32EX(expansionMenuLabel);
		std::replace( expansionMenuLabel32EX.begin(), expansionMenuLabel32EX.end(), '4', '7');
		ExpansionItem *expItem = MenuItem::create<ExpansionItem>(expansionMenuLabel32EX, CHECKMARK(module->expansion != 0));
		expItem->module = module;
		menu->addChild(expItem);
		
		return menu;
	}	
	
	void step() override {
		if(module->expansion != oldExpansion) {
			if (oldExpansion != -1 && module->expansion == 0) {// if just removed expansion panel, disconnect wires to those jacks
				for (int i = 0; i < 12; i++)
					RACK_PLUGIN_UI_RACKWIDGET->wireContainer->removeAllWires(expPorts[i]);
				module->writeMode = 0;
			}
			oldExpansion = module->expansion;		
		}
		box.size.x = panel->box.size.x - (1 - module->expansion) * expWidth;
		Widget::step();
	}
	
	struct CKSSNotify : CKSS {// Not randomizable
		CKSSNotify() {}
		void randomize() override {}
		void onChange(EventChange &e) override {
			((Foundry*)(module))->displayState = Foundry::DISP_NORMAL;
			if (paramId != Foundry::KEY_GATE_PARAM) {
				((Foundry*)(module))->multiSteps = false;
			}
			SVGSwitch::onChange(e);		
		}
	};
	struct CKSSHThreeNotify : CKSSHThree {
		CKSSHThreeNotify() {};
		void onChange(EventChange &e) override {
			((Foundry*)(module))->displayState = Foundry::DISP_NORMAL;
			SVGSwitch::onChange(e);		
		}
	};
	struct CPModeSwitch : CKSSThreeInvNoRandom {// Not randomizable
		CPModeSwitch() {};
		void onChange(EventChange &e) override {
			SVGSwitch::onChange(e);	
			Foundry* module = dynamic_cast<Foundry*>(this->module);			
			module->cpSeqLength = module->getCPMode();
			if (module->displayState == Foundry::DISP_COPY_SONG_CUST)
				module->displayState = Foundry::DISP_NORMAL;
		}
	};
	struct VelocityKnob : IMMediumKnobInf {
		VelocityKnob() {};		
		void onMouseDown(EventMouseDown &e) override {// from ParamWidget.cpp
			Foundry* module = dynamic_cast<Foundry*>(this->module);
			if (e.button == 1) {
				// same code structure below as in velocity knob in main step()
				if (module->isEditingSequence() && !module->attached) {
					int multiStepsCount = module->multiSteps ? module->getCPMode() : 1;
					if (module->velEditMode == 2) {
						module->seq.initSlideVal(multiStepsCount, module->multiTracks);
					}
					else if (module->velEditMode == 1) {
						module->seq.initGatePVal(multiStepsCount, module->multiTracks);
					}
					else {
						module->seq.initVelocityVal(multiStepsCount, module->multiTracks);
					}
					module->displayState = Foundry::DISP_NORMAL;
				}
			}
			ParamWidget::onMouseDown(e);
		}
	};
	struct SequenceKnob : IMMediumKnobInf {
		SequenceKnob() {};		
		void onMouseDown(EventMouseDown &e) override {// from ParamWidget.cpp
			Foundry* module = dynamic_cast<Foundry*>(this->module);
			if (e.button == 1) {
				// same code structure below as in sequence knob in main step()
				if (module->displayState == Foundry::DISP_LEN) {
					module->seq.initLength(module->multiTracks);
				}
				else if (module->displayState == Foundry::DISP_TRANSPOSE) {
					module->seq.unTransposeSeq(module->multiTracks);
				}
				else if (module->displayState == Foundry::DISP_ROTATE) {
					module->seq.unRotateSeq(module->multiTracks);
				}							
				else if (module->displayState == Foundry::DISP_REPS) {
					module->seq.initPhraseReps(module->multiTracks);
				}
				else if (!module->attached) {
					if (module->isEditingSequence()) {
						if (!module->inputs[Foundry::SEQCV_INPUT].active)
							module->seq.setSeqIndexEdit(0);
					}
					else {// editing song
						module->seq.initPhraseSeqNum(module->multiTracks);
					}
					module->displayState = Foundry::DISP_NORMAL;
				}
			}
			ParamWidget::onMouseDown(e);
		}
	};
	struct PhraseKnob : IMMediumKnobInf {
		PhraseKnob() {};		
		void onMouseDown(EventMouseDown &e) override {// from ParamWidget.cpp
			Foundry* module = dynamic_cast<Foundry*>(this->module);
			if (e.button == 1) {
				// same code structure below as in phrase knob in main step()
				if (module->displayState == Foundry::DISP_MODE_SEQ) {
					module->seq.initRunModeSeq(module->multiTracks);
				}
				else if (module->displayState == Foundry::DISP_PPQN) {
					module->seq.initPulsesPerStep(module->multiTracks);
				}
				else if (module->displayState == Foundry::DISP_DELAY) {
					module->seq.initDelay(module->multiTracks);
				}
				else if (module->displayState == Foundry::DISP_MODE_SONG) {
					module->seq.initRunModeSong(module->multiTracks);
				}
				else if (!module->isEditingSequence() && !module->attached) {
					module->seq.setPhraseIndexEdit(0);
					module->displayState = Foundry::DISP_NORMAL;
				}
			}
			ParamWidget::onMouseDown(e);
		}
	};
		
	FoundryWidget(Foundry *module) : ModuleWidget(module) {
		this->module = module;
		oldExpansion = -1;
		
		// Main panel from Inkscape
        panel = new DynamicSVGPanel();
        panel->mode = &module->panelTheme;
		panel->expWidth = &expWidth;
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/Foundry.svg")));
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/Foundry_metal.svg")));
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/dark/Foundry_dark.svg")));
        box.size = panel->box.size;
		box.size.x = box.size.x - (1 - module->expansion) * expWidth;
        addChild(panel);
		
		// Screws
		addChild(createDynamicScrew<IMScrew>(Vec(15, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(15, 365), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(panel->box.size.x-30, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(panel->box.size.x-30, 365), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(panel->box.size.x-30-expWidth, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(panel->box.size.x-30-expWidth, 365), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(panel->box.size.x-expWidth + 15, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(panel->box.size.x-expWidth + 15, 365), &module->panelTheme));

		
		
		// ****** Top row ******
		
		static const int rowRulerT0 = 56;
		static const int columnRulerT0 = 25;// Step/Phase LED buttons
		static const int columnRulerT1 = 373;// Select (multi-steps) 
		static const int columnRulerT2 = 426;// Copy-paste-select mode switch
		//static const int columnRulerT3 = 463;// Copy paste buttons (not needed when align to track display)
		static const int columnRulerT5 = 538;// Edit mode switch (and overview switch also)
		static const int stepsOffsetY = 10;
		static const int posLEDvsButton = 26;

		// Step LED buttons
		int posX = columnRulerT0;
		static int spacingSteps = 20;
		static int spacingSteps4 = 4;
		const int numX = SequencerKernel::MAX_STEPS / 2;
		for (int x = 0; x < numX; x++) {
			// First row
			addParam(createParamCentered<LEDButton>(Vec(posX, rowRulerT0 - stepsOffsetY), module, Foundry::STEP_PHRASE_PARAMS + x, 0.0f, 1.0f, 0.0f));
			addChild(createLightCentered<MediumLight<GreenRedWhiteLight>>(Vec(posX, rowRulerT0 - stepsOffsetY), module, Foundry::STEP_PHRASE_LIGHTS + (x * 3)));
			// Second row
			addParam(createParamCentered<LEDButton>(Vec(posX, rowRulerT0 + stepsOffsetY), module, Foundry::STEP_PHRASE_PARAMS + x + numX, 0.0f, 1.0f, 0.0f));
			addChild(createLightCentered<MediumLight<GreenRedWhiteLight>>(Vec(posX, rowRulerT0 + stepsOffsetY), module, Foundry::STEP_PHRASE_LIGHTS + ((x + numX) * 3)));
			// step position to next location and handle groups of four
			posX += spacingSteps;
			if ((x + 1) % 4 == 0)
				posX += spacingSteps4;
		}
		// Sel button
		addParam(createDynamicParamCentered<IMPushButton>(Vec(columnRulerT1, rowRulerT0), module, Foundry::SEL_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		
		// Copy-paste and select mode switch (3 position)
		addParam(createParamCentered<CPModeSwitch>(Vec(columnRulerT2, rowRulerT0), module, Foundry::CPMODE_PARAM, 0.0f, 2.0f, 0.0f));	// 0.0f is top position
		
		// Copy/paste buttons
		// see under Track display
		
		// Main switch
		addParam(createParamCentered<CKSSNotify>(Vec(columnRulerT5, rowRulerT0), module, Foundry::EDIT_PARAM, 0.0f, 1.0f, 0.0f));// 1.0f is top position

		
		
		// ****** Octave and keyboard area ******
		
		// Octave LED buttons
		static const int octLightsIntY = 20;
		static const int rowRulerOct = 111;
		for (int i = 0; i < 7; i++) {
			addParam(createParamCentered<LEDButton>(Vec(columnRulerT0, rowRulerOct + i * octLightsIntY), module, Foundry::OCTAVE_PARAM + i, 0.0f, 1.0f, 0.0f));
			addChild(createLightCentered<MediumLight<RedLight>>(Vec(columnRulerT0, rowRulerOct + i * octLightsIntY), module, Foundry::OCTAVE_LIGHTS + i));
		}
		
		// Keys and Key lights
		static const int keyNudgeX = 2;
		static const int KeyBlackY = 103;
		static const int KeyWhiteY = 141;
		static const int offsetKeyLEDx = 6;
		static const int offsetKeyLEDy = 16;
		// Black keys and lights
		addParam(createParam<InvisibleKeySmall>(			Vec(65+keyNudgeX, KeyBlackY), module, Foundry::KEY_PARAMS + 1, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(65+keyNudgeX+offsetKeyLEDx, KeyBlackY+offsetKeyLEDy), module, Foundry::KEY_LIGHTS + 1 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(93+keyNudgeX, KeyBlackY), module, Foundry::KEY_PARAMS + 3, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(93+keyNudgeX+offsetKeyLEDx, KeyBlackY+offsetKeyLEDy), module, Foundry::KEY_LIGHTS + 3 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(150+keyNudgeX, KeyBlackY), module, Foundry::KEY_PARAMS + 6, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(150+keyNudgeX+offsetKeyLEDx, KeyBlackY+offsetKeyLEDy), module, Foundry::KEY_LIGHTS + 6 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(178+keyNudgeX, KeyBlackY), module, Foundry::KEY_PARAMS + 8, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(178+keyNudgeX+offsetKeyLEDx, KeyBlackY+offsetKeyLEDy), module, Foundry::KEY_LIGHTS + 8 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(206+keyNudgeX, KeyBlackY), module, Foundry::KEY_PARAMS + 10, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(206+keyNudgeX+offsetKeyLEDx, KeyBlackY+offsetKeyLEDy), module, Foundry::KEY_LIGHTS + 10 * 2));
		// White keys and lights
		addParam(createParam<InvisibleKeySmall>(			Vec(51+keyNudgeX, KeyWhiteY), module, Foundry::KEY_PARAMS + 0, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(51+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, Foundry::KEY_LIGHTS + 0 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(79+keyNudgeX, KeyWhiteY), module, Foundry::KEY_PARAMS + 2, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(79+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, Foundry::KEY_LIGHTS + 2 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(107+keyNudgeX, KeyWhiteY), module, Foundry::KEY_PARAMS + 4, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(107+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, Foundry::KEY_LIGHTS + 4 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(136+keyNudgeX, KeyWhiteY), module, Foundry::KEY_PARAMS + 5, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(136+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, Foundry::KEY_LIGHTS + 5 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(164+keyNudgeX, KeyWhiteY), module, Foundry::KEY_PARAMS + 7, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(164+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, Foundry::KEY_LIGHTS + 7 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(192+keyNudgeX, KeyWhiteY), module, Foundry::KEY_PARAMS + 9, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(192+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, Foundry::KEY_LIGHTS + 9 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(220+keyNudgeX, KeyWhiteY), module, Foundry::KEY_PARAMS + 11, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(220+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, Foundry::KEY_LIGHTS + 11 * 2));



		// ****** Right side control area ******
		
		static const int rowRulerDisp = 110;
		static const int rowRulerKnobs = 145;
		static const int rowRulerSmallButtons = 189;
		static const int displayWidths = 48; // 43 for 14pt, 46 for 15pt
		static const int displayHeights = 24; // 22 for 14pt, 24 for 15pt
		static const int displaySpacingX = 62;

		// Velocity display
		static const int colRulerVel = 289;
		static const int trkButtonsOffsetX = 14;
		addChild(new VelocityDisplayWidget(Vec(colRulerVel, rowRulerDisp), Vec(displayWidths + 4, displayHeights), module));// 3 characters
		// Velocity knob
		addParam(createDynamicParamCentered<VelocityKnob>(Vec(colRulerVel, rowRulerKnobs), module, Foundry::VEL_KNOB_PARAM, -INFINITY, INFINITY, 0.0f, &module->panelTheme));	
		// Veocity mode button and lights
		addParam(createDynamicParamCentered<IMPushButton>(Vec(colRulerVel - trkButtonsOffsetX - 2, rowRulerSmallButtons), module, Foundry::VEL_EDIT_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<MediumLight<GreenRedLight>>(Vec(colRulerVel + 4, rowRulerSmallButtons), module, Foundry::VEL_PROB_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(Vec(colRulerVel + 20, rowRulerSmallButtons), module, Foundry::VEL_SLIDE_LIGHT));
		

		// Seq edit display 
		static const int colRulerEditSeq = colRulerVel + displaySpacingX + 3;
		addChild(new SeqEditDisplayWidget(Vec(colRulerEditSeq, rowRulerDisp), Vec(displayWidths, displayHeights), module));// 5 characters
		// Sequence-edit knob
		addParam(createDynamicParamCentered<SequenceKnob>(Vec(colRulerEditSeq, rowRulerKnobs), module, Foundry::SEQUENCE_PARAM, -INFINITY, INFINITY, 0.0f, &module->panelTheme));		
		// Transpose/rotate button
		addParam(createDynamicParamCentered<IMPushButton>(Vec(colRulerEditSeq, rowRulerSmallButtons), module, Foundry::TRAN_ROT_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
	
			
		// Phrase edit display 
		static const int colRulerEditPhr = colRulerEditSeq + displaySpacingX + 1;
		addChild(new PhrEditDisplayWidget(Vec(colRulerEditPhr, rowRulerDisp), Vec(displayWidths, displayHeights), module));// 5 characters
		// Phrase knob
		addParam(createDynamicParamCentered<PhraseKnob>(Vec(colRulerEditPhr, rowRulerKnobs), module, Foundry::PHRASE_PARAM, -INFINITY, INFINITY, 0.0f, &module->panelTheme));		
		// Begin/end buttons
		addParam(createDynamicParamCentered<IMPushButton>(Vec(colRulerEditPhr - trkButtonsOffsetX, rowRulerSmallButtons), module, Foundry::BEGIN_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParamCentered<IMPushButton>(Vec(colRulerEditPhr + trkButtonsOffsetX, rowRulerSmallButtons), module, Foundry::END_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));

				
		// Track display
		static const int colRulerTrk = colRulerEditPhr + displaySpacingX;
		addChild(new TrackDisplayWidget(Vec(colRulerTrk, rowRulerDisp), Vec(displayWidths - 13, displayHeights), module));// 2 characters
		// Track buttons
		addParam(createDynamicParamCentered<IMPushButton>(Vec(colRulerTrk + trkButtonsOffsetX, rowRulerKnobs), module, Foundry::TRACKUP_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParamCentered<IMPushButton>(Vec(colRulerTrk - trkButtonsOffsetX, rowRulerKnobs), module, Foundry::TRACKDOWN_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// AllTracks button
		addParam(createDynamicParamCentered<IMPushButton>(Vec(colRulerTrk, rowRulerSmallButtons - 12), module, Foundry::ALLTRACKS_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Copy/paste buttons
		addParam(createDynamicParamCentered<IMPushButton>(Vec(colRulerTrk - trkButtonsOffsetX, rowRulerT0), module, Foundry::COPY_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParamCentered<IMPushButton>(Vec(colRulerTrk + trkButtonsOffsetX, rowRulerT0), module, Foundry::PASTE_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
	
	
		// Attach button and light
		addParam(createDynamicParamCentered<IMPushButton>(Vec(columnRulerT5 - 10, rowRulerDisp + 4), module, Foundry::ATTACH_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLightCentered<MediumLight<RedLight>>(Vec(columnRulerT5 + 10, rowRulerDisp + 4), module, Foundry::ATTACH_LIGHT));
	
	
		// ****** Gate and slide section ******
		
		static const int rowRulerMB0 = rowRulerOct + 6 * octLightsIntY;
		static const int columnRulerMB3 = colRulerVel - displaySpacingX;
		static const int columnRulerMB2 = colRulerVel - 2 * displaySpacingX;
		static const int columnRulerMB1 = colRulerVel - 3 * displaySpacingX;
		
		// Key mode LED buttons	
		static const int colRulerKM = 61;
		addParam(createParamCentered<CKSSNotify>(Vec(colRulerKM, rowRulerMB0), module, Foundry::KEY_GATE_PARAM, 0.0f, 1.0f, 1.0f));
		
		// Gate 1 light and button
		addChild(createLightCentered<MediumLight<GreenRedLight>>(Vec(columnRulerMB1 + posLEDvsButton, rowRulerMB0), module, Foundry::GATE_LIGHT));		
		addParam(createDynamicParamCentered<IMBigPushButton>(Vec(columnRulerMB1, rowRulerMB0), module, Foundry::GATE_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Tie light and button
		addChild(createLightCentered<MediumLight<RedLight>>(Vec(columnRulerMB2 + posLEDvsButton, rowRulerMB0), module, Foundry::TIE_LIGHT));		
		addParam(createDynamicParamCentered<IMBigPushButton>(Vec(columnRulerMB2, rowRulerMB0), module, Foundry::TIE_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Gate 1 probability light and button
		addChild(createLightCentered<MediumLight<GreenRedLight>>(Vec(columnRulerMB3 + posLEDvsButton, rowRulerMB0), module, Foundry::GATE_PROB_LIGHT));		
		addParam(createDynamicParamCentered<IMBigPushButton>(Vec(columnRulerMB3, rowRulerMB0), module, Foundry::GATE_PROB_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		
		// Slide light and button
		addChild(createLightCentered<MediumLight<RedLight>>(Vec(colRulerVel + posLEDvsButton, rowRulerMB0), module, Foundry::SLIDE_LIGHT));		
		addParam(createDynamicParamCentered<IMBigPushButton>(Vec(colRulerVel, rowRulerMB0), module, Foundry::SLIDE_BTN_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Mode button
		addParam(createDynamicParamCentered<IMBigPushButton>(Vec(colRulerEditPhr, rowRulerMB0), module, Foundry::MODE_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Rep/Len button
		addParam(createDynamicParamCentered<IMBigPushButton>(Vec(colRulerEditSeq, rowRulerMB0), module, Foundry::REP_LEN_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Clk res
		addParam(createDynamicParamCentered<IMBigPushButton>(Vec(colRulerTrk, rowRulerMB0), module, Foundry::CLKRES_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		
		// Reset and run LED buttons
		static const int colRulerResetRun = columnRulerT5;
		// Run LED bezel and light
		addParam(createParamCentered<LEDBezel>(Vec(colRulerResetRun, rowRulerSmallButtons - 6), module, Foundry::RUN_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(createLightCentered<MuteLight<GreenLight>>(Vec(colRulerResetRun, rowRulerSmallButtons - 6), module, Foundry::RUN_LIGHT));
		// Reset LED bezel and light
		addParam(createParamCentered<LEDBezel>(Vec(colRulerResetRun, rowRulerMB0), module, Foundry::RESET_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(createLightCentered<MuteLight<GreenLight>>(Vec(colRulerResetRun, rowRulerMB0), module, Foundry::RESET_LIGHT));
		


		
		// ****** Bottom two rows ******
		
		static const int rowRulerBLow = 335;
		static const int rowRulerBHigh = 286;
		
		static const int bottomJackSpacingX = 46;
		static const int columnRulerB0 = 32;
		static const int columnRulerB1 = columnRulerB0 + bottomJackSpacingX;
		static const int columnRulerB2 = columnRulerB1 + bottomJackSpacingX;
		static const int columnRulerB3 = columnRulerB2 + bottomJackSpacingX;
		static const int columnRulerB4 = columnRulerB3 + bottomJackSpacingX;
		static const int columnRulerB5 = columnRulerB4 + bottomJackSpacingX;
		
		static const int columnRulerB6 = columnRulerB5 + bottomJackSpacingX;
		static const int columnRulerB7 = columnRulerB6 + bottomJackSpacingX;
		static const int columnRulerB8 = columnRulerB7 + bottomJackSpacingX;
		static const int columnRulerB9 = columnRulerB8 + bottomJackSpacingX;
		static const int columnRulerB10 = columnRulerB9 + bottomJackSpacingX;
		static const int columnRulerB11 = columnRulerB10 + bottomJackSpacingX;
		

		// Autostep and write
		addParam(createParamCentered<CKSSNoRandom>(Vec(columnRulerB0, rowRulerBHigh), module, Foundry::AUTOSTEP_PARAM, 0.0f, 1.0f, 1.0f));		
		addInput(createDynamicPortCentered<IMPort>(Vec(columnRulerB0, rowRulerBLow), Port::INPUT, module, Foundry::WRITE_INPUT, &module->panelTheme));
	
		// CV IN inputs
		static const int writeLEDoffsetX = 16;
		static const int writeLEDoffsetY = 18;
		addInput(createDynamicPortCentered<IMPort>(Vec(columnRulerB1, rowRulerBHigh), Port::INPUT, module, Foundry::CV_INPUTS + 0, &module->panelTheme));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(columnRulerB1 + writeLEDoffsetX, rowRulerBHigh + writeLEDoffsetY), module, Foundry::WRITECV_LIGHTS + 0));
		
		addInput(createDynamicPortCentered<IMPort>(Vec(columnRulerB2, rowRulerBHigh), Port::INPUT, module, Foundry::CV_INPUTS + 2, &module->panelTheme));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(columnRulerB2 - writeLEDoffsetX, rowRulerBHigh + writeLEDoffsetY), module, Foundry::WRITECV_LIGHTS + 2));

		addInput(createDynamicPortCentered<IMPort>(Vec(columnRulerB1, rowRulerBLow), Port::INPUT, module, Foundry::CV_INPUTS + 1, &module->panelTheme));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(columnRulerB1 + writeLEDoffsetX, rowRulerBLow - writeLEDoffsetY), module, Foundry::WRITECV_LIGHTS + 1));

		addInput(createDynamicPortCentered<IMPort>(Vec(columnRulerB2, rowRulerBLow), Port::INPUT, module, Foundry::CV_INPUTS + 3, &module->panelTheme));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(columnRulerB2 - writeLEDoffsetX, rowRulerBLow - writeLEDoffsetY), module, Foundry::WRITECV_LIGHTS + 3));
		
		// Clock+CV+Gate+Vel outputs
		// Track A
		addInput(createDynamicPortCentered<IMPort>(Vec(columnRulerB3, rowRulerBHigh), Port::INPUT, module, Foundry::CLOCK_INPUTS + 0, &module->panelTheme));
		addOutput(createDynamicPortCentered<IMPort>(Vec(columnRulerB4, rowRulerBHigh), Port::OUTPUT, module, Foundry::CV_OUTPUTS + 0, &module->panelTheme));
		addOutput(createDynamicPortCentered<IMPort>(Vec(columnRulerB5, rowRulerBHigh), Port::OUTPUT, module, Foundry::GATE_OUTPUTS + 0, &module->panelTheme));
		addOutput(createDynamicPortCentered<IMPort>(Vec(columnRulerB6, rowRulerBHigh), Port::OUTPUT, module, Foundry::VEL_OUTPUTS + 0, &module->panelTheme));
		// Track C
		addInput(createDynamicPortCentered<IMPort>(Vec(columnRulerB7, rowRulerBHigh), Port::INPUT, module, Foundry::CLOCK_INPUTS + 2, &module->panelTheme));
		addOutput(createDynamicPortCentered<IMPort>(Vec(columnRulerB8, rowRulerBHigh), Port::OUTPUT, module, Foundry::CV_OUTPUTS + 2, &module->panelTheme));
		addOutput(createDynamicPortCentered<IMPort>(Vec(columnRulerB9, rowRulerBHigh), Port::OUTPUT, module, Foundry::GATE_OUTPUTS + 2, &module->panelTheme));
		addOutput(createDynamicPortCentered<IMPort>(Vec(columnRulerB10, rowRulerBHigh), Port::OUTPUT, module, Foundry::VEL_OUTPUTS + 2, &module->panelTheme));
		//
		// Track B
		addInput(createDynamicPortCentered<IMPort>(Vec(columnRulerB3, rowRulerBLow), Port::INPUT, module, Foundry::CLOCK_INPUTS + 1, &module->panelTheme));
		addOutput(createDynamicPortCentered<IMPort>(Vec(columnRulerB4, rowRulerBLow), Port::OUTPUT, module, Foundry::CV_OUTPUTS + 1, &module->panelTheme));
		addOutput(createDynamicPortCentered<IMPort>(Vec(columnRulerB5, rowRulerBLow), Port::OUTPUT, module, Foundry::GATE_OUTPUTS + 1, &module->panelTheme));
		addOutput(createDynamicPortCentered<IMPort>(Vec(columnRulerB6, rowRulerBLow), Port::OUTPUT, module, Foundry::VEL_OUTPUTS + 1, &module->panelTheme));
		// Track D
		addInput(createDynamicPortCentered<IMPort>(Vec(columnRulerB7, rowRulerBLow), Port::INPUT, module, Foundry::CLOCK_INPUTS + 3, &module->panelTheme));
		addOutput(createDynamicPortCentered<IMPort>(Vec(columnRulerB8, rowRulerBLow), Port::OUTPUT, module, Foundry::CV_OUTPUTS + 3, &module->panelTheme));
		addOutput(createDynamicPortCentered<IMPort>(Vec(columnRulerB9, rowRulerBLow), Port::OUTPUT, module, Foundry::GATE_OUTPUTS + 3, &module->panelTheme));
		addOutput(createDynamicPortCentered<IMPort>(Vec(columnRulerB10, rowRulerBLow), Port::OUTPUT, module, Foundry::VEL_OUTPUTS + 3, &module->panelTheme));

		// Run and reset inputs
		addInput(createDynamicPortCentered<IMPort>(Vec(columnRulerB11, rowRulerBHigh), Port::INPUT, module, Foundry::RUNCV_INPUT, &module->panelTheme));
		addInput(createDynamicPortCentered<IMPort>(Vec(columnRulerB11, rowRulerBLow), Port::INPUT, module, Foundry::RESET_INPUT, &module->panelTheme));
		
		
		
		// Expansion module
		static const int rowRulerExpTop = 67;
		static const int rowSpacingExp = 51;
		static const int colRulerExp = panel->box.size.x - expWidth / 2;
		static const int colOffsetX = 23;
		addInput(expPorts[4] = createDynamicPortCentered<IMPort>(Vec(colRulerExp - colOffsetX, rowRulerExpTop + rowSpacingExp * 0), Port::INPUT, module, Foundry::SEQCV_INPUT, &module->panelTheme));
		addInput(expPorts[5] = createDynamicPortCentered<IMPort>(Vec(colRulerExp + colOffsetX, rowRulerExpTop + rowSpacingExp * 0), Port::INPUT, module, Foundry::TRKCV_INPUT, &module->panelTheme));
		
		// Step arrow CV inputs
		addInput(expPorts[6] = createDynamicPortCentered<IMPort>(Vec(colRulerExp - colOffsetX, rowRulerExpTop + rowSpacingExp * 1), Port::INPUT, module, Foundry::LEFTCV_INPUT, &module->panelTheme));
		addInput(expPorts[7] = createDynamicPortCentered<IMPort>(Vec(colRulerExp + colOffsetX, rowRulerExpTop + rowSpacingExp * 1), Port::INPUT, module, Foundry::RIGHTCV_INPUT, &module->panelTheme));

		addInput(expPorts[0] = createDynamicPortCentered<IMPort>(Vec(colRulerExp - colOffsetX, rowRulerExpTop + rowSpacingExp * 2), Port::INPUT, module, Foundry::GATECV_INPUT, &module->panelTheme));
		addInput(expPorts[1] = createDynamicPortCentered<IMPort>(Vec(colRulerExp + colOffsetX, rowRulerExpTop + rowSpacingExp * 2), Port::INPUT, module, Foundry::TIEDCV_INPUT, &module->panelTheme));
		
		addInput(expPorts[2] = createDynamicPortCentered<IMPort>(Vec(colRulerExp - colOffsetX, rowRulerExpTop + rowSpacingExp * 3), Port::INPUT, module, Foundry::GATEPCV_INPUT, &module->panelTheme));
		addInput(expPorts[3] = createDynamicPortCentered<IMPort>(Vec(colRulerExp + colOffsetX, rowRulerExpTop + rowSpacingExp * 3), Port::INPUT, module, Foundry::SLIDECV_INPUT, &module->panelTheme));
		
		addParam(createDynamicParamCentered<IMPushButton>(Vec(colRulerExp, rowRulerExpTop + 180), module, Foundry::WRITEMODE_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		
		// Velocity inputs 
		addInput(expPorts[8] = createDynamicPortCentered<IMPort>(Vec(colRulerExp - colOffsetX, rowRulerBHigh), Port::INPUT, module, Foundry::VEL_INPUTS + 0, &module->panelTheme));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(colRulerExp - colOffsetX + writeLEDoffsetX, rowRulerBHigh + writeLEDoffsetY), module, Foundry::WRITECV2_LIGHTS + 0));
		
		addInput(expPorts[9] = createDynamicPortCentered<IMPort>(Vec(colRulerExp + colOffsetX, rowRulerBHigh), Port::INPUT, module, Foundry::VEL_INPUTS + 2, &module->panelTheme));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(colRulerExp + colOffsetX - writeLEDoffsetX, rowRulerBHigh + writeLEDoffsetY), module, Foundry::WRITECV2_LIGHTS + 2));

		addInput(expPorts[10] = createDynamicPortCentered<IMPort>(Vec(colRulerExp - colOffsetX, rowRulerBLow), Port::INPUT, module, Foundry::VEL_INPUTS + 1, &module->panelTheme));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(colRulerExp - colOffsetX + writeLEDoffsetX, rowRulerBLow - writeLEDoffsetY), module, Foundry::WRITECV2_LIGHTS + 1));

		addInput(expPorts[11] = createDynamicPortCentered<IMPort>(Vec(colRulerExp + colOffsetX, rowRulerBLow), Port::INPUT, module, Foundry::VEL_INPUTS + 3, &module->panelTheme));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(colRulerExp + colOffsetX - writeLEDoffsetX, rowRulerBLow - writeLEDoffsetY), module, Foundry::WRITECV2_LIGHTS + 3));
	}
};

} // namespace rack_plugin_ImpromptuModular

using namespace rack_plugin_ImpromptuModular;

RACK_PLUGIN_MODEL_INIT(ImpromptuModular, Foundry) {
   Model *modelFoundry = Model::create<Foundry, FoundryWidget>("Impromptu Modular", "Foundry", "SEQ - Foundry", SEQUENCER_TAG);
   return modelFoundry;
}

/*CHANGE LOG

0.6.16:
add gate status feedback in steps (white lights)

0.6.15:
save ALL state and don't init ALL nor SEL on run or reset 
allow END in seq mode to choose custom seq lengths
add proper CV2 monitoring when not running and editing sequences
fix TKA song runmode slaving
add right-click menu option to bound AutoStep writes by sequence lengths

0.6.14: 
allow ctrl-right-click of notes to copy note/gate-type over to next step (not just move to next step)
add CV IN and CV2 IN LEDs, and make CV2 button, when returning to CV2 mode, toggle which CV or CV2 groups can be written (see LEDs)
add CV2 bipol option in right click menu; display notes for semitone CV2 mode (now called "Notes")
rotate offsets are now persistent and stored in the sequencer
expand TRACK cv input to allow starred tracks (ALL) to be selected, and write CVs only to selected track when not starred

0.6.13:
created

*/
