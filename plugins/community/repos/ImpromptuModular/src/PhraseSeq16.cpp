//***********************************************************************************************
//Multi-phrase 16 step sequencer module for VCV Rack by Marc Boulé
//
//Based on code from the Fundamental and AudibleInstruments plugins by Andrew Belt 
//and graphics from the Component Library by Wes Milholen 
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//
//Module inspired by the SA-100 Stepper Acid sequencer by Transistor Sounds Labs
//
//Acknowledgements: please see README.md
//***********************************************************************************************


#include "ImpromptuModular.hpp"
#include "PhraseSeqUtil.hpp"

namespace rack_plugin_ImpromptuModular {

struct PhraseSeq16 : Module {
	enum ParamIds {
		LEFT_PARAM,
		RIGHT_PARAM,
		LENGTH_PARAM,
		EDIT_PARAM,
		SEQUENCE_PARAM,
		RUN_PARAM,
		COPY_PARAM,
		PASTE_PARAM,
		RESET_PARAM,
		ENUMS(OCTAVE_PARAM, 7),
		GATE1_PARAM,
		GATE2_PARAM,
		SLIDE_BTN_PARAM,
		SLIDE_KNOB_PARAM,
		ATTACH_PARAM,
		ROTATEL_PARAM,// no longer used
		ROTATER_PARAM,// no longer used
		PASTESYNC_PARAM,// no longer used
		AUTOSTEP_PARAM,
		ENUMS(KEY_PARAMS, 12),
		TRANSPOSEU_PARAM,// no longer used
		TRANSPOSED_PARAM,// no longer used
		// -- 0.6.2 ^^
		RUNMODE_PARAM,
		TRAN_ROT_PARAM,
		ROTATE_PARAM,//no longer used
		GATE1_KNOB_PARAM,
		GATE2_KNOB_PARAM,// no longer used
		GATE1_PROB_PARAM,
		TIE_PARAM,// Legato
		// -- 0.6.3 ^^
		CPMODE_PARAM,
		// -- 0.6.4 ^^
		NUM_PARAMS
	};
	enum InputIds {
		WRITE_INPUT,
		CV_INPUT,
		RESET_INPUT,
		CLOCK_INPUT,
		// -- 0.6.2 ^^
		LEFTCV_INPUT,
		RIGHTCV_INPUT,
		RUNCV_INPUT,
		SEQCV_INPUT,
		MODECV_INPUT,
		// -- 0.6.3 ^^
		// -- 0.6.4 ^^
		GATE1CV_INPUT,
		GATE2CV_INPUT,
		TIEDCV_INPUT,
		SLIDECV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CV_OUTPUT,
		GATE1_OUTPUT,
		GATE2_OUTPUT,
		// -- 0.6.2 ^^
		// -- 0.6.3 ^^
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(STEP_PHRASE_LIGHTS, 16 * 2),// room for GreenRed
		ENUMS(OCTAVE_LIGHTS, 7),// octaves 1 to 7
		ENUMS(KEY_LIGHTS, 12 * 2),// room for GreenRed
		RUN_LIGHT,
		RESET_LIGHT,
		ENUMS(GATE1_LIGHT, 2),// room for GreenRed
		ENUMS(GATE2_LIGHT, 2),// room for GreenRed
		SLIDE_LIGHT,
		ATTACH_LIGHT,
		PENDING_LIGHT,// no longer used
		// -- 0.6.2 ^^
		GATE1_PROB_LIGHT,
		// -- 0.6.3 ^^
		TIE_LIGHT,
		NUM_LIGHTS
	};
	
	enum DisplayStateIds {DISP_NORMAL, DISP_MODE, DISP_TRANSPOSE, DISP_ROTATE};
	

	// Need to save
	int panelTheme = 0;
	int expansion = 0;
	int pulsesPerStep;// 1 means normal gate mode, alt choices are 4, 6, 12, 24 PPS (Pulses per step)
	bool running;
	int runModeSeq[16]; 
	int runModeSong;
	//
	int sequence;
	int lengths[16];//1 to 16
	//
	int phrase[16];// This is the song (series of phases; a phrase is a patten number)
	int phrases;//1 to 16
	//
	float cv[16][16];// [-3.0 : 3.917]. First index is patten number, 2nd index is step
	int attributes[16][16];// First index is patten number, 2nd index is step (see enum AttributeBitMasks for details)
	//
	bool resetOnRun;
	bool attached;

	// No need to save
	float resetLight = 0.0f;
	int stepIndexEdit;
	int stepIndexRun;
	int phraseIndexEdit;
	int phraseIndexRun;
	unsigned long editingLength;// 0 when not editing length, downward step counter timer when editing length
	long infoCopyPaste;// 0 when no info, positive downward step counter timer when copy, negative upward when paste
	unsigned long editingGate;// 0 when no edit gate, downward step counter timer when edit gate
	float editingGateCV;// no need to initialize, this is a companion to editingGate (output this only when editingGate > 0)
	int editingGateKeyLight;// no need to initialize, this is a companion to editingGate (use this only when editingGate > 0)
	int stepIndexRunHistory;// no need to initialize
	int phraseIndexRunHistory;// no need to initialize
	int displayState;
	unsigned long slideStepsRemain;// 0 when no slide under way, downward step counter when sliding
	float slideCVdelta;// no need to initialize, this is a companion to slideStepsRemain
	float cvCPbuffer[16];// copy paste buffer for CVs
	int attributesCPbuffer[16];// copy paste buffer for attributes
	int lengthCPbuffer;
	int modeCPbuffer;
	int countCP;// number of steps to paste (in case CPMODE_PARAM changes between copy and paste)
	int transposeOffset;// no need to initialize, this is companion to displayMode = DISP_TRANSPOSE
	int rotateOffset;// no need to initialize, this is companion to displayMode = DISP_ROTATE
	long clockIgnoreOnReset;
	const float clockIgnoreOnResetDuration = 0.001f;// disable clock on powerup and reset for 1 ms (so that the first step plays)
	unsigned long clockPeriod;// counts number of step() calls upward from last clock (reset after clock processed)
	long tiedWarning;// 0 when no warning, positive downward step counter timer when warning
	int sequenceKnob = 0;
	int gate1Code;
	int gate2Code;
	long editingGateLength;// 0 when no info, positive downward step counter timer when gate1, negative upward when gate2
	long editGateLengthTimeInitMult;// multiplier for extended setting of advanced gates
	long editingPpqn;// 0 when no info, positive downward step counter timer when editing ppqn
	int ppqnCount;
	int lightRefreshCounter;

	static constexpr float EDIT_PARAM_INIT_VALUE = 1.0f;// so that module constructor is coherent with widget initialization, since module created before widget
	bool editingSequence;
	bool editingSequenceLast;

	
	SchmittTrigger resetTrigger;
	SchmittTrigger leftTrigger;
	SchmittTrigger rightTrigger;
	SchmittTrigger runningTrigger;
	SchmittTrigger clockTrigger;
	SchmittTrigger octTriggers[7];
	SchmittTrigger octmTrigger;
	SchmittTrigger gate1Trigger;
	SchmittTrigger gate1ProbTrigger;
	SchmittTrigger gate2Trigger;
	SchmittTrigger slideTrigger;
	SchmittTrigger lengthTrigger;
	SchmittTrigger keyTriggers[12];
	SchmittTrigger writeTrigger;
	SchmittTrigger attachedTrigger;
	SchmittTrigger copyTrigger;
	SchmittTrigger pasteTrigger;
	SchmittTrigger modeTrigger;
	SchmittTrigger rotateTrigger;
	SchmittTrigger transposeTrigger;
	SchmittTrigger tiedTrigger;
	HoldDetect modeHoldDetect;
	HoldDetect gate1HoldDetect;
	HoldDetect gate2HoldDetect;

	
	inline bool isEditingSequence(void) {return params[EDIT_PARAM].value > 0.5f;}
	inline bool getGate1(int seq, int step) {return getGate1a(attributes[seq][step]);}
	inline bool getGate1P(int seq, int step) {return getGate1Pa(attributes[seq][step]);}
	inline bool getGate2(int seq, int step) {return getGate2a(attributes[seq][step]);}
	inline bool getSlide(int seq, int step) {return getSlideA(attributes[seq][step]);}
	inline bool getTied(int seq, int step) {return getTiedA(attributes[seq][step]);}
	inline int getGate1Mode(int seq, int step) {return getGate1aMode(attributes[seq][step]);}
	inline int getGate2Mode(int seq, int step) {return getGate2aMode(attributes[seq][step]);}
	
	inline void setGate1Mode(int seq, int step, int gateMode) {attributes[seq][step] &= ~ATT_MSK_GATE1MODE; attributes[seq][step] |= (gateMode << gate1ModeShift);}
	inline void setGate2Mode(int seq, int step, int gateMode) {attributes[seq][step] &= ~ATT_MSK_GATE2MODE; attributes[seq][step] |= (gateMode << gate2ModeShift);}

	
	PhraseSeq16() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}
	

	void onReset() override {
		pulsesPerStep = 1;
		running = false;
		runModeSong = MODE_FWD;
		stepIndexEdit = 0;
		phraseIndexEdit = 0;
		sequence = 0;
		phrases = 4;
		for (int i = 0; i < 16; i++) {
			for (int s = 0; s < 16; s++) {
				cv[i][s] = 0.0f;
				attributes[i][s] = ATT_MSK_GATE1;
			}
			runModeSeq[i] = MODE_FWD;
			phrase[i] = 0;
			lengths[i] = 16;
			cvCPbuffer[i] = 0.0f;
			attributesCPbuffer[i] = ATT_MSK_GATE1;
		}
		initRun(true);
		lengthCPbuffer = 16;
		modeCPbuffer = MODE_FWD;
		countCP = 16;
		editingLength = 0ul;
		editingGate = 0ul;
		infoCopyPaste = 0l;
		displayState = DISP_NORMAL;
		slideStepsRemain = 0ul;
		attached = true;
		clockPeriod = 0ul;
		tiedWarning = 0ul;
		editingSequence = EDIT_PARAM_INIT_VALUE > 0.5f;
		editingSequenceLast = editingSequence;
		resetOnRun = false;
		modeHoldDetect.reset();
		gate1HoldDetect.reset();
		gate2HoldDetect.reset();
		editGateLengthTimeInitMult = 1l;
		editingPpqn = 0l;
		lightRefreshCounter = 0;
	}

	
	void onRandomize() override {
		running = false;
		runModeSong = randomu32() % 5;
		stepIndexEdit = 0;
		phraseIndexEdit = 0;
		sequence = randomu32() % 16;
		phrases = 1 + (randomu32() % 16);
		for (int i = 0; i < 16; i++) {
			for (int s = 0; s < 16; s++) {
				cv[i][s] = ((float)(randomu32() % 7)) + ((float)(randomu32() % 12)) / 12.0f - 3.0f;
				attributes[i][s] = randomu32() & 0x1FFF;// 5 bit for normal attributes + 2 * 4 bits for advanced gate modes
				if (getTied(i,s)) {
					attributes[i][s] = ATT_MSK_TIED;// clear other attributes if tied
					applyTiedStep(i, s, lengths[i]);
				}
			}
			runModeSeq[i] = randomu32() % NUM_MODES;
			phrase[i] = randomu32() % 16;
			lengths[i] = 1 + (randomu32() % 16);
			cvCPbuffer[i] = 0.0f;
			attributesCPbuffer[i] = ATT_MSK_GATE1;
		}
		initRun(true);
		lengthCPbuffer = 16;
		modeCPbuffer = MODE_FWD;
		countCP = 16;
		editingLength = 0ul;
		editingGate = 0ul;
		infoCopyPaste = 0l;
		displayState = DISP_NORMAL;
		slideStepsRemain = 0ul;
		attached = true;
		clockPeriod = 0ul;
		tiedWarning = 0ul;
		editingSequence = isEditingSequence();
		editingSequenceLast = editingSequence;
		resetOnRun = false;
		modeHoldDetect.reset();
		gate1HoldDetect.reset();
		gate2HoldDetect.reset();
		editGateLengthTimeInitMult = 1l;
		editingPpqn = 0l;
	}
	
	
	void initRun(bool hard) {// run button activated or run edge in run input jack or edit mode toggled
		if (hard) 
			phraseIndexRun = (runModeSong == MODE_REV ? phrases - 1 : 0);
		int seq = (editingSequence ? sequence : phrase[phraseIndexRun]);
		if (hard)	
			stepIndexRun = (runModeSeq[seq] == MODE_REV ? lengths[seq] - 1 : 0);
		ppqnCount = 0;
		gate1Code = calcGate1Code(attributes[seq][stepIndexRun], 0, pulsesPerStep, params[GATE1_KNOB_PARAM].value);
		gate2Code = calcGate2Code(attributes[seq][stepIndexRun], 0, pulsesPerStep);
		clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * engineGetSampleRate());
		editingGateLength = 0l;
	}
	
	
	json_t *toJson() override {
		json_t *rootJ = json_object();

		// panelTheme
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));

		// expansion
		json_object_set_new(rootJ, "expansion", json_integer(expansion));

		// pulsesPerStep
		json_object_set_new(rootJ, "pulsesPerStep", json_integer(pulsesPerStep));

		// running
		json_object_set_new(rootJ, "running", json_boolean(running));
		
		// runModeSeq
		json_t *runModeSeqJ = json_array();
		for (int i = 0; i < 16; i++)
			json_array_insert_new(runModeSeqJ, i, json_integer(runModeSeq[i]));
		json_object_set_new(rootJ, "runModeSeq2", runModeSeqJ);// "2" appended so no break patches

		// runModeSong
		json_object_set_new(rootJ, "runModeSong", json_integer(runModeSong));

		// sequence
		json_object_set_new(rootJ, "sequence", json_integer(sequence));

		// lengths
		json_t *lengthsJ = json_array();
		for (int i = 0; i < 16; i++)
			json_array_insert_new(lengthsJ, i, json_integer(lengths[i]));
		json_object_set_new(rootJ, "lengths", lengthsJ);

		// phrase 
		json_t *phraseJ = json_array();
		for (int i = 0; i < 16; i++)
			json_array_insert_new(phraseJ, i, json_integer(phrase[i]));
		json_object_set_new(rootJ, "phrase", phraseJ);

		// phrases
		json_object_set_new(rootJ, "phrases", json_integer(phrases));

		// CV
		json_t *cvJ = json_array();
		for (int i = 0; i < 16; i++)
			for (int s = 0; s < 16; s++) {
				json_array_insert_new(cvJ, s + (i * 16), json_real(cv[i][s]));
			}
		json_object_set_new(rootJ, "cv", cvJ);

		// attributes
		json_t *attributesJ = json_array();
		for (int i = 0; i < 16; i++)
			for (int s = 0; s < 16; s++) {
				json_array_insert_new(attributesJ, s + (i * 16), json_integer(attributes[i][s]));
			}
		json_object_set_new(rootJ, "attributes", attributesJ);

		// attached
		json_object_set_new(rootJ, "attached", json_boolean(attached));

		// resetOnRun
		json_object_set_new(rootJ, "resetOnRun", json_boolean(resetOnRun));
		
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

		// pulsesPerStep
		json_t *pulsesPerStepJ = json_object_get(rootJ, "pulsesPerStep");
		if (pulsesPerStepJ)
			pulsesPerStep = json_integer_value(pulsesPerStepJ);

		// running
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);

		// runModeSeq
		json_t *runModeSeqJ = json_object_get(rootJ, "runModeSeq2");// "2" appended so no break patches
		if (runModeSeqJ) {
			for (int i = 0; i < 16; i++)
			{
				json_t *runModeSeqArrayJ = json_array_get(runModeSeqJ, i);
				if (runModeSeqArrayJ)
					runModeSeq[i] = json_integer_value(runModeSeqArrayJ);
			}			
		}		
		else {// legacy
			runModeSeqJ = json_object_get(rootJ, "runModeSeq");
			if (runModeSeqJ)
				runModeSeq[0] = json_integer_value(runModeSeqJ);
			for (int i = 1; i < 16; i++)
				runModeSeq[i] = runModeSeq[0];
		}
		
		// runModeSong
		json_t *runModeSongJ = json_object_get(rootJ, "runModeSong");
		if (runModeSongJ)
			runModeSong = json_integer_value(runModeSongJ);
		
		// sequence
		json_t *sequenceJ = json_object_get(rootJ, "sequence");
		if (sequenceJ)
			sequence = json_integer_value(sequenceJ);
		
		// lengths
		json_t *lengthsJ = json_object_get(rootJ, "lengths");
		if (lengthsJ) {
			for (int i = 0; i < 16; i++)
			{
				json_t *lengthsArrayJ = json_array_get(lengthsJ, i);
				if (lengthsArrayJ)
					lengths[i] = json_integer_value(lengthsArrayJ);
			}			
		}
		else {// legacy
			json_t *stepsJ = json_object_get(rootJ, "steps");
			if (stepsJ) {
				int steps = json_integer_value(stepsJ);
				for (int i = 0; i < 16; i++)
					lengths[i] = steps;
			}
		}
		
		// phrase
		json_t *phraseJ = json_object_get(rootJ, "phrase");
		if (phraseJ)
			for (int i = 0; i < 16; i++)
			{
				json_t *phraseArrayJ = json_array_get(phraseJ, i);
				if (phraseArrayJ)
					phrase[i] = json_integer_value(phraseArrayJ);
			}
			
		// phrases
		json_t *phrasesJ = json_object_get(rootJ, "phrases");
		if (phrasesJ)
			phrases = json_integer_value(phrasesJ);
		
		// CV
		json_t *cvJ = json_object_get(rootJ, "cv");
		if (cvJ) {
			for (int i = 0; i < 16; i++)
				for (int s = 0; s < 16; s++) {
					json_t *cvArrayJ = json_array_get(cvJ, s + (i * 16));
					if (cvArrayJ)
						cv[i][s] = json_real_value(cvArrayJ);
				}
		}

		// attributes
		json_t *attributesJ = json_object_get(rootJ, "attributes");
		if (attributesJ) {
			for (int i = 0; i < 16; i++)
				for (int s = 0; s < 16; s++) {
					json_t *attributesArrayJ = json_array_get(attributesJ, s + (i * 16));
					if (attributesArrayJ)
						attributes[i][s] = json_integer_value(attributesArrayJ);
				}
		}
		else {// legacy
			for (int i = 0; i < 16; i++)
				for (int s = 0; s < 16; s++)
					attributes[i][s] = 0;
			// gate1
			json_t *gate1J = json_object_get(rootJ, "gate1");
			if (gate1J) {
				for (int i = 0; i < 16; i++)
					for (int s = 0; s < 16; s++) {
						json_t *gate1arrayJ = json_array_get(gate1J, s + (i * 16));
						if (gate1arrayJ)
							if (!!json_integer_value(gate1arrayJ)) attributes[i][s] |= ATT_MSK_GATE1;
					}
			}
			// gate1Prob
			json_t *gate1ProbJ = json_object_get(rootJ, "gate1Prob");
			if (gate1ProbJ) {
				for (int i = 0; i < 16; i++)
					for (int s = 0; s < 16; s++) {
						json_t *gate1ProbarrayJ = json_array_get(gate1ProbJ, s + (i * 16));
						if (gate1ProbarrayJ)
							if (!!json_integer_value(gate1ProbarrayJ)) attributes[i][s] |= ATT_MSK_GATE1P;
					}
			}
			// gate2
			json_t *gate2J = json_object_get(rootJ, "gate2");
			if (gate2J) {
				for (int i = 0; i < 16; i++)
					for (int s = 0; s < 16; s++) {
						json_t *gate2arrayJ = json_array_get(gate2J, s + (i * 16));
						if (gate2arrayJ)
							if (!!json_integer_value(gate2arrayJ)) attributes[i][s] |= ATT_MSK_GATE2;
					}
			}
			// slide
			json_t *slideJ = json_object_get(rootJ, "slide");
			if (slideJ) {
				for (int i = 0; i < 16; i++)
					for (int s = 0; s < 16; s++) {
						json_t *slideArrayJ = json_array_get(slideJ, s + (i * 16));
						if (slideArrayJ)
							if (!!json_integer_value(slideArrayJ)) attributes[i][s] |= ATT_MSK_SLIDE;
					}
			}
			// tied
			json_t *tiedJ = json_object_get(rootJ, "tied");
			if (tiedJ) {
				for (int i = 0; i < 16; i++)
					for (int s = 0; s < 16; s++) {
						json_t *tiedArrayJ = json_array_get(tiedJ, s + (i * 16));
						if (tiedArrayJ)
							if (!!json_integer_value(tiedArrayJ)) attributes[i][s] |= ATT_MSK_TIED;
					}
			}
		}
	
		// attached
		json_t *attachedJ = json_object_get(rootJ, "attached");
		if (attachedJ)
			attached = json_is_true(attachedJ);
		
		// resetOnRun
		json_t *resetOnRunJ = json_object_get(rootJ, "resetOnRun");
		if (resetOnRunJ)
			resetOnRun = json_is_true(resetOnRunJ);

		// Initialize dependants after everything loaded
		initRun(true);
		editingSequence = isEditingSequence();
		editingSequenceLast = editingSequence;
	}


	void rotateSeq(int seqNum, bool directionRight, int seqLength) {
		float rotCV;
		int rotAttributes;
		int iStart = 0;
		int iEnd = seqLength - 1;
		int iRot = iStart;
		int iDelta = 1;
		if (directionRight) {
			iRot = iEnd;
			iDelta = -1;
		}
		rotCV = cv[seqNum][iRot];
		rotAttributes = attributes[seqNum][iRot];
		for ( ; ; iRot += iDelta) {
			if (iDelta == 1 && iRot >= iEnd) break;
			if (iDelta == -1 && iRot <= iStart) break;
			cv[seqNum][iRot] = cv[seqNum][iRot + iDelta];
			attributes[seqNum][iRot] = attributes[seqNum][iRot + iDelta];
		}
		cv[seqNum][iRot] = rotCV;
		attributes[seqNum][iRot] = rotAttributes;
	}
	

	// Advances the module by 1 audio frame with duration 1.0 / engineGetSampleRate()
	void step() override {
		float sampleRate = engineGetSampleRate();
		static const float gateTime = 0.4f;// seconds
		static const float copyPasteInfoTime = 0.5f;// seconds
		static const float editLengthTime = 2.0f;// seconds
		static const float tiedWarningTime = 0.7f;// seconds
		static const float holdDetectTime = 2.0f;// seconds
		static const float editGateLengthTime = 4.0f;// seconds
		
		
		//********** Buttons, knobs, switches and inputs **********
		
		// Notes: 
		// * a tied step's attributes can not be modified by any of the following: 
		//   write input, oct and keyboard buttons, gate1Prob and slide buttons
		//   however, paste, transpose, rotate obviously can, and gate1/2 can be turned back on if desired.
		// * Whenever cv[][] is modified or tied[] is activated for a step, call applyTiedStep(sequence,stepIndexEdit,steps)
		
		// Edit mode
		editingSequence = isEditingSequence();// true = editing sequence, false = editing song
		if (editingSequenceLast != editingSequence) {
			if (running)
				initRun(true);
			displayState = DISP_NORMAL;
			editingSequenceLast = editingSequence;
		}
		
		// Seq CV input
		if (inputs[SEQCV_INPUT].active) {
			sequence = (int) clamp( round(inputs[SEQCV_INPUT].value * (16.0f - 1.0f) / 10.0f), 0.0f, (16.0f - 1.0f) );
		}
		// Mode CV input
		if (inputs[MODECV_INPUT].active) {
			if (editingSequence)
				runModeSeq[sequence] = (int) clamp( round(inputs[MODECV_INPUT].value * ((float)NUM_MODES - 1.0f) / 10.0f), 0.0f, (float)NUM_MODES - 1.0f );
		}
		
		// Run button
		if (runningTrigger.process(params[RUN_PARAM].value + inputs[RUNCV_INPUT].value)) {
			running = !running;
			if (running)
				initRun(resetOnRun);
			displayState = DISP_NORMAL;
		}

		// Attach button
		if (attachedTrigger.process(params[ATTACH_PARAM].value)) {
			if (running)
				attached = !attached;	
			displayState = DISP_NORMAL;			
		}
		if (running && attached) {
			if (editingSequence)
				stepIndexEdit = stepIndexRun;
			else
				phraseIndexEdit = phraseIndexRun;
		}
		
		// Copy button
		if (copyTrigger.process(params[COPY_PARAM].value)) {
			if (editingSequence) {
				infoCopyPaste = (long) (copyPasteInfoTime * sampleRate / displayRefreshStepSkips);
				int sStart = stepIndexEdit;
				int sCount = 16;
				if (params[CPMODE_PARAM].value > 1.5f)// all
					sStart = 0;
				else if (params[CPMODE_PARAM].value < 0.5f)// 4
					sCount = 4;
				else// 8
					sCount = 8;
				countCP = sCount;
				for (int i = 0, s = sStart; i < countCP; i++, s++) {
					if (s >= 16) s = 0;
					cvCPbuffer[i] = cv[sequence][s];
					attributesCPbuffer[i] = attributes[sequence][s];
					if ((--sCount) <= 0)
						break;
				}
				lengthCPbuffer = lengths[sequence];
				modeCPbuffer = runModeSeq[sequence];
			}
			displayState = DISP_NORMAL;
		}
		// Paste button
		if (pasteTrigger.process(params[PASTE_PARAM].value)) {
			if (editingSequence) {
				infoCopyPaste = (long) (-1 * copyPasteInfoTime * sampleRate / displayRefreshStepSkips);
				int sStart = ((countCP == 16) ? 0 : stepIndexEdit);
				int sCount = countCP;
				for (int i = 0, s = sStart; i < countCP; i++, s++) {
					if (s >= 16) 
						break;
					cv[sequence][s] = cvCPbuffer[i];
					attributes[sequence][s] = attributesCPbuffer[i];
					if ((--sCount) <= 0)
						break;
				}
				if (params[CPMODE_PARAM].value > 1.5f) {// all
					lengths[sequence] = lengthCPbuffer;
					runModeSeq[sequence] = modeCPbuffer;
				}
			}
			displayState = DISP_NORMAL;
		}

		// Length button
		if (lengthTrigger.process(params[LENGTH_PARAM].value)) {
			if (editingLength > 0ul)
				editingLength = 0ul;// allow user to quickly leave editing mode when re-press
			else
				editingLength = (unsigned long) (editLengthTime * sampleRate / displayRefreshStepSkips);
			displayState = DISP_NORMAL;
		}
		
		// Write input (must be before Left and Right in case route gate simultaneously to Right and Write for example)
		//  (write must be to correct step)
		bool writeTrig = writeTrigger.process(inputs[WRITE_INPUT].value);
		if (writeTrig) {
			if (editingSequence) {
				cv[sequence][stepIndexEdit] = inputs[CV_INPUT].value;
				// Extra CVs from expansion panel:
				if (inputs[TIEDCV_INPUT].active)
					setTiedA(&attributes[sequence][stepIndexEdit], inputs[TIEDCV_INPUT].value > 1.0f);
				if (inputs[GATE1CV_INPUT].active)
					setGate1a(&attributes[sequence][stepIndexEdit], inputs[GATE1CV_INPUT].value > 1.0f);
				if (inputs[GATE2CV_INPUT].active)
					setGate2a(&attributes[sequence][stepIndexEdit], inputs[GATE2CV_INPUT].value > 1.0f);
				if (inputs[SLIDECV_INPUT].active)
					setSlideA(&attributes[sequence][stepIndexEdit], inputs[SLIDECV_INPUT].value > 1.0f);			
				applyTiedStep(sequence, stepIndexEdit, lengths[sequence]);
				editingGate = (unsigned long) (gateTime * sampleRate / displayRefreshStepSkips);
				editingGateCV = cv[sequence][stepIndexEdit];
				editingGateKeyLight = -1;
				// Autostep (after grab all active inputs)
				if (params[AUTOSTEP_PARAM].value > 0.5f)
					stepIndexEdit = moveIndex(stepIndexEdit, stepIndexEdit + 1, 16);
			}
			displayState = DISP_NORMAL;
		}
		// Left and Right CV inputs and buttons
		int delta = 0;
		if (leftTrigger.process(inputs[LEFTCV_INPUT].value + params[LEFT_PARAM].value)) { 
			delta = -1;
			displayState = DISP_NORMAL;
		}
		if (rightTrigger.process(inputs[RIGHTCV_INPUT].value + params[RIGHT_PARAM].value)) {
			delta = +1;
			displayState = DISP_NORMAL;
		}
		if (delta != 0) {
			if (editingLength > 0ul) {
				editingLength = (unsigned long) (editLengthTime * sampleRate / displayRefreshStepSkips);// restart editing length timer
				if (editingSequence) {
					lengths[sequence] += delta;
					if (lengths[sequence] > 16) lengths[sequence] = 16;
					if (lengths[sequence] < 1 ) lengths[sequence] = 1;
				}
				else {
					phrases += delta;
					if (phrases > 16) phrases = 16;
					if (phrases < 1 ) phrases = 1;
				}
			}
			else {
				if (!running || !attached) {// don't move heads when attach and running
					if (editingSequence) {
						stepIndexEdit = moveIndex(stepIndexEdit, stepIndexEdit + delta, 16);
						if (!getTied(sequence,stepIndexEdit)) {// play if non-tied step
							if (!writeTrig) {// in case autostep when simultaneous writeCV and stepCV (keep what was done in Write Input block above)
								editingGate = (unsigned long) (gateTime * sampleRate / displayRefreshStepSkips);
								editingGateCV = cv[sequence][stepIndexEdit];
								editingGateKeyLight = -1;
							}
						}
					}
					else
						phraseIndexEdit = moveIndex(phraseIndexEdit, phraseIndexEdit + delta, 16);
				}
			}
		}
		
		// Mode and Transpose/Rotate buttons
		if (modeTrigger.process(params[RUNMODE_PARAM].value)) {
			if (displayState != DISP_MODE)
				displayState = DISP_MODE;
			else
				displayState = DISP_NORMAL;
			//if (!running) {
				modeHoldDetect.start((long) (holdDetectTime * sampleRate / displayRefreshStepSkips));
			//}
		}
		if (transposeTrigger.process(params[TRAN_ROT_PARAM].value)) {
			if (editingSequence) {
				if (displayState == DISP_NORMAL || displayState == DISP_MODE) {
					displayState = DISP_TRANSPOSE;
					transposeOffset = 0;
				}
				else if (displayState == DISP_TRANSPOSE) {
					displayState = DISP_ROTATE;
					rotateOffset = 0;
				}
				else 
					displayState = DISP_NORMAL;
			}
		}			
		
		// Sequence knob  
		float seqParamValue = params[SEQUENCE_PARAM].value;
		int newSequenceKnob = (int)roundf(seqParamValue * 7.0f);
		if (seqParamValue == 0.0f)// true when constructor or fromJson() occured
			sequenceKnob = newSequenceKnob;
		int deltaKnob = newSequenceKnob - sequenceKnob;
		if (deltaKnob != 0) {
			if (abs(deltaKnob) <= 3) {// avoid discontinuous step (initialize for example)
				 if (editingLength > 0ul) {
					editingLength = (unsigned long) (editLengthTime * sampleRate / displayRefreshStepSkips);// restart editing length timer
					if (editingSequence) {
						lengths[sequence] += deltaKnob;
						if (lengths[sequence] > 16) lengths[sequence] = 16 ;
						if (lengths[sequence] < 1 ) lengths[sequence] = 1;
					}
					else {
						phrases += deltaKnob;
						if (phrases > 16) phrases = 16;
						if (phrases < 1 ) phrases = 1;
					}
				}
				else if (editingPpqn != 0) {
					pulsesPerStep = indexToPps(ppsToIndex(pulsesPerStep) + deltaKnob);// indexToPps() does clamping
					editingPpqn = (long) (editGateLengthTime * sampleRate / displayRefreshStepSkips);
				}
				else if (displayState == DISP_MODE) {
					if (editingSequence) {
						if (!inputs[MODECV_INPUT].active) {
							runModeSeq[sequence] += deltaKnob;
							if (runModeSeq[sequence] < 0) runModeSeq[sequence] = 0;
							if (runModeSeq[sequence] >= NUM_MODES) runModeSeq[sequence] = NUM_MODES - 1;
						}
					}
					else {
						runModeSong += deltaKnob;
						if (runModeSong < 0) runModeSong = 0;
						if (runModeSong >= 5) runModeSong = 5 - 1;
					}
				}
				else if (displayState == DISP_TRANSPOSE) {
					if (editingSequence) {
						transposeOffset += deltaKnob;
						if (transposeOffset > 99) transposeOffset = 99;
						if (transposeOffset < -99) transposeOffset = -99;						
						// Tranpose by this number of semi-tones: deltaKnob
						float transposeOffsetCV = ((float)(deltaKnob))/12.0f;
						for (int s = 0; s < 16; s++) {
							cv[sequence][s] += transposeOffsetCV;
						}
					}
				}
				else if (displayState == DISP_ROTATE) {
					if (editingSequence) {
						rotateOffset += deltaKnob;
						if (rotateOffset > 99) rotateOffset = 99;
						if (rotateOffset < -99) rotateOffset = -99;	
						if (deltaKnob > 0 && deltaKnob < 99) {// Rotate right, 99 is safety
							for (int i = deltaKnob; i > 0; i--)
								rotateSeq(sequence, true, lengths[sequence]);
						}
						if (deltaKnob < 0 && deltaKnob > -99) {// Rotate left, 99 is safety
							for (int i = deltaKnob; i < 0; i++)
								rotateSeq(sequence, false, lengths[sequence]);
						}
					}						
				}
				else {// DISP_NORMAL
					if (editingSequence) {
						if (!inputs[SEQCV_INPUT].active) {
							sequence += deltaKnob;
							if (sequence < 0) sequence = 0;
							if (sequence >= 16) sequence = (16 - 1);
						}
					}
					else {
						phrase[phraseIndexEdit] += deltaKnob;
						if (phrase[phraseIndexEdit] < 0) phrase[phraseIndexEdit] = 0;
						if (phrase[phraseIndexEdit] >= 16) phrase[phraseIndexEdit] = (16 - 1);				
					}
				}
			}
			sequenceKnob = newSequenceKnob;
		}	
		
		// Octave buttons
		int newOct = -1;
		for (int i = 0; i < 7; i++) {
			if (octTriggers[i].process(params[OCTAVE_PARAM + i].value)) {
				newOct = 6 - i;
				displayState = DISP_NORMAL;
			}
		}
		if (newOct >= 0 && newOct <= 6) {
			if (editingSequence) {
				if (getTied(sequence,stepIndexEdit))
					tiedWarning = (long) (tiedWarningTime * sampleRate / displayRefreshStepSkips);
				else {			
					float newCV = cv[sequence][stepIndexEdit] + 10.0f;//to properly handle negative note voltages
					newCV = newCV - floor(newCV) + (float) (newOct - 3);
					if (newCV >= -3.0f && newCV < 4.0f) {
						cv[sequence][stepIndexEdit] = newCV;
						applyTiedStep(sequence, stepIndexEdit, lengths[sequence]);
					}
					editingGate = (unsigned long) (gateTime * sampleRate / displayRefreshStepSkips);
					editingGateCV = cv[sequence][stepIndexEdit];
					editingGateKeyLight = -1;
				}
			}
		}		
		
		// Keyboard buttons
		for (int i = 0; i < 12; i++) {
			if (keyTriggers[i].process(params[KEY_PARAMS + i].value)) {
				if (editingSequence) {
					if (editingGateLength != 0l) {
						int newMode = keyIndexToGateMode(i, pulsesPerStep);
						if (editingGateLength > 0l) {
							if (newMode != -1)
								setGate1Mode(sequence, stepIndexEdit, newMode);
							else
								editingPpqn = (long) (editGateLengthTime * sampleRate / displayRefreshStepSkips);
							editingGateLength = ((long) (editGateLengthTime * sampleRate / displayRefreshStepSkips) * editGateLengthTimeInitMult);
						}
						else {
							if (newMode != -1)
								setGate2Mode(sequence, stepIndexEdit, newMode);
							else
								editingPpqn = (long) (editGateLengthTime * sampleRate / displayRefreshStepSkips);
							editingGateLength = -1l * ((long) (editGateLengthTime * sampleRate / displayRefreshStepSkips) * editGateLengthTimeInitMult);
						}
					}
					else if (getTied(sequence,stepIndexEdit)) {
						if (params[KEY_PARAMS + i].value > 1.5f)
							stepIndexEdit = moveIndex(stepIndexEdit, stepIndexEdit + 1, 16);
						else
							tiedWarning = (long) (tiedWarningTime * sampleRate / displayRefreshStepSkips);
					}
					else {			
						cv[sequence][stepIndexEdit] = floor(cv[sequence][stepIndexEdit]) + ((float) i) / 12.0f;
						applyTiedStep(sequence, stepIndexEdit, lengths[sequence]);
						editingGate = (unsigned long) (gateTime * sampleRate / displayRefreshStepSkips);
						editingGateCV = cv[sequence][stepIndexEdit];
						editingGateKeyLight = -1;
						if (params[KEY_PARAMS + i].value > 1.5f) {
							stepIndexEdit = moveIndex(stepIndexEdit, stepIndexEdit + 1, 16);
							editingGateKeyLight = i;
						}
					}						
				}
				displayState = DISP_NORMAL;
			}
		}
				
		// Gate1, Gate1Prob, Gate2, Slide and Tied buttons
		if (gate1Trigger.process(params[GATE1_PARAM].value)) {
			if (editingSequence) {
				toggleGate1a(&attributes[sequence][stepIndexEdit]);
				//if (!running) {
					if (pulsesPerStep != 1) {
						editingGateLength = getGate1(sequence,stepIndexEdit) ? ((long) (editGateLengthTime * sampleRate / displayRefreshStepSkips) * editGateLengthTimeInitMult) : 0l;
						gate1HoldDetect.start((long) (holdDetectTime * sampleRate / displayRefreshStepSkips));
					}
				//}
			}
			displayState = DISP_NORMAL;
		}		
		if (gate1ProbTrigger.process(params[GATE1_PROB_PARAM].value)) {
			if (editingSequence) {
				if (getTied(sequence,stepIndexEdit))
					tiedWarning = (long) (tiedWarningTime * sampleRate / displayRefreshStepSkips);
				else
					toggleGate1Pa(&attributes[sequence][stepIndexEdit]);
			}
			displayState = DISP_NORMAL;
		}		
		if (gate2Trigger.process(params[GATE2_PARAM].value)) {
			if (editingSequence) {
				toggleGate2a(&attributes[sequence][stepIndexEdit]);
				//if (!running) {
					if (pulsesPerStep != 1) {
						editingGateLength = getGate2(sequence,stepIndexEdit) ? -1l * ((long) (editGateLengthTime * sampleRate / displayRefreshStepSkips) * editGateLengthTimeInitMult) : 0l;
						gate2HoldDetect.start((long) (holdDetectTime * sampleRate / displayRefreshStepSkips));
					}
				//}
			}
			displayState = DISP_NORMAL;
		}		
		if (slideTrigger.process(params[SLIDE_BTN_PARAM].value)) {
			if (editingSequence) {
				if (getTied(sequence,stepIndexEdit))
					tiedWarning = (long) (tiedWarningTime * sampleRate / displayRefreshStepSkips);
				else
					toggleSlideA(&attributes[sequence][stepIndexEdit]);
			}
			displayState = DISP_NORMAL;
		}		
		if (tiedTrigger.process(params[TIE_PARAM].value)) {
			if (editingSequence) {
				toggleTiedA(&attributes[sequence][stepIndexEdit]);
				if (getTied(sequence,stepIndexEdit)) {
					setGate1a(&attributes[sequence][stepIndexEdit], false);
					setGate2a(&attributes[sequence][stepIndexEdit], false);
					setSlideA(&attributes[sequence][stepIndexEdit], false);
					applyTiedStep(sequence, stepIndexEdit, lengths[sequence]);
				}
			}
			displayState = DISP_NORMAL;
		}		
		
		
		//********** Clock and reset **********
		
		// Clock
		if (clockTrigger.process(inputs[CLOCK_INPUT].value)) {
			if (running && clockIgnoreOnReset == 0l) {
				ppqnCount++;
				if (ppqnCount >= pulsesPerStep)
					ppqnCount = 0;

				int newSeq = sequence;// good value when editingSequence, overwrite if not editingSequence
				if (ppqnCount == 0) {
					float slideFromCV = 0.0f;
					if (editingSequence) {
						slideFromCV = cv[sequence][stepIndexRun];
						moveIndexRunMode(&stepIndexRun, lengths[sequence], runModeSeq[sequence], &stepIndexRunHistory);
					}
					else {
						slideFromCV = cv[phrase[phraseIndexRun]][stepIndexRun];
						if (moveIndexRunMode(&stepIndexRun, lengths[phrase[phraseIndexRun]], runModeSeq[phrase[phraseIndexRun]], &stepIndexRunHistory)) {
							moveIndexRunMode(&phraseIndexRun, phrases, runModeSong, &phraseIndexRunHistory);
							stepIndexRun = (runModeSeq[phrase[phraseIndexRun]] == MODE_REV ? lengths[phrase[phraseIndexRun]] - 1 : 0);// must always refresh after phraseIndexRun has changed
						}
						newSeq = phrase[phraseIndexRun];
					}
					
					// Slide
					if (getSlide(newSeq, stepIndexRun)) {
						// activate sliding (slideStepsRemain can be reset, else runs down to 0, either way, no need to reinit)
						slideStepsRemain =   (unsigned long) (((float)clockPeriod * pulsesPerStep)   * params[SLIDE_KNOB_PARAM].value / 2.0f);// 0-T slide, where T is clock period (can be too long when user does clock gating)
						//slideStepsRemain = (unsigned long)  (engineGetSampleRate() * params[SLIDE_KNOB_PARAM].value );// 0-2s slide
						float slideToCV = cv[newSeq][stepIndexRun];
						slideCVdelta = (slideToCV - slideFromCV)/(float)slideStepsRemain;
					}
				}
				else {
					if (!editingSequence)
						newSeq = phrase[phraseIndexRun];
				}
				if (gate1Code != -1 || ppqnCount == 0)
					gate1Code = calcGate1Code(attributes[newSeq][stepIndexRun], ppqnCount, pulsesPerStep, params[GATE1_KNOB_PARAM].value);
				gate2Code = calcGate2Code(attributes[newSeq][stepIndexRun], ppqnCount, pulsesPerStep);						 
			}
			clockPeriod = 0ul;
		}	
		clockPeriod++;
		
		// Reset
		if (resetTrigger.process(inputs[RESET_INPUT].value + params[RESET_PARAM].value)) {
			//stepIndexEdit = 0;
			//sequence = 0;
			initRun(true);// must be after sequence reset
			resetLight = 1.0f;
			displayState = DISP_NORMAL;
			clockTrigger.reset();
		}
		
		
		//********** Outputs and lights **********
				
		// CV and gates outputs
		int seq = editingSequence ? (sequence) : (running ? phrase[phraseIndexRun] : phrase[phraseIndexEdit]);
		int step = editingSequence ? (running ? stepIndexRun : stepIndexEdit) : (stepIndexRun);
		if (running) {
			float slideOffset = (slideStepsRemain > 0ul ? (slideCVdelta * (float)slideStepsRemain) : 0.0f);
			outputs[CV_OUTPUT].value = cv[seq][step] - slideOffset;
			outputs[GATE1_OUTPUT].value = calcGate(gate1Code, clockTrigger, clockPeriod, sampleRate) ? 10.0f : 0.0f;
			outputs[GATE2_OUTPUT].value = calcGate(gate2Code, clockTrigger, clockPeriod, sampleRate) ? 10.0f : 0.0f;
		}
		else {// not running
			outputs[CV_OUTPUT].value = (editingGate > 0ul) ? editingGateCV : cv[seq][step];
			outputs[GATE1_OUTPUT].value = (editingGate > 0ul) ? 10.0f : 0.0f;
			outputs[GATE2_OUTPUT].value = (editingGate > 0ul) ? 10.0f : 0.0f;
		}
		if (slideStepsRemain > 0ul)
			slideStepsRemain--;
		
		lightRefreshCounter++;
		if (lightRefreshCounter > displayRefreshStepSkips) {
			lightRefreshCounter = 0;

			// Step/phrase lights
			if (infoCopyPaste != 0l) {
				for (int i = 0; i < 16; i++) {
					if ( (i >= stepIndexEdit && i < (stepIndexEdit + countCP)) || (countCP == 16) )
						lights[STEP_PHRASE_LIGHTS + (i<<1)].value = 0.5f;// Green when copy interval
					else
						lights[STEP_PHRASE_LIGHTS + (i<<1)].value = 0.0f; // Green (nothing)
					lights[STEP_PHRASE_LIGHTS + (i<<1) + 1].value = 0.0f;// Red (nothing)
				}
			}
			else {
				for (int i = 0; i < 16; i++) {
					if (editingLength > 0ul) {
						// Length (green)
						if (editingSequence)
							lights[STEP_PHRASE_LIGHTS + (i<<1)].value = ((i < lengths[sequence]) ? 0.5f : 0.0f);
						else
							lights[STEP_PHRASE_LIGHTS + (i<<1)].value = ((i < phrases) ? 0.5f : 0.0f);
						// Nothing (red)
						lights[STEP_PHRASE_LIGHTS + (i<<1) + 1].value = 0.0f;
					}
					else {
						// Run cursor (green)
						if (editingSequence)
							lights[STEP_PHRASE_LIGHTS + (i<<1)].value = ((running && (i == stepIndexRun)) ? 1.0f : 0.0f);
						else {
							float green = ((running && (i == phraseIndexRun)) ? 1.0f : 0.0f);
							green += ((running && (i == stepIndexRun) && i != phraseIndexEdit) ? 0.1f : 0.0f);
							lights[STEP_PHRASE_LIGHTS + (i<<1)].value = clamp(green, 0.0f, 1.0f);
						}
						// Edit cursor (red)
						if (editingSequence)
							lights[STEP_PHRASE_LIGHTS + (i<<1) + 1].value = (i == stepIndexEdit ? 1.0f : 0.0f);
						else
							lights[STEP_PHRASE_LIGHTS + (i<<1) + 1].value = (i == phraseIndexEdit ? 1.0f : 0.0f);
					}
				}
			}
		
			// Octave lights
			float octCV = 0.0f;
			if (editingSequence)
				octCV = cv[sequence][stepIndexEdit];
			else
				octCV = cv[phrase[phraseIndexEdit]][stepIndexRun];
			int octLightIndex = (int) floor(octCV + 3.0f);
			for (int i = 0; i < 7; i++) {
				if (!editingSequence && (!attached || !running))// no oct lights when song mode and either (detached [1] or stopped [2])
												// [1] makes no sense, can't mod steps and stepping though seq that may not be playing
												// [2] CV is set to 0V when not running and in song mode, so cv[][] makes no sense to display
					lights[OCTAVE_LIGHTS + i].value = 0.0f;
				else {
					if (tiedWarning > 0l) {
						bool warningFlashState = calcWarningFlash(tiedWarning, (long) (tiedWarningTime * sampleRate / displayRefreshStepSkips));
						lights[OCTAVE_LIGHTS + i].value = (warningFlashState && (i == (6 - octLightIndex))) ? 1.0f : 0.0f;
					}
					else				
						lights[OCTAVE_LIGHTS + i].value = (i == (6 - octLightIndex) ? 1.0f : 0.0f);
				}
			}
			
			// Keyboard lights
			float cvValOffset;
			if (editingSequence) 
				cvValOffset = cv[sequence][stepIndexEdit] + 10.0f;//to properly handle negative note voltages
			else	
				cvValOffset = cv[phrase[phraseIndexEdit]][stepIndexRun] + 10.0f;//to properly handle negative note voltages
			int keyLightIndex = (int) clamp(  roundf( (cvValOffset-floor(cvValOffset)) * 12.0f ),  0.0f,  11.0f);
			if (editingGateLength != 0 && editingSequence) {
				int modeLightIndex = gateModeToKeyLightIndex(attributes[sequence][stepIndexEdit], editingGateLength > 0l);
				for (int i = 0; i < 12; i++) {
					if (i == modeLightIndex) {
						lights[KEY_LIGHTS + i * 2 + 0].value = editingGateLength > 0l ? 1.0f : 0.2f;
						lights[KEY_LIGHTS + i * 2 + 1].value = editingGateLength > 0l ? 0.2f : 1.0f;
					}
					else { 
						lights[KEY_LIGHTS + i * 2 + 0].value = 0.0f;
						if (i == keyLightIndex) 
							lights[KEY_LIGHTS + i * 2 + 1].value = 0.1f;	
						else 
							lights[KEY_LIGHTS + i * 2 + 1].value = 0.0f;
					}
				}
			}
			else {
				for (int i = 0; i < 12; i++) {
					lights[KEY_LIGHTS + i * 2 + 0].value = 0.0f;
					if (!editingSequence && (!attached || !running))// no keyboard lights when song mode and either (detached [1] or stopped [2])
													// [1] makes no sense, can't mod steps and stepping though seq that may not be playing
													// [2] CV is set to 0V when not running and in song mode, so cv[][] makes no sense to display
						lights[KEY_LIGHTS + i * 2 + 1].value = 0.0f;
					else {
						if (tiedWarning > 0l) {
							bool warningFlashState = calcWarningFlash(tiedWarning, (long) (tiedWarningTime * sampleRate / displayRefreshStepSkips));
							lights[KEY_LIGHTS + i * 2 + 1].value = (warningFlashState && i == keyLightIndex) ? 1.0f : 0.0f;
						}
						else {
							if (editingGate > 0ul && editingGateKeyLight != -1)
								lights[KEY_LIGHTS + i * 2 + 1].value = (i == editingGateKeyLight ? ((float) editingGate / (float)(gateTime * sampleRate / displayRefreshStepSkips)) : 0.0f);
							else
								lights[KEY_LIGHTS + i * 2 + 1].value = (i == keyLightIndex ? 1.0f : 0.0f);
						}
					}
				}	
			}			
			
			// Gate1, Gate1Prob, Gate2, Slide and Tied lights
			int attributesVal = attributes[sequence][stepIndexEdit];
			if (!editingSequence)
				attributesVal = attributes[phrase[phraseIndexEdit]][stepIndexRun];
			//
			setGateLight(getGate1a(attributesVal), GATE1_LIGHT);
			setGateLight(getGate2a(attributesVal), GATE2_LIGHT);
			lights[GATE1_PROB_LIGHT].value = getGate1Pa(attributesVal) ? 1.0f : 0.0f;
			lights[SLIDE_LIGHT].value = getSlideA(attributesVal) ? 1.0f : 0.0f;
			if (tiedWarning > 0l) {
				bool warningFlashState = calcWarningFlash(tiedWarning, (long) (tiedWarningTime * sampleRate / displayRefreshStepSkips));
				lights[TIE_LIGHT].value = (warningFlashState) ? 1.0f : 0.0f;
			}
			else
				lights[TIE_LIGHT].value = getTiedA(attributesVal) ? 1.0f : 0.0f;

			// Attach light
			lights[ATTACH_LIGHT].value = (running && attached) ? 1.0f : 0.0f;
			
			// Reset light
			lights[RESET_LIGHT].value =	resetLight;	
			resetLight -= (resetLight / lightLambda) * engineGetSampleTime() * displayRefreshStepSkips;
			
			// Run light
			lights[RUN_LIGHT].value = running ? 1.0f : 0.0f;
			
			if (tiedWarning > 0l)
				tiedWarning--;
			if (editingLength > 0ul)
				editingLength--;
			if (editingGate > 0ul)
				editingGate--;
			if (infoCopyPaste != 0l) {
				if (infoCopyPaste > 0l)
					infoCopyPaste --;
				if (infoCopyPaste < 0l)
					infoCopyPaste ++;
			}
			if (modeHoldDetect.process(params[RUNMODE_PARAM].value)) {
				displayState = DISP_NORMAL;
				editingPpqn = (long) (editGateLengthTime * sampleRate / displayRefreshStepSkips);
			}
			if (gate1HoldDetect.process(params[GATE1_PARAM].value)) {
				toggleGate1a(&attributes[sequence][stepIndexEdit]);
				editGateLengthTimeInitMult = 1;
			}
			if (gate2HoldDetect.process(params[GATE2_PARAM].value)) {
				toggleGate2a(&attributes[sequence][stepIndexEdit]);
				editGateLengthTimeInitMult = 100;
			}
			if (editingPpqn > 0l)
				editingPpqn--;
			if (editingGateLength != 0l) {
				if (editingGateLength > 0l)
					editingGateLength --;
				if (editingGateLength < 0l)
					editingGateLength ++;
			}
		}// lightRefreshCounter
		
		if (clockIgnoreOnReset > 0l)
			clockIgnoreOnReset--;

	}// step()
	
	void applyTiedStep(int seqNum, int indexTied, int seqLength) {
		// Start on indexTied and loop until seqLength
		// Called because either:
		//   case A: tied was activated for given step
		//   case B: the given step's CV was modified
		// These cases are mutually exclusive
		
		// copy previous CV over to current step if tied
		if (getTied(seqNum,indexTied) && (indexTied > 0))
			cv[seqNum][indexTied] = cv[seqNum][indexTied - 1];
		
		// Affect downstream CVs of subsequent tied note chain (can be 0 length if next note is not tied)
		for (int i = indexTied + 1; i < seqLength && getTied(seqNum,i); i++) 
			cv[seqNum][i] = cv[seqNum][indexTied];
	}
	
	int calcNewGateMode(int currentGateMode, int deltaKnob) {
		return clamp(currentGateMode + deltaKnob, 0, NUM_GATES - 1); 
	}
	
	inline void setGateLight(bool gateOn, int lightIndex) {
		if (!gateOn) {
			lights[lightIndex + 0].value = 0.0f;
			lights[lightIndex + 1].value = 0.0f;
		}
		else if (pulsesPerStep == 1) {
			lights[lightIndex + 0].value = 0.0f;
			lights[lightIndex + 1].value = 1.0f;
		}
		else {
			lights[lightIndex + 0].value = lightIndex == GATE1_LIGHT ? 1.0f : 0.2f;
			lights[lightIndex + 1].value = lightIndex == GATE1_LIGHT ? 0.2f : 1.0f;
		}
	}
	
};



struct PhraseSeq16Widget : ModuleWidget {
	PhraseSeq16 *module;
	DynamicSVGPanel *panel;
	int oldExpansion;
	int expWidth = 60;
	IMPort* expPorts[5];

	struct SequenceDisplayWidget : TransparentWidget {
		PhraseSeq16 *module;
		std::shared_ptr<Font> font;
		char displayStr[4];
		
		SequenceDisplayWidget() {
			font = Font::load(assetPlugin(plugin, "res/fonts/Segment14.ttf"));
		}
		
		void runModeToStr(int num) {
			if (num >= 0 && num < NUM_MODES)
				snprintf(displayStr, 4, "%s", modeLabels[num].c_str());
		}

		void draw(NVGcontext *vg) override {
			NVGcolor textColor = prepareDisplay(vg, &box);
			nvgFontFaceId(vg, font->handle);
			//nvgTextLetterSpacing(vg, 2.5);

			Vec textPos = Vec(6, 24);
			nvgFillColor(vg, nvgTransRGBA(textColor, 16));
			nvgText(vg, textPos.x, textPos.y, "~~~", NULL);
			nvgFillColor(vg, textColor);
			if (module->infoCopyPaste != 0l) {
				if (module->infoCopyPaste > 0l)
					snprintf(displayStr, 4, "CPY");
				else
					snprintf(displayStr, 4, "PST");
			}
			else if (module->editingLength > 0ul) {
				if (module->editingSequence)
					snprintf(displayStr, 4, "L%2u", (unsigned) module->lengths[module->sequence]);
				else
					snprintf(displayStr, 4, "L%2u", (unsigned) module->phrases);
			}
			else if (module->editingPpqn != 0ul) {
				snprintf(displayStr, 4, "x%2u", (unsigned) module->pulsesPerStep);
			}
			else if (module->displayState == PhraseSeq16::DISP_MODE) {
				if (module->editingSequence)
					runModeToStr(module->runModeSeq[module->sequence]);
				else
					runModeToStr(module->runModeSong);
			}
			else if (module->displayState == PhraseSeq16::DISP_TRANSPOSE) {
				snprintf(displayStr, 4, "+%2u", (unsigned) abs(module->transposeOffset));
				if (module->transposeOffset < 0)
					displayStr[0] = '-';
			}
			else if (module->displayState == PhraseSeq16::DISP_ROTATE) {
				snprintf(displayStr, 4, ")%2u", (unsigned) abs(module->rotateOffset));
				if (module->rotateOffset < 0)
					displayStr[0] = '(';
			}
			else {// DISP_NORMAL
				snprintf(displayStr, 4, " %2u", (unsigned) (module->editingSequence ? 
					module->sequence : module->phrase[module->phraseIndexEdit]) + 1 );
			}
			nvgText(vg, textPos.x, textPos.y, displayStr, NULL);
		}
	};		
	
	struct PanelThemeItem : MenuItem {
		PhraseSeq16 *module;
		int theme;
		void onAction(EventAction &e) override {
			module->panelTheme = theme;
		}
		void step() override {
			rightText = (module->panelTheme == theme) ? "✔" : "";
		}
	};
	struct ExpansionItem : MenuItem {
		PhraseSeq16 *module;
		void onAction(EventAction &e) override {
			module->expansion = module->expansion == 1 ? 0 : 1;
		}
	};
	struct ResetOnRunItem : MenuItem {
		PhraseSeq16 *module;
		void onAction(EventAction &e) override {
			module->resetOnRun = !module->resetOnRun;
		}
	};
	Menu *createContextMenu() override {
		Menu *menu = ModuleWidget::createContextMenu();

		MenuLabel *spacerLabel = new MenuLabel();
		menu->addChild(spacerLabel);

		PhraseSeq16 *module = dynamic_cast<PhraseSeq16*>(this->module);
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
		
		menu->addChild(new MenuLabel());// empty line
		
		MenuLabel *expansionLabel = new MenuLabel();
		expansionLabel->text = "Expansion module";
		menu->addChild(expansionLabel);

		ExpansionItem *expItem = MenuItem::create<ExpansionItem>(expansionMenuLabel, CHECKMARK(module->expansion != 0));
		expItem->module = module;
		menu->addChild(expItem);
		
		return menu;
	}	
	
	void step() override {
		if(module->expansion != oldExpansion) {
			if (oldExpansion!= -1 && module->expansion == 0) {// if just removed expansion panel, disconnect wires to those jacks
				for (int i = 0; i < 5; i++)
               rack::global_ui->app.gRackWidget->wireContainer->removeAllWires(expPorts[i]);
			}
			oldExpansion = module->expansion;		
		}
		box.size.x = panel->box.size.x - (1 - module->expansion) * expWidth;
		Widget::step();
	}
	
	PhraseSeq16Widget(PhraseSeq16 *module) : ModuleWidget(module) {
		this->module = module;
		oldExpansion = -1;
		
		// Main panel from Inkscape
      panel = new DynamicSVGPanel();
      panel->mode = &module->panelTheme;
		panel->expWidth = &expWidth;
      panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/PhraseSeq16.svg")));
      panel->addPanel(SVG::load(assetPlugin(plugin, "res/dark/PhraseSeq16_dark.svg")));
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

		
		
		// ****** Top row ******
		
		static const int rowRulerT0 = 48;
		static const int columnRulerT0 = 15;// Length button
		static const int columnRulerT1 = columnRulerT0 + 47;// Left/Right buttons
		static const int columnRulerT2 = columnRulerT1 + 75;// Step/Phase lights
		static const int columnRulerT3 = columnRulerT2 + 263;// Attach (also used to align rest of right side of module)

		// Length button
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerT0 + offsetCKD6b, rowRulerT0 + offsetCKD6b), module, PhraseSeq16::LENGTH_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Left/Right buttons
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerT1 + offsetCKD6b, rowRulerT0 + offsetCKD6b), module, PhraseSeq16::LEFT_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerT1 + 38 + offsetCKD6b, rowRulerT0 + offsetCKD6b), module, PhraseSeq16::RIGHT_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Step/Phrase lights
		static const int spLightsSpacing = 15;
		for (int i = 0; i < 16; i++) {
			addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(columnRulerT2 + spLightsSpacing * i + offsetMediumLight, rowRulerT0 + offsetMediumLight), module, PhraseSeq16::STEP_PHRASE_LIGHTS + (i*2)));
		}
		// Attach button and light
		addParam(ParamWidget::create<TL1105>(Vec(columnRulerT3 - 4, rowRulerT0 + 2 + offsetTL1105), module, PhraseSeq16::ATTACH_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(columnRulerT3 + 12 + offsetMediumLight, rowRulerT0 + offsetMediumLight), module, PhraseSeq16::ATTACH_LIGHT));		

		
		
		// ****** Octave and keyboard area ******
		
		// Octave LED buttons
		static const float octLightsIntY = 20.0f;
		for (int i = 0; i < 7; i++) {
			addParam(ParamWidget::create<LEDButton>(Vec(15 + 3, 82 + 24 + i * octLightsIntY- 4.4f), module, PhraseSeq16::OCTAVE_PARAM + i, 0.0f, 1.0f, 0.0f));
			addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(15 + 3 + 4.4f, 82 + 24 + i * octLightsIntY), module, PhraseSeq16::OCTAVE_LIGHTS + i));
		}
		// Keys and Key lights
		static const int keyNudgeX = 7;
		static const int KeyBlackY = 103;
		static const int KeyWhiteY = 141;
		static const int offsetKeyLEDx = 6;
		static const int offsetKeyLEDy = 16;
		// Black keys and lights
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(65+keyNudgeX, KeyBlackY), module, PhraseSeq16::KEY_PARAMS + 1, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(65+keyNudgeX+offsetKeyLEDx, KeyBlackY+offsetKeyLEDy), module, PhraseSeq16::KEY_LIGHTS + 1 * 2));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(93+keyNudgeX, KeyBlackY), module, PhraseSeq16::KEY_PARAMS + 3, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(93+keyNudgeX+offsetKeyLEDx, KeyBlackY+offsetKeyLEDy), module, PhraseSeq16::KEY_LIGHTS + 3 * 2));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(150+keyNudgeX, KeyBlackY), module, PhraseSeq16::KEY_PARAMS + 6, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(150+keyNudgeX+offsetKeyLEDx, KeyBlackY+offsetKeyLEDy), module, PhraseSeq16::KEY_LIGHTS + 6 * 2));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(178+keyNudgeX, KeyBlackY), module, PhraseSeq16::KEY_PARAMS + 8, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(178+keyNudgeX+offsetKeyLEDx, KeyBlackY+offsetKeyLEDy), module, PhraseSeq16::KEY_LIGHTS + 8 * 2));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(206+keyNudgeX, KeyBlackY), module, PhraseSeq16::KEY_PARAMS + 10, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(206+keyNudgeX+offsetKeyLEDx, KeyBlackY+offsetKeyLEDy), module, PhraseSeq16::KEY_LIGHTS + 10 * 2));
		// White keys and lights
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(51+keyNudgeX, KeyWhiteY), module, PhraseSeq16::KEY_PARAMS + 0, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(51+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, PhraseSeq16::KEY_LIGHTS + 0 * 2));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(79+keyNudgeX, KeyWhiteY), module, PhraseSeq16::KEY_PARAMS + 2, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(79+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, PhraseSeq16::KEY_LIGHTS + 2 * 2));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(107+keyNudgeX, KeyWhiteY), module, PhraseSeq16::KEY_PARAMS + 4, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(107+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, PhraseSeq16::KEY_LIGHTS + 4 * 2));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(136+keyNudgeX, KeyWhiteY), module, PhraseSeq16::KEY_PARAMS + 5, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(136+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, PhraseSeq16::KEY_LIGHTS + 5 * 2));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(164+keyNudgeX, KeyWhiteY), module, PhraseSeq16::KEY_PARAMS + 7, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(164+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, PhraseSeq16::KEY_LIGHTS + 7 * 2));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(192+keyNudgeX, KeyWhiteY), module, PhraseSeq16::KEY_PARAMS + 9, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(192+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, PhraseSeq16::KEY_LIGHTS + 9 * 2));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(220+keyNudgeX, KeyWhiteY), module, PhraseSeq16::KEY_PARAMS + 11, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(220+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, PhraseSeq16::KEY_LIGHTS + 11 * 2));
		
		
		
		// ****** Right side control area ******

		static const int rowRulerMK0 = 101;// Edit mode row
		static const int rowRulerMK1 = rowRulerMK0 + 56; // Run row
		static const int rowRulerMK2 = rowRulerMK1 + 54; // Reset row
		static const int columnRulerMK0 = 277;// Edit mode column
		static const int columnRulerMK1 = columnRulerMK0 + 59;// Display column
		static const int columnRulerMK2 = columnRulerT3;// Run mode column
		
		// Edit mode switch
		addParam(ParamWidget::create<CKSS>(Vec(columnRulerMK0 + hOffsetCKSS, rowRulerMK0 + vOffsetCKSS), module, PhraseSeq16::EDIT_PARAM, 0.0f, 1.0f, PhraseSeq16::EDIT_PARAM_INIT_VALUE));
		// Sequence display
		SequenceDisplayWidget *displaySequence = new SequenceDisplayWidget();
		displaySequence->box.pos = Vec(columnRulerMK1-15, rowRulerMK0 + 3 + vOffsetDisplay);
		displaySequence->box.size = Vec(55, 30);// 3 characters
		displaySequence->module = module;
		addChild(displaySequence);
		// Run mode button
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerMK2 + offsetCKD6b, rowRulerMK0 + 0 + offsetCKD6b), module, PhraseSeq16::RUNMODE_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));

		// Run LED bezel and light
		addParam(ParamWidget::create<LEDBezel>(Vec(columnRulerMK0 + offsetLEDbezel, rowRulerMK1 + 7 + offsetLEDbezel), module, PhraseSeq16::RUN_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(Vec(columnRulerMK0 + offsetLEDbezel + offsetLEDbezelLight, rowRulerMK1 + 7 + offsetLEDbezel + offsetLEDbezelLight), module, PhraseSeq16::RUN_LIGHT));
		// Sequence knob
		addParam(createDynamicParam<IMBigKnobInf>(Vec(columnRulerMK1 + 1 + offsetIMBigKnob, rowRulerMK0 + 55 + offsetIMBigKnob), module, PhraseSeq16::SEQUENCE_PARAM, -INFINITY, INFINITY, 0.0f, &module->panelTheme));		
		// Transpose/rotate button
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerMK2 + offsetCKD6b, rowRulerMK1 + 4 + offsetCKD6b), module, PhraseSeq16::TRAN_ROT_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		
		// Reset LED bezel and light
		addParam(ParamWidget::create<LEDBezel>(Vec(columnRulerMK0 + offsetLEDbezel, rowRulerMK2 + 5 + offsetLEDbezel), module, PhraseSeq16::RESET_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(Vec(columnRulerMK0 + offsetLEDbezel + offsetLEDbezelLight, rowRulerMK2 + 5 + offsetLEDbezel + offsetLEDbezelLight), module, PhraseSeq16::RESET_LIGHT));
		// Copy/paste buttons
		addParam(ParamWidget::create<TL1105>(Vec(columnRulerMK1 - 10, rowRulerMK2 + 5 + offsetTL1105), module, PhraseSeq16::COPY_PARAM, 0.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<TL1105>(Vec(columnRulerMK1 + 20, rowRulerMK2 + 5 + offsetTL1105), module, PhraseSeq16::PASTE_PARAM, 0.0f, 1.0f, 0.0f));
		// Copy-paste mode switch (3 position)
		addParam(ParamWidget::create<CKSSThreeInv>(Vec(columnRulerMK2 + hOffsetCKSS + 1, rowRulerMK2 - 3 + vOffsetCKSSThree), module, PhraseSeq16::CPMODE_PARAM, 0.0f, 2.0f, 2.0f));	// 0.0f is top position

		
		
		// ****** Gate and slide section ******
		
		static const int rowRulerMB0 = 214;
		static const int columnRulerMBspacing = 70;
		static const int columnRulerMB2 = 130;// Gate2
		static const int columnRulerMB1 = columnRulerMB2 - columnRulerMBspacing;// Gate1 
		static const int columnRulerMB3 = columnRulerMB2 + columnRulerMBspacing;// Tie
		static const int posLEDvsButton = + 25;
		
		// Gate 1 light and button
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(columnRulerMB1 + posLEDvsButton + offsetMediumLight, rowRulerMB0 + 4 + offsetMediumLight), module, PhraseSeq16::GATE1_LIGHT));		
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerMB1 + offsetCKD6b, rowRulerMB0 + 4 + offsetCKD6b), module, PhraseSeq16::GATE1_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Gate 2 light and button
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(columnRulerMB2 + posLEDvsButton + offsetMediumLight, rowRulerMB0 + 4 + offsetMediumLight), module, PhraseSeq16::GATE2_LIGHT));		
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerMB2 + offsetCKD6b, rowRulerMB0 + 4 + offsetCKD6b), module, PhraseSeq16::GATE2_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Tie light and button
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(columnRulerMB3 + posLEDvsButton + offsetMediumLight, rowRulerMB0 + 4 + offsetMediumLight), module, PhraseSeq16::TIE_LIGHT));		
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerMB3 + offsetCKD6b, rowRulerMB0 + 4 + offsetCKD6b), module, PhraseSeq16::TIE_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));

						
		
		// ****** Bottom two rows ******
		
		static const int outputJackSpacingX = 54;
		static const int rowRulerB0 = 323;
		static const int rowRulerB1 = 269;
		static const int columnRulerB0 = 22;
		static const int columnRulerB1 = columnRulerB0 + outputJackSpacingX;
		static const int columnRulerB2 = columnRulerB1 + outputJackSpacingX;
		static const int columnRulerB3 = columnRulerB2 + outputJackSpacingX;
		static const int columnRulerB4 = columnRulerB3 + outputJackSpacingX;
		static const int columnRulerB7 = columnRulerMK2 + 1;
		static const int columnRulerB6 = columnRulerB7 - outputJackSpacingX;
		static const int columnRulerB5 = columnRulerB6 - outputJackSpacingX;

		
		// Gate 1 probability light and button
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(columnRulerB0 + posLEDvsButton + offsetMediumLight, rowRulerB1 + offsetMediumLight), module, PhraseSeq16::GATE1_PROB_LIGHT));		
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerB0 + offsetCKD6b, rowRulerB1 + offsetCKD6b), module, PhraseSeq16::GATE1_PROB_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Gate 1 probability knob
		addParam(createDynamicParam<IMSmallKnob>(Vec(columnRulerB1 + offsetIMSmallKnob, rowRulerB1 + offsetIMSmallKnob), module, PhraseSeq16::GATE1_KNOB_PARAM, 0.0f, 1.0f, 1.0f, &module->panelTheme));
		// Slide light and button
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(columnRulerB2 + posLEDvsButton + offsetMediumLight, rowRulerB1 + offsetMediumLight), module, PhraseSeq16::SLIDE_LIGHT));		
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerB2 + offsetCKD6b, rowRulerB1 + offsetCKD6b), module, PhraseSeq16::SLIDE_BTN_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Slide knob
		addParam(createDynamicParam<IMSmallKnob>(Vec(columnRulerB3 + offsetIMSmallKnob, rowRulerB1 + offsetIMSmallKnob), module, PhraseSeq16::SLIDE_KNOB_PARAM, 0.0f, 2.0f, 0.2f, &module->panelTheme));
		// Autostep
		addParam(ParamWidget::create<CKSS>(Vec(columnRulerB4 + hOffsetCKSS, rowRulerB1 + vOffsetCKSS), module, PhraseSeq16::AUTOSTEP_PARAM, 0.0f, 1.0f, 1.0f));		
		// CV in
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB5, rowRulerB1), Port::INPUT, module, PhraseSeq16::CV_INPUT, &module->panelTheme));
		// Clock
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB6, rowRulerB1), Port::INPUT, module, PhraseSeq16::CLOCK_INPUT, &module->panelTheme));
		// Reset
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB7, rowRulerB1), Port::INPUT, module, PhraseSeq16::RESET_INPUT, &module->panelTheme));

		

		// ****** Bottom row (all aligned) ******

	
		// CV control Inputs 
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB0, rowRulerB0), Port::INPUT, module, PhraseSeq16::LEFTCV_INPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB1, rowRulerB0), Port::INPUT, module, PhraseSeq16::RIGHTCV_INPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB2, rowRulerB0), Port::INPUT, module, PhraseSeq16::SEQCV_INPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB3, rowRulerB0), Port::INPUT, module, PhraseSeq16::RUNCV_INPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB4, rowRulerB0), Port::INPUT, module, PhraseSeq16::WRITE_INPUT, &module->panelTheme));
		// Outputs
		addOutput(createDynamicPort<IMPort>(Vec(columnRulerB5, rowRulerB0), Port::OUTPUT, module, PhraseSeq16::CV_OUTPUT, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRulerB6, rowRulerB0), Port::OUTPUT, module, PhraseSeq16::GATE1_OUTPUT, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRulerB7, rowRulerB0), Port::OUTPUT, module, PhraseSeq16::GATE2_OUTPUT, &module->panelTheme));

		
		// Expansion module
		static const int rowRulerExpTop = 65;
		static const int rowSpacingExp = 60;
		static const int colRulerExp = 497 - 30;// PS16 is 2HP less than PS32
		addInput(expPorts[0] = createDynamicPort<IMPort>(Vec(colRulerExp, rowRulerExpTop + rowSpacingExp * 0), Port::INPUT, module, PhraseSeq16::GATE1CV_INPUT, &module->panelTheme));
		addInput(expPorts[1] = createDynamicPort<IMPort>(Vec(colRulerExp, rowRulerExpTop + rowSpacingExp * 1), Port::INPUT, module, PhraseSeq16::GATE2CV_INPUT, &module->panelTheme));
		addInput(expPorts[2] = createDynamicPort<IMPort>(Vec(colRulerExp, rowRulerExpTop + rowSpacingExp * 2), Port::INPUT, module, PhraseSeq16::TIEDCV_INPUT, &module->panelTheme));
		addInput(expPorts[3] = createDynamicPort<IMPort>(Vec(colRulerExp, rowRulerExpTop + rowSpacingExp * 3), Port::INPUT, module, PhraseSeq16::SLIDECV_INPUT, &module->panelTheme));
		addInput(expPorts[4] = createDynamicPort<IMPort>(Vec(colRulerExp, rowRulerExpTop + rowSpacingExp * 4), Port::INPUT, module, PhraseSeq16::MODECV_INPUT, &module->panelTheme));
	}
};

} // namespace rack_plugin_ImpromptuModular

using namespace rack_plugin_ImpromptuModular;

RACK_PLUGIN_MODEL_INIT(ImpromptuModular, PhraseSeq16) {
   Model *modelPhraseSeq16 = Model::create<PhraseSeq16, PhraseSeq16Widget>("Impromptu Modular", "Phrase-Seq-16", "SEQ - Phrase-Seq-16", SEQUENCER_TAG);
   return modelPhraseSeq16;
}

/*CHANGE LOG

0.6.11:
step optimization of lights refresh


0.6.10:
add advanced gate mode
unlock gates when tied (turn off when press tied, but allow to be turned back on)
allow main knob to also change length when length editing is active

0.6.9:
add FW2, FW3 and FW4 run modes for sequences (but not for song)
right click on notes now does same as left click but with autostep

0.6.8:
show length in display when editing length

0.6.7:
allow full edit capabilities in Seq and song mode
no reset on run by default, with switch added in context menu
reset does not revert seq or song number to 1
gate 2 is off by default
fix emitted monitoring gates to depend on gate states instead of always triggering

0.6.6:
knob bug fixes when loading patch

0.6.5:
paste 4, 8 doesn't loop over to overwrite first steps
small adjustements to gates and CVs used in monitoring write operations
add GATE1, GATE2, TIED, SLIDE CV inputs 
add MODE CV input (affects only selected sequence and in Seq mode)
add expansion panel option

0.6.4:
turn off keyboard and oct lights when detached in song mode (makes no sense, can't mod steps and shows stepping though seq that may not be playing)
removed mode CV input, added paste 4/8/ALL option (ALL copies length and run mode also)
allow each sequence to have its own length and run mode
merged functionalities of transpose and rotate into one knob
implemented tied notes state bit for each step, and added light to tied steps
implemented 0-T slide as opposed to 0-2s slide, where T is clock period
changed copy-paste indication, now uses display rather than keyboard lights

0.6.3: 
added tie step macro button
added gate probabilities (one prob setting for all steps)
removed paste-sync

0.6.2:
initial release of PS16
*/
