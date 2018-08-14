//***********************************************************************************************
//Semi-Modular Synthesizer module for VCV Rack by Marc Boulé and Xavier Belmont
//
//Based on code from the Fundamental and AudibleInstruments plugins by Andrew Belt 
//and graphics from the Component Library by Wes Milholen 
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//
//Module concept, desing and layout by Xavier Belmont
//Code by Marc Boulé
//
//Acknowledgements: please see README.md
//***********************************************************************************************


#include "ImpromptuModular.hpp"
#include "FundamentalUtil.hpp"

namespace rack_plugin_ImpromptuModular {

struct SemiModularSynth : Module {
	enum ParamIds {
		// SEQUENCER
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
		AUTOSTEP_PARAM,
		ENUMS(KEY_PARAMS, 12),
		RUNMODE_PARAM,
		TRAN_ROT_PARAM,
		GATE1_KNOB_PARAM,
		GATE1_PROB_PARAM,
		TIE_PARAM,// Legato
		CPMODE_PARAM,
		
		// VCO
		VCO_MODE_PARAM,
		VCO_OCT_PARAM,
		VCO_FREQ_PARAM,
		VCO_FINE_PARAM,
		VCO_FM_PARAM,
		VCO_PW_PARAM,
		VCO_PWM_PARAM,

		// CLK
		CLK_FREQ_PARAM,
		CLK_PW_PARAM,

		// VCA
		VCA_LEVEL1_PARAM,
		
		// ADSR
		ADSR_ATTACK_PARAM,
		ADSR_DECAY_PARAM,
		ADSR_SUSTAIN_PARAM,
		ADSR_RELEASE_PARAM,
		
		// VCF
		VCF_FREQ_PARAM,
		VCF_RES_PARAM,
		VCF_FREQ_CV_PARAM,
		VCF_DRIVE_PARAM,
		
		// LFO
		LFO_FREQ_PARAM,
		LFO_GAIN_PARAM,
		LFO_OFFSET_PARAM,
				
		NUM_PARAMS
	};
	enum InputIds {
		// SEQUENCER
		WRITE_INPUT,
		CV_INPUT,
		RESET_INPUT,
		CLOCK_INPUT,
		LEFTCV_INPUT,
		RIGHTCV_INPUT,
		RUNCV_INPUT,
		SEQCV_INPUT,
		
		// VCO
		VCO_PITCH_INPUT,
		VCO_FM_INPUT,
		VCO_SYNC_INPUT,
		VCO_PW_INPUT,

		// CLK
		// none
		
		// VCA
		VCA_LIN1_INPUT,
		VCA_IN1_INPUT,
		
		// ADSR
		ADSR_GATE_INPUT,

		// VCF
		VCF_FREQ_INPUT,
		VCF_RES_INPUT,
		VCF_DRIVE_INPUT,
		VCF_IN_INPUT,
		
		// LFO
		LFO_RESET_INPUT,
		
		NUM_INPUTS
	};
	enum OutputIds {
		// SEQUENCER
		CV_OUTPUT,
		GATE1_OUTPUT,
		GATE2_OUTPUT,
		
		// VCO
		VCO_SIN_OUTPUT,
		VCO_TRI_OUTPUT,
		VCO_SAW_OUTPUT,
		VCO_SQR_OUTPUT,

		// CLK
		CLK_OUT_OUTPUT,
		
		// VCA
		VCA_OUT1_OUTPUT,
		
		// ADSR
		ADSR_ENVELOPE_OUTPUT,
		
		// VCF
		VCF_LPF_OUTPUT,
		VCF_HPF_OUTPUT,
		
		// LFO
		LFO_SIN_OUTPUT,
		LFO_TRI_OUTPUT,
		
		NUM_OUTPUTS
	};
	enum LightIds {
		// SEQUENCER
		ENUMS(STEP_PHRASE_LIGHTS, 16 * 2),// room for GreenRed
		ENUMS(OCTAVE_LIGHTS, 7),// octaves 1 to 7
		ENUMS(KEY_LIGHTS, 12),
		RUN_LIGHT,
		RESET_LIGHT,
		GATE1_LIGHT,
		GATE2_LIGHT,
		SLIDE_LIGHT,
		ATTACH_LIGHT,
		GATE1_PROB_LIGHT,
		TIE_LIGHT,
		
		// VCO, CLK, VCA
		// none
		
		NUM_LIGHTS
	};
	
	enum DisplayStateIds {DISP_NORMAL, DISP_MODE, DISP_TRANSPOSE, DISP_ROTATE};
	enum AttributeBitMasks {ATT_MSK_GATE1 = 0x01, ATT_MSK_GATE1P = 0x02, ATT_MSK_GATE2 = 0x04, ATT_MSK_SLIDE = 0x08, ATT_MSK_TIED = 0x10};

	
	// SEQUENCER
	
	// Need to save
	int panelTheme = 1;
	int portTheme = 1;
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
	bool gate1RandomEnable;
	
	static constexpr float EDIT_PARAM_INIT_VALUE = 1.0f;// so that module constructor is coherent with widget initialization, since module created before widget
	bool editingSequence;
	bool editingSequenceLast;

	// VCO
	// none
	
	// CLK
	float clkValue;
	
	// VCA
	// none
	
	// ADSR
	bool decaying = false;
	float env = 0.0f;
	
	// VCF
	LadderFilter filter;
	

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

	
	inline bool getGate1(int seq, int step) {return (attributes[seq][step] & ATT_MSK_GATE1) != 0;}
	inline bool getGate2(int seq, int step) {return (attributes[seq][step] & ATT_MSK_GATE2) != 0;}
	inline bool getGate1P(int seq, int step) {return (attributes[seq][step] & ATT_MSK_GATE1P) != 0;}
	inline bool getTied(int seq, int step) {return (attributes[seq][step] & ATT_MSK_TIED) != 0;}
	inline bool isEditingSequence(void) {return params[EDIT_PARAM].value > 0.5f;}
	inline bool calcGate1RandomEnable(bool gate1P) {return (randomUniform() < (params[GATE1_KNOB_PARAM].value)) || !gate1P;}// randomUniform is [0.0, 1.0), see include/util/common.hpp
	
	
	LowFrequencyOscillator oscillatorClk;
	LowFrequencyOscillator oscillatorLfo;
	VoltageControlledOscillator oscillatorVco;


	SemiModularSynth() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}
	

	void onReset() override {
		// SEQUENCER
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
		
		// VCO
		// none
		
		// CLK
		clkValue = 0.0f;
		
		// VCF
		filter.reset();
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
				attributes[i][s] = randomu32() % 32;// 32 because 5 attributes
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
	}
	
	
	void initRun(bool hard) {// run button activated or run edge in run input jack or edit mode toggled
		if (hard) {
			phraseIndexRun = (runModeSong == MODE_REV ? phrases - 1 : 0);
			if (editingSequence)
				stepIndexRun = (runModeSeq[sequence] == MODE_REV ? lengths[sequence] - 1 : 0);
			else
				stepIndexRun = (runModeSeq[phrase[phraseIndexRun]] == MODE_REV ? lengths[phrase[phraseIndexRun]] - 1 : 0);
		}
		gate1RandomEnable = false;
		if (editingSequence)
			gate1RandomEnable = calcGate1RandomEnable(getGate1P(sequence, stepIndexRun));
		else
			gate1RandomEnable = calcGate1RandomEnable(getGate1P(phrase[phraseIndexRun], stepIndexRun));
		clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * engineGetSampleRate());	
	}
	
	
	json_t *toJson() override {
		json_t *rootJ = json_object();

		// panelTheme and portTheme
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));
		json_object_set_new(rootJ, "portTheme", json_integer(portTheme));

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
		// panelTheme and portTheme
		json_t *panelThemeJ = json_object_get(rootJ, "panelTheme");
		if (panelThemeJ)
			panelTheme = json_integer_value(panelThemeJ);
		json_t *portThemeJ = json_object_get(rootJ, "portTheme");
		if (portThemeJ)
			portTheme = json_integer_value(portThemeJ);

		// running
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);

		// runModeSeq
		json_t *runModeSeqJ = json_object_get(rootJ, "runModeSeq2");// "2" was in PS16. No legacy in SMS16 though
		if (runModeSeqJ) {
			for (int i = 0; i < 16; i++)
			{
				json_t *runModeSeqArrayJ = json_array_get(runModeSeqJ, i);
				if (runModeSeqArrayJ)
					runModeSeq[i] = json_integer_value(runModeSeqArrayJ);
			}			
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

		// SEQUENCER

		static const float gateTime = 0.4f;// seconds
		static const float copyPasteInfoTime = 0.5f;// seconds
		static const float editLengthTime = 2.0f;// seconds
		static const float tiedWarningTime = 0.7f;// seconds
		long tiedWarningInit = (long) (tiedWarningTime * engineGetSampleRate());
		
		
		//********** Buttons, knobs, switches and inputs **********
		
		// Notes: 
		// * a tied step's attributes can not be modified by any of the following: 
		//   write input, oct and keyboard buttons, gate1, gate1Prob, gate2 and slide buttons
		//   however, paste, transpose, rotate obviously can.
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
			//if (stepIndexEdit >= lengths[sequence])// Commented for full edit capabilities
				//stepIndexEdit = lengths[sequence] - 1;// Commented for full edit capabilities
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
				infoCopyPaste = (long) (copyPasteInfoTime * engineGetSampleRate());
				//CPinfo must be set to 0 for copy/paste all, and 0x1ii for copy/paste 4 at pos ii, 0x2ii for copy/paste 8 at 0xii
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
				infoCopyPaste = (long) (-1 * copyPasteInfoTime * engineGetSampleRate());
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
				editingLength = (unsigned long) (editLengthTime * engineGetSampleRate());
			displayState = DISP_NORMAL;
		}
		
		// Write input (must be before Left and Right in case route gate simultaneously to Right and Write for example)
		//  (write must be to correct step)
		bool writeTrig = writeTrigger.process(inputs[WRITE_INPUT].value);
		if (writeTrig) {
			if (editingSequence) {
				if (getTied(sequence,stepIndexEdit))
					tiedWarning = tiedWarningInit;
				else {			
					cv[sequence][stepIndexEdit] = inputs[CV_INPUT].value;
					applyTiedStep(sequence, stepIndexEdit, lengths[sequence]);
					editingGate = (unsigned long) (gateTime * engineGetSampleRate());
					editingGateCV = cv[sequence][stepIndexEdit];
					editingGateKeyLight = -1;
					// Autostep (after grab all active inputs)
					if (params[AUTOSTEP_PARAM].value > 0.5f)
						stepIndexEdit = moveIndex(stepIndexEdit, stepIndexEdit + 1, 16);//lengths[sequence]);// Commented for full edit capabilities
				}
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
				editingLength = (unsigned long) (editLengthTime * engineGetSampleRate());// restart editing length timer
				if (editingSequence) {
					lengths[sequence] += delta;
					if (lengths[sequence] > 16) lengths[sequence] = 16;
					if (lengths[sequence] < 1 ) lengths[sequence] = 1;
					//if (stepIndexEdit >= lengths[sequence])// Commented for full edit capabilities
						//stepIndexEdit = lengths[sequence] - 1;// Commented for full edit capabilities
				}
				else {
					phrases += delta;
					if (phrases > 16) phrases = 16;
					if (phrases < 1 ) phrases = 1;
					//if (phraseIndexEdit >= phrases) phraseIndexEdit = phrases - 1;// Commented for full edit capabilities
				}
			}
			else {
				if (!running || !attached) {// don't move heads when attach and running
					if (editingSequence) {
						stepIndexEdit = moveIndex(stepIndexEdit, stepIndexEdit + delta, 16);//lengths[sequence]);// Commented for full edit capabilities
						if (!getTied(sequence,stepIndexEdit)) {// play if non-tied step
							if (!writeTrig) {// in case autostep when simultaneous writeCV and stepCV (keep what was done in Write Input block above)
								editingGate = (unsigned long) (gateTime * engineGetSampleRate());
								editingGateCV = cv[sequence][stepIndexEdit];
								editingGateKeyLight = -1;
							}
						}
					}
					else
						phraseIndexEdit = moveIndex(phraseIndexEdit, phraseIndexEdit + delta, 16);//phrases);// Commented for full edit capabilities
				}
			}
		}
		
		// Mode and Transpose/Rotate buttons
		if (modeTrigger.process(params[RUNMODE_PARAM].value)) {
			if (displayState != DISP_MODE)
				displayState = DISP_MODE;
			else
				displayState = DISP_NORMAL;
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
					editingLength = (unsigned long) (editLengthTime * engineGetSampleRate());// restart editing length timer
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
				else if (displayState == DISP_MODE) {
					if (editingSequence) {
						runModeSeq[sequence] += deltaKnob;
						if (runModeSeq[sequence] < 0) runModeSeq[sequence] = 0;
						if (runModeSeq[sequence] >= NUM_MODES) runModeSeq[sequence] = NUM_MODES - 1;
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
							//if (stepIndexEdit >= lengths[sequence])// Commented for full edit capabilities
								//stepIndexEdit = lengths[sequence] - 1;// Commented for full edit capabilities
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
					tiedWarning = tiedWarningInit;
				else {			
					float newCV = cv[sequence][stepIndexEdit] + 10.0f;//to properly handle negative note voltages
					newCV = newCV - floor(newCV) + (float) (newOct - 3);
					if (newCV >= -3.0f && newCV < 4.0f) {
						cv[sequence][stepIndexEdit] = newCV;
						applyTiedStep(sequence, stepIndexEdit, lengths[sequence]);
					}
					editingGate = (unsigned long) (gateTime * engineGetSampleRate());
					editingGateCV = cv[sequence][stepIndexEdit];
					editingGateKeyLight = -1;
				}
			}
		}		
		
		// Keyboard buttons
		for (int i = 0; i < 12; i++) {
			if (keyTriggers[i].process(params[KEY_PARAMS + i].value)) {
				if (editingSequence) {
					if (getTied(sequence,stepIndexEdit)) {
						if (params[KEY_PARAMS + i].value > 1.5f)
							stepIndexEdit = moveIndex(stepIndexEdit, stepIndexEdit + 1, 16);
						else
							tiedWarning = tiedWarningInit;
					}
					else {			
						cv[sequence][stepIndexEdit] = floor(cv[sequence][stepIndexEdit]) + ((float) i) / 12.0f;
						applyTiedStep(sequence, stepIndexEdit, lengths[sequence]);
						editingGate = (unsigned long) (gateTime * engineGetSampleRate());
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
				if (getTied(sequence,stepIndexEdit))
					tiedWarning = tiedWarningInit;
				else
					attributes[sequence][stepIndexEdit] ^= ATT_MSK_GATE1;
			}
			displayState = DISP_NORMAL;
		}		
		if (gate1ProbTrigger.process(params[GATE1_PROB_PARAM].value)) {
			if (editingSequence) {
				if (getTied(sequence,stepIndexEdit))
					tiedWarning = tiedWarningInit;
				else
					attributes[sequence][stepIndexEdit] ^= ATT_MSK_GATE1P;
			}
			displayState = DISP_NORMAL;
		}		
		if (gate2Trigger.process(params[GATE2_PARAM].value)) {
			if (editingSequence) {
				if (getTied(sequence,stepIndexEdit))
					tiedWarning = tiedWarningInit;
				else
					attributes[sequence][stepIndexEdit] ^= ATT_MSK_GATE2;
			}
			displayState = DISP_NORMAL;
		}		
		if (slideTrigger.process(params[SLIDE_BTN_PARAM].value)) {
			if (editingSequence) {
				if (getTied(sequence,stepIndexEdit))
					tiedWarning = tiedWarningInit;
				else
					attributes[sequence][stepIndexEdit] ^= ATT_MSK_SLIDE;
			}
			displayState = DISP_NORMAL;
		}		
		if (tiedTrigger.process(params[TIE_PARAM].value)) {
			if (editingSequence) {
				attributes[sequence][stepIndexEdit] ^= ATT_MSK_TIED;
				if (getTied(sequence,stepIndexEdit)) {
					attributes[sequence][stepIndexEdit] = ATT_MSK_TIED;// clear other attributes if tied
					applyTiedStep(sequence, stepIndexEdit, lengths[sequence]);
				}
				else
					attributes[sequence][stepIndexEdit] |= (ATT_MSK_GATE1 | ATT_MSK_GATE2);
			}
			displayState = DISP_NORMAL;
		}		
		
		
		//********** Clock and reset **********
		
		// Clock
		float clockInput = inputs[CLOCK_INPUT].active ? inputs[CLOCK_INPUT].value : clkValue;// Pre-patching
		if (clockTrigger.process(clockInput)) {
			if (running && clockIgnoreOnReset == 0l) {
				float slideFromCV = 0.0f;
				float slideToCV = 0.0f;
				if (editingSequence) {
					slideFromCV = cv[sequence][stepIndexRun];
					moveIndexRunMode(&stepIndexRun, lengths[sequence], runModeSeq[sequence], &stepIndexRunHistory);
					slideToCV = cv[sequence][stepIndexRun];
					gate1RandomEnable = calcGate1RandomEnable(getGate1P(sequence,stepIndexRun));// must be calculated on clock edge only
				}
				else {
					slideFromCV = cv[phrase[phraseIndexRun]][stepIndexRun];
					if (moveIndexRunMode(&stepIndexRun, lengths[phrase[phraseIndexRun]], runModeSeq[phrase[phraseIndexRun]], &stepIndexRunHistory)) {
						moveIndexRunMode(&phraseIndexRun, phrases, runModeSong, &phraseIndexRunHistory);
						stepIndexRun = (runModeSeq[phrase[phraseIndexRun]] == MODE_REV ? lengths[phrase[phraseIndexRun]] - 1 : 0);// must always refresh after phraseIndexRun has changed
					}
					slideToCV = cv[phrase[phraseIndexRun]][stepIndexRun];
					gate1RandomEnable = calcGate1RandomEnable(getGate1P(phrase[phraseIndexRun],stepIndexRun));// must be calculated on clock edge only
				}
				
				// Slide
				if ( (editingSequence && ((attributes[sequence][stepIndexRun] & ATT_MSK_SLIDE) != 0) ) || 
				    (!editingSequence && ((attributes[phrase[phraseIndexRun]][stepIndexRun] & ATT_MSK_SLIDE) != 0) ) ) {
					// avtivate sliding (slideStepsRemain can be reset, else runs down to 0, either way, no need to reinit)
					slideStepsRemain =   (unsigned long) (((float)clockPeriod)   * params[SLIDE_KNOB_PARAM].value / 2.0f);// 0-T slide, where T is clock period (can be too long when user does clock gating)
					//slideStepsRemain = (unsigned long)  (engineGetSampleRate() * params[SLIDE_KNOB_PARAM].value );// 0-2s slide
					slideCVdelta = (slideToCV - slideFromCV)/(float)slideStepsRemain;
				}
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
		else
			resetLight -= (resetLight / lightLambda) * engineGetSampleTime();
		
		
		//********** Outputs and lights **********
				
		// CV and gates outputs
		int seq = editingSequence ? (sequence) : (running ? phrase[phraseIndexRun] : phrase[phraseIndexEdit]);
		int step = editingSequence ? (running ? stepIndexRun : stepIndexEdit) : (stepIndexRun);
		if (running) {
			float slideOffset = (slideStepsRemain > 0ul ? (slideCVdelta * (float)slideStepsRemain) : 0.0f);
			outputs[CV_OUTPUT].value = cv[seq][step] - slideOffset;
			outputs[GATE1_OUTPUT].value = (clockTrigger.isHigh() && gate1RandomEnable && 
											((attributes[seq][step] & ATT_MSK_GATE1) != 0)) ? 10.0f : 0.0f;
			outputs[GATE2_OUTPUT].value = (clockTrigger.isHigh() && 
											((attributes[seq][step] & ATT_MSK_GATE2) != 0)) ? 10.0f : 0.0f;
		}
		else {// not running 
			outputs[CV_OUTPUT].value = (editingGate > 0ul) ? editingGateCV : cv[seq][step];
			outputs[GATE1_OUTPUT].value = (editingGate > 0ul) ? 10.0f : 0.0f;
			outputs[GATE2_OUTPUT].value = (editingGate > 0ul) ? 10.0f : 0.0f;
		}
		
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
					bool warningFlashState = calcWarningFlash(tiedWarning, tiedWarningInit);
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
		for (int i = 0; i < 12; i++) {
			if (!editingSequence && (!attached || !running))// no keyboard lights when song mode and either (detached [1] or stopped [2])
											// [1] makes no sense, can't mod steps and stepping though seq that may not be playing
											// [2] CV is set to 0V when not running and in song mode, so cv[][] makes no sense to display
				lights[KEY_LIGHTS + i].value = 0.0f;
			else {
				if (tiedWarning > 0l) {
					bool warningFlashState = calcWarningFlash(tiedWarning, tiedWarningInit);
					lights[KEY_LIGHTS + i].value = (warningFlashState && i == keyLightIndex) ? 1.0f : 0.0f;
				}
				else {
					if (editingGate > 0ul && editingGateKeyLight != -1)
						lights[KEY_LIGHTS + i].value = (i == editingGateKeyLight ? ((float) editingGate / (float)(gateTime * engineGetSampleRate())) : 0.0f);
					else
						lights[KEY_LIGHTS + i].value = (i == keyLightIndex ? 1.0f : 0.0f);
				}
			}
		}			
		
		// Gate1, Gate1Prob, Gate2, Slide and Tied lights
		int attributesVal = attributes[sequence][stepIndexEdit];
		if (!editingSequence)
			attributesVal = attributes[phrase[phraseIndexEdit]][stepIndexRun];
		//
		lights[GATE1_LIGHT].value = ((attributesVal & ATT_MSK_GATE1) != 0) ? 1.0f : 0.0f;
		lights[GATE1_PROB_LIGHT].value = ((attributesVal & ATT_MSK_GATE1P) != 0) ? 1.0f : 0.0f;
		lights[GATE2_LIGHT].value = ((attributesVal & ATT_MSK_GATE2) != 0) ? 1.0f : 0.0f;
		lights[SLIDE_LIGHT].value = ((attributesVal & ATT_MSK_SLIDE) != 0) ? 1.0f : 0.0f;
		if (tiedWarning > 0l) {
			bool warningFlashState = calcWarningFlash(tiedWarning, tiedWarningInit);
			lights[TIE_LIGHT].value = (warningFlashState) ? 1.0f : 0.0f;
		}
		else
			lights[TIE_LIGHT].value = ((attributesVal & ATT_MSK_TIED) != 0) ? 1.0f : 0.0f;

		// Attach light
		lights[ATTACH_LIGHT].value = (running && attached) ? 1.0f : 0.0f;
		
		// Reset light
		lights[RESET_LIGHT].value =	resetLight;	
		
		// Run light
		lights[RUN_LIGHT].value = lights[RUN_LIGHT].value = running ? 1.0f : 0.0f;

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
		if (slideStepsRemain > 0ul)
			slideStepsRemain--;
		if (clockIgnoreOnReset > 0l)
			clockIgnoreOnReset--;
		if (tiedWarning > 0l)
			tiedWarning--;

		
		// VCO
		oscillatorVco.analog = params[VCO_MODE_PARAM].value > 0.0f;
		oscillatorVco.soft = false;//params[VCO_SYNC_PARAM].value <= 0.0f;
		float pitchFine = 3.0f * quadraticBipolar(params[VCO_FINE_PARAM].value);
		float pitchCv = 12.0f * (inputs[VCO_PITCH_INPUT].active ? inputs[VCO_PITCH_INPUT].value : outputs[CV_OUTPUT].value);// Pre-patching
		float pitchOctOffset = 12.0f * params[VCO_OCT_PARAM].value;
		if (inputs[VCO_FM_INPUT].active) {
			pitchCv += quadraticBipolar(params[VCO_FM_PARAM].value) * 12.0f * inputs[VCO_FM_INPUT].value;
		}
		oscillatorVco.setPitch(params[VCO_FREQ_PARAM].value, pitchFine + pitchCv + pitchOctOffset);
		oscillatorVco.setPulseWidth(params[VCO_PW_PARAM].value + params[VCO_PWM_PARAM].value * inputs[VCO_PW_INPUT].value / 10.0f);
		oscillatorVco.syncEnabled = inputs[VCO_SYNC_INPUT].active;
		oscillatorVco.process(engineGetSampleTime(), inputs[VCO_SYNC_INPUT].value);
		if (outputs[VCO_SIN_OUTPUT].active)
			outputs[VCO_SIN_OUTPUT].value = 5.0f * oscillatorVco.sin();
		if (outputs[VCO_TRI_OUTPUT].active)
			outputs[VCO_TRI_OUTPUT].value = 5.0f * oscillatorVco.tri();
		if (outputs[VCO_SAW_OUTPUT].active)
			outputs[VCO_SAW_OUTPUT].value = 5.0f * oscillatorVco.saw();
		//if (outputs[VCO_SQR_OUTPUT].active)
			outputs[VCO_SQR_OUTPUT].value = 5.0f * oscillatorVco.sqr();		
			
			
		// CLK
		oscillatorClk.setPitch(params[CLK_FREQ_PARAM].value);
		oscillatorClk.setPulseWidth(params[CLK_PW_PARAM].value);
		oscillatorClk.offset = true;//(params[OFFSET_PARAM].value > 0.0f);
		oscillatorClk.invert = false;//(params[INVERT_PARAM].value <= 0.0f);
		oscillatorClk.step(engineGetSampleTime());
		oscillatorClk.setReset(inputs[RESET_INPUT].value + params[RESET_PARAM].value + params[RUN_PARAM].value + inputs[RUNCV_INPUT].value);//inputs[RESET_INPUT].value);
		clkValue = 5.0f * oscillatorClk.sqr();	
		outputs[CLK_OUT_OUTPUT].value = clkValue;
		
		
		// VCA
		float vcaIn = inputs[VCA_IN1_INPUT].active ? inputs[VCA_IN1_INPUT].value : outputs[VCO_SQR_OUTPUT].value;// Pre-patching
		float vcaLin = inputs[VCA_LIN1_INPUT].active ? inputs[VCA_LIN1_INPUT].value : outputs[ADSR_ENVELOPE_OUTPUT].value;// Pre-patching
		float v = vcaIn * params[VCA_LEVEL1_PARAM].value;
		v *= clamp(vcaLin / 10.0f, 0.0f, 1.0f);
		outputs[VCA_OUT1_OUTPUT].value = v;

				
		// ADSR
		float attack = clamp(params[ADSR_ATTACK_PARAM].value, 0.0f, 1.0f);
		float decay = clamp(params[ADSR_DECAY_PARAM].value, 0.0f, 1.0f);
		float sustain = clamp(params[ADSR_SUSTAIN_PARAM].value, 0.0f, 1.0f);
		float release = clamp(params[ADSR_RELEASE_PARAM].value, 0.0f, 1.0f);
		// Gate
		float adsrIn = inputs[ADSR_GATE_INPUT].active ? inputs[ADSR_GATE_INPUT].value : outputs[GATE1_OUTPUT].value;// Pre-patching
		bool gated = adsrIn >= 1.0f;
		const float base = 20000.0f;
		const float maxTime = 10.0f;
		if (gated) {
			if (decaying) {
				// Decay
				if (decay < 1e-4) {
					env = sustain;
				}
				else {
					env += powf(base, 1 - decay) / maxTime * (sustain - env) * engineGetSampleTime();
				}
			}
			else {
				// Attack
				// Skip ahead if attack is all the way down (infinitely fast)
				if (attack < 1e-4) {
					env = 1.0f;
				}
				else {
					env += powf(base, 1 - attack) / maxTime * (1.01f - env) * engineGetSampleTime();
				}
				if (env >= 1.0f) {
					env = 1.0f;
					decaying = true;
				}
			}
		}
		else {
			// Release
			if (release < 1e-4) {
				env = 0.0f;
			}
			else {
				env += powf(base, 1 - release) / maxTime * (0.0f - env) * engineGetSampleTime();
			}
			decaying = false;
		}
		outputs[ADSR_ENVELOPE_OUTPUT].value = 10.0f * env;
		
		
		// VCF
		if (outputs[VCF_LPF_OUTPUT].active || outputs[VCF_HPF_OUTPUT].active) {
		
			float input = (inputs[VCF_IN_INPUT].active ? inputs[VCF_IN_INPUT].value : outputs[VCA_OUT1_OUTPUT].value) / 5.0f;// Pre-patching
			float drive = clamp(params[VCF_DRIVE_PARAM].value + inputs[VCF_DRIVE_INPUT].value / 10.0f, 0.f, 1.f);
			float gain = powf(1.f + drive, 5);
			input *= gain;
			// Add -60dB noise to bootstrap self-oscillation
			input += 1e-6f * (2.f * randomUniform() - 1.f);
			// Set resonance
			float res = clamp(params[VCF_RES_PARAM].value + inputs[VCF_RES_INPUT].value / 10.f, 0.f, 1.f);
			filter.resonance = powf(res, 2) * 10.f;
			// Set cutoff frequency
			float pitch = 0.f;
			if (inputs[VCF_FREQ_INPUT].active)
				pitch += inputs[VCF_FREQ_INPUT].value * quadraticBipolar(params[VCF_FREQ_CV_PARAM].value);
			pitch += params[VCF_FREQ_PARAM].value * 10.f - 5.f;
			//pitch += quadraticBipolar(params[FINE_PARAM].value * 2.f - 1.f) * 7.f / 12.f;
			float cutoff = 261.626f * powf(2.f, pitch);
			cutoff = clamp(cutoff, 1.f, 8000.f);
			filter.setCutoff(cutoff);
			filter.process(input, engineGetSampleTime());
			outputs[VCF_LPF_OUTPUT].value = 5.f * filter.lowpass;
			outputs[VCF_HPF_OUTPUT].value = 5.f * filter.highpass;	
		}			
		else {
			outputs[VCF_LPF_OUTPUT].value = 0.0f;
			outputs[VCF_HPF_OUTPUT].value = 0.0f;
		}
		
		// LFO
		if (outputs[LFO_SIN_OUTPUT].active || outputs[LFO_TRI_OUTPUT].active) {
			oscillatorLfo.setPitch(params[LFO_FREQ_PARAM].value);
			oscillatorLfo.setPulseWidth(0.5f);//params[PW_PARAM].value + params[PWM_PARAM].value * inputs[PW_INPUT].value / 10.0f);
			oscillatorLfo.offset = false;//(params[OFFSET_PARAM].value > 0.0f);
			oscillatorLfo.invert = false;//(params[INVERT_PARAM].value <= 0.0f);
			oscillatorLfo.step(engineGetSampleTime());
			oscillatorLfo.setReset(inputs[LFO_RESET_INPUT].value + inputs[RESET_INPUT].value + params[RESET_PARAM].value + params[RUN_PARAM].value + inputs[RUNCV_INPUT].value);
			float lfoGain = params[LFO_GAIN_PARAM].value;
			float lfoOffset = (2.0f - lfoGain) * params[LFO_OFFSET_PARAM].value;
			outputs[LFO_SIN_OUTPUT].value = 5.0f * (lfoOffset + lfoGain * oscillatorLfo.sin());
			outputs[LFO_TRI_OUTPUT].value = 5.0f * (lfoOffset + lfoGain * oscillatorLfo.tri());	
		} 
		else {
			outputs[LFO_SIN_OUTPUT].value = 0.0f;
			outputs[LFO_TRI_OUTPUT].value = 0.0f;
		}			
		
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
};



struct SemiModularSynthWidget : ModuleWidget {
	SemiModularSynth *module;
	DynamicSVGPanel *panel;

	struct SequenceDisplayWidget : TransparentWidget {
		SemiModularSynth *module;
		std::shared_ptr<Font> font;
		char displayStr[4];
		//std::string modeLabels[5]={"FWD","REV","PPG","BRN","RND"};
		
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
				if (module->infoCopyPaste > 0l) {// if copy display "CPY"
					snprintf(displayStr, 4, "CPY");
				}
				else {// if paste display "PST"
					snprintf(displayStr, 4, "PST");
				}
			}
			else {
				if (module->displayState == SemiModularSynth::DISP_MODE) {
					if (module->editingSequence)
						runModeToStr(module->runModeSeq[module->sequence]);
					else
						runModeToStr(module->runModeSong);
				}
				else if (module->editingLength > 0ul) {
					if (module->editingSequence)
						snprintf(displayStr, 4, "L%2u", (unsigned) module->lengths[module->sequence]);
					else
						snprintf(displayStr, 4, "L%2u", (unsigned) module->phrases);
				}
				else if (module->displayState == SemiModularSynth::DISP_TRANSPOSE) {
					snprintf(displayStr, 4, "+%2u", (unsigned) abs(module->transposeOffset));
					if (module->transposeOffset < 0)
						displayStr[0] = '-';
				}
				else if (module->displayState == SemiModularSynth::DISP_ROTATE) {
					snprintf(displayStr, 4, ")%2u", (unsigned) abs(module->rotateOffset));
					if (module->rotateOffset < 0)
						displayStr[0] = '(';
				}
				else {// DISP_NORMAL
					snprintf(displayStr, 4, " %2u", (unsigned) (module->editingSequence ? 
						module->sequence : module->phrase[module->phraseIndexEdit]) + 1 );
				}
			}
			displayStr[3] = 0;// more safety
			nvgText(vg, textPos.x, textPos.y, displayStr, NULL);
		}
	};		
	
	struct PanelThemeItem : MenuItem {
		SemiModularSynth *module;
		int panelTheme;
		int portTheme;
		void onAction(EventAction &e) override {
			module->panelTheme = panelTheme;
			module->portTheme = portTheme;
		}
		void step() override {
			rightText = ((module->panelTheme == panelTheme) && (module->portTheme == portTheme)) ? "✔" : "";
		}
	};
	struct ResetOnRunItem : MenuItem {
		SemiModularSynth *module;
		void onAction(EventAction &e) override {
			module->resetOnRun = !module->resetOnRun;
		}
	};
	Menu *createContextMenu() override {
		Menu *menu = ModuleWidget::createContextMenu();

		MenuLabel *spacerLabel = new MenuLabel();
		menu->addChild(spacerLabel);

		SemiModularSynth *module = dynamic_cast<SemiModularSynth*>(this->module);
		assert(module);

		MenuLabel *themeLabel = new MenuLabel();
		themeLabel->text = "Panel Theme";
		menu->addChild(themeLabel);

		PanelThemeItem *classicItem = new PanelThemeItem();
		classicItem->text = lightPanelID;// ImpromptuModular.hpp
		classicItem->module = module;
		classicItem->panelTheme = 0;
		classicItem->portTheme = 0;
		menu->addChild(classicItem);

		PanelThemeItem *lightItem = new PanelThemeItem();
		lightItem->text = "Light";
		lightItem->module = module;
		lightItem->panelTheme = 0;
		lightItem->portTheme = 1;
		menu->addChild(lightItem);

		PanelThemeItem *darkItem = new PanelThemeItem();
		darkItem->text = darkPanelID;// ImpromptuModular.hpp
		darkItem->module = module;
		darkItem->panelTheme = 1;
		darkItem->portTheme = 1;
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
	
	SemiModularSynthWidget(SemiModularSynth *module) : ModuleWidget(module) {
		this->module = module;
		
		// SEQUENCER 
		
		// Main panel from Inkscape
        panel = new DynamicSVGPanel();
        panel->mode = &module->panelTheme;
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/SemiModular.svg")));
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/dark/SemiModular_dark.svg")));
        box.size = panel->box.size;
        addChild(panel);		
		
		// Screws
		addChild(createDynamicScrew<IMScrew>(Vec(15, 0), &module->portTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(15, 365), &module->portTheme));
		addChild(createDynamicScrew<IMScrew>(Vec((box.size.x - 90) * 1 / 3 + 30 , 0), &module->portTheme));
		addChild(createDynamicScrew<IMScrew>(Vec((box.size.x - 90) * 1 / 3 + 30 , 365), &module->portTheme));
		addChild(createDynamicScrew<IMScrew>(Vec((box.size.x - 90) * 2 / 3 + 45 , 0), &module->portTheme));
		addChild(createDynamicScrew<IMScrew>(Vec((box.size.x - 90) * 2 / 3 + 45 , 365), &module->portTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(box.size.x-30, 0), &module->portTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(box.size.x-30, 365), &module->portTheme));

		
		
		// ****** Top row ******
		
		static const int rowRulerT0 = 52;
		static const int columnRulerT0 = 19;// Length button
		static const int columnRulerT1 = columnRulerT0 + 47;// Left/Right buttons
		static const int columnRulerT2 = columnRulerT1 + 75;// Step/Phase lights
		static const int columnRulerT3 = columnRulerT2 + 263;// Attach (also used to align rest of right side of module)

		// Length button
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerT0 + offsetCKD6b, rowRulerT0 + offsetCKD6b), module, SemiModularSynth::LENGTH_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Left/Right buttons
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerT1 + offsetCKD6b, rowRulerT0 + offsetCKD6b), module, SemiModularSynth::LEFT_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerT1 + 38 + offsetCKD6b, rowRulerT0 + offsetCKD6b), module, SemiModularSynth::RIGHT_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Step/Phrase lights
		static const int spLightsSpacing = 15;
		for (int i = 0; i < 16; i++) {
			addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(columnRulerT2 + spLightsSpacing * i + offsetMediumLight, rowRulerT0 + offsetMediumLight), module, SemiModularSynth::STEP_PHRASE_LIGHTS + (i*2)));
		}
		// Attach button and light
		addParam(ParamWidget::create<TL1105>(Vec(columnRulerT3 - 4, rowRulerT0 + 2 + offsetTL1105), module, SemiModularSynth::ATTACH_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(columnRulerT3 + 12 + offsetMediumLight, rowRulerT0 + offsetMediumLight), module, SemiModularSynth::ATTACH_LIGHT));		

		
		
		// ****** Octave and keyboard area ******
		
		// Octave LED buttons
		static const float octLightsIntY = 20.0f;
		for (int i = 0; i < 7; i++) {
			addParam(ParamWidget::create<LEDButton>(Vec(columnRulerT0 + 3, 86 + 24 + i * octLightsIntY- 4.4f), module, SemiModularSynth::OCTAVE_PARAM + i, 0.0f, 1.0f, 0.0f));
			addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(columnRulerT0 + 3 + 4.4f, 86 + 24 + i * octLightsIntY), module, SemiModularSynth::OCTAVE_LIGHTS + i));
		}
		// Keys and Key lights
		static const int keyNudgeX = 7;
		static const int keyNudgeY = 2;
		static const int offsetKeyLEDx = 6;
		static const int offsetKeyLEDy = 28;
		// Black keys and lights
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(69+keyNudgeX, 93+keyNudgeY), module, SemiModularSynth::KEY_PARAMS + 1, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(69+keyNudgeX+offsetKeyLEDx, 93+keyNudgeY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 1));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(97+keyNudgeX, 93+keyNudgeY), module, SemiModularSynth::KEY_PARAMS + 3, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(97+keyNudgeX+offsetKeyLEDx, 93+keyNudgeY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 3));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(154+keyNudgeX, 93+keyNudgeY), module, SemiModularSynth::KEY_PARAMS + 6, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(154+keyNudgeX+offsetKeyLEDx, 93+keyNudgeY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 6));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(182+keyNudgeX, 93+keyNudgeY), module, SemiModularSynth::KEY_PARAMS + 8, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(182+keyNudgeX+offsetKeyLEDx, 93+keyNudgeY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 8));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(210+keyNudgeX, 93+keyNudgeY), module, SemiModularSynth::KEY_PARAMS + 10, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(210+keyNudgeX+offsetKeyLEDx, 93+keyNudgeY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 10));
		// White keys and lights
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(55+keyNudgeX, 143+keyNudgeY), module, SemiModularSynth::KEY_PARAMS + 0, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(55+keyNudgeX+offsetKeyLEDx, 143+keyNudgeY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 0));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(83+keyNudgeX, 143+keyNudgeY), module, SemiModularSynth::KEY_PARAMS + 2, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(83+keyNudgeX+offsetKeyLEDx, 143+keyNudgeY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 2));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(111+keyNudgeX, 143+keyNudgeY), module, SemiModularSynth::KEY_PARAMS + 4, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(111+keyNudgeX+offsetKeyLEDx, 143+keyNudgeY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 4));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(140+keyNudgeX, 143+keyNudgeY), module, SemiModularSynth::KEY_PARAMS + 5, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(140+keyNudgeX+offsetKeyLEDx, 143+keyNudgeY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 5));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(168+keyNudgeX, 143+keyNudgeY), module, SemiModularSynth::KEY_PARAMS + 7, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(168+keyNudgeX+offsetKeyLEDx, 143+keyNudgeY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 7));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(196+keyNudgeX, 143+keyNudgeY), module, SemiModularSynth::KEY_PARAMS + 9, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(196+keyNudgeX+offsetKeyLEDx, 143+keyNudgeY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 9));
		addParam(ParamWidget::create<InvisibleKeySmall>(			Vec(224+keyNudgeX, 143+keyNudgeY), module, SemiModularSynth::KEY_PARAMS + 11, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(224+keyNudgeX+offsetKeyLEDx, 143+keyNudgeY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 11));
		
		
		
		// ****** Right side control area ******

		static const int rowRulerMK0 = 105;// Edit mode row
		static const int rowRulerMK1 = rowRulerMK0 + 56; // Run row
		static const int rowRulerMK2 = rowRulerMK1 + 54; // Reset row
		static const int columnRulerMK0 = 280;// Edit mode column
		static const int columnRulerMK1 = columnRulerMK0 + 59;// Display column
		static const int columnRulerMK2 = columnRulerT3;// Run mode column
		
		// Edit mode switch
		addParam(ParamWidget::create<CKSS>(Vec(columnRulerMK0 + hOffsetCKSS, rowRulerMK0 + vOffsetCKSS), module, SemiModularSynth::EDIT_PARAM, 0.0f, 1.0f, SemiModularSynth::EDIT_PARAM_INIT_VALUE));
		// Sequence display
		SequenceDisplayWidget *displaySequence = new SequenceDisplayWidget();
		displaySequence->box.pos = Vec(columnRulerMK1-15, rowRulerMK0 + 3 + vOffsetDisplay);
		displaySequence->box.size = Vec(55, 30);// 3 characters
		displaySequence->module = module;
		addChild(displaySequence);
		// Run mode button
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerMK2 + offsetCKD6b, rowRulerMK0 + 0 + offsetCKD6b), module, SemiModularSynth::RUNMODE_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));

		// Run LED bezel and light
		addParam(ParamWidget::create<LEDBezel>(Vec(columnRulerMK0 + offsetLEDbezel, rowRulerMK1 + 7 + offsetLEDbezel), module, SemiModularSynth::RUN_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(Vec(columnRulerMK0 + offsetLEDbezel + offsetLEDbezelLight, rowRulerMK1 + 7 + offsetLEDbezel + offsetLEDbezelLight), module, SemiModularSynth::RUN_LIGHT));
		// Sequence knob
		addParam(createDynamicParam<IMBigKnobInf>(Vec(columnRulerMK1 + 1 + offsetIMBigKnob, rowRulerMK0 + 55 + offsetIMBigKnob), module, SemiModularSynth::SEQUENCE_PARAM, -INFINITY, INFINITY, 0.0f, &module->portTheme));		
		// Transpose/rotate button
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerMK2 + offsetCKD6b, rowRulerMK1 + 4 + offsetCKD6b), module, SemiModularSynth::TRAN_ROT_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		
		// Reset LED bezel and light
		addParam(ParamWidget::create<LEDBezel>(Vec(columnRulerMK0 + offsetLEDbezel, rowRulerMK2 + 5 + offsetLEDbezel), module, SemiModularSynth::RESET_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(Vec(columnRulerMK0 + offsetLEDbezel + offsetLEDbezelLight, rowRulerMK2 + 5 + offsetLEDbezel + offsetLEDbezelLight), module, SemiModularSynth::RESET_LIGHT));
		// Copy/paste buttons
		addParam(ParamWidget::create<TL1105>(Vec(columnRulerMK1 - 10, rowRulerMK2 + 5 + offsetTL1105), module, SemiModularSynth::COPY_PARAM, 0.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<TL1105>(Vec(columnRulerMK1 + 20, rowRulerMK2 + 5 + offsetTL1105), module, SemiModularSynth::PASTE_PARAM, 0.0f, 1.0f, 0.0f));
		// Copy-paste mode switch (3 position)
		addParam(ParamWidget::create<CKSSThreeInv>(Vec(columnRulerMK2 + hOffsetCKSS + 1, rowRulerMK2 - 3 + vOffsetCKSSThree), module, SemiModularSynth::CPMODE_PARAM, 0.0f, 2.0f, 2.0f));	// 0.0f is top position

		
		
		// ****** Gate and slide section ******
		
		static const int rowRulerMB0 = 218;
		static const int columnRulerMBspacing = 70;
		static const int columnRulerMB2 = 134;// Gate2
		static const int columnRulerMB1 = columnRulerMB2 - columnRulerMBspacing;// Gate1 
		static const int columnRulerMB3 = columnRulerMB2 + columnRulerMBspacing;// Tie
		static const int posLEDvsButton = + 25;
		
		// Gate 1 light and button
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(columnRulerMB1 + posLEDvsButton + offsetMediumLight, rowRulerMB0 + 4 + offsetMediumLight), module, SemiModularSynth::GATE1_LIGHT));		
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerMB1 + offsetCKD6b, rowRulerMB0 + 4 + offsetCKD6b), module, SemiModularSynth::GATE1_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Gate 2 light and button
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(columnRulerMB2 + posLEDvsButton + offsetMediumLight, rowRulerMB0 + 4 + offsetMediumLight), module, SemiModularSynth::GATE2_LIGHT));		
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerMB2 + offsetCKD6b, rowRulerMB0 + 4 + offsetCKD6b), module, SemiModularSynth::GATE2_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Tie light and button
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(columnRulerMB3 + posLEDvsButton + offsetMediumLight, rowRulerMB0 + 4 + offsetMediumLight), module, SemiModularSynth::TIE_LIGHT));		
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerMB3 + offsetCKD6b, rowRulerMB0 + 4 + offsetCKD6b), module, SemiModularSynth::TIE_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));

						
		
		// ****** Bottom two rows ******
		
		static const int outputJackSpacingX = 54;
		static const int rowRulerB0 = 327;
		static const int rowRulerB1 = 273;
		static const int columnRulerB0 = 26;
		static const int columnRulerB1 = columnRulerB0 + outputJackSpacingX;
		static const int columnRulerB2 = columnRulerB1 + outputJackSpacingX;
		static const int columnRulerB3 = columnRulerB2 + outputJackSpacingX;
		static const int columnRulerB4 = columnRulerB3 + outputJackSpacingX;
		static const int columnRulerB7 = columnRulerMK2 + 1;
		static const int columnRulerB6 = columnRulerB7 - outputJackSpacingX;
		static const int columnRulerB5 = columnRulerB6 - outputJackSpacingX;

		
		// Gate 1 probability light and button
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(columnRulerB0 + posLEDvsButton + offsetMediumLight, rowRulerB1 + offsetMediumLight), module, SemiModularSynth::GATE1_PROB_LIGHT));		
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerB0 + offsetCKD6b, rowRulerB1 + offsetCKD6b), module, SemiModularSynth::GATE1_PROB_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Gate 1 probability knob
		addParam(createDynamicParam<IMSmallKnob>(Vec(columnRulerB1 + offsetIMSmallKnob, rowRulerB1 + offsetIMSmallKnob), module, SemiModularSynth::GATE1_KNOB_PARAM, 0.0f, 1.0f, 1.0f, &module->portTheme));
		// Slide light and button
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(columnRulerB2 + posLEDvsButton + offsetMediumLight, rowRulerB1 + offsetMediumLight), module, SemiModularSynth::SLIDE_LIGHT));		
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerB2 + offsetCKD6b, rowRulerB1 + offsetCKD6b), module, SemiModularSynth::SLIDE_BTN_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Slide knob
		addParam(createDynamicParam<IMSmallKnob>(Vec(columnRulerB3 + offsetIMSmallKnob, rowRulerB1 + offsetIMSmallKnob), module, SemiModularSynth::SLIDE_KNOB_PARAM, 0.0f, 2.0f, 0.2f, &module->portTheme));
		// Autostep
		addParam(ParamWidget::create<CKSS>(Vec(columnRulerB4 + hOffsetCKSS, rowRulerB1 + vOffsetCKSS), module, SemiModularSynth::AUTOSTEP_PARAM, 0.0f, 1.0f, 1.0f));		
		// CV in
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB5, rowRulerB1), Port::INPUT, module, SemiModularSynth::CV_INPUT, &module->portTheme));
		// Reset
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB6, rowRulerB1), Port::INPUT, module, SemiModularSynth::RESET_INPUT, &module->portTheme));
		// Clock
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB7, rowRulerB1), Port::INPUT, module, SemiModularSynth::CLOCK_INPUT, &module->portTheme));

		

		// ****** Bottom row (all aligned) ******

	
		// CV control Inputs 
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB0, rowRulerB0), Port::INPUT, module, SemiModularSynth::LEFTCV_INPUT, &module->portTheme));
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB1, rowRulerB0), Port::INPUT, module, SemiModularSynth::RIGHTCV_INPUT, &module->portTheme));
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB2, rowRulerB0), Port::INPUT, module, SemiModularSynth::SEQCV_INPUT, &module->portTheme));
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB3, rowRulerB0), Port::INPUT, module, SemiModularSynth::RUNCV_INPUT, &module->portTheme));
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB4, rowRulerB0), Port::INPUT, module, SemiModularSynth::WRITE_INPUT, &module->portTheme));
		// Outputs
		addOutput(createDynamicPort<IMPort>(Vec(columnRulerB5, rowRulerB0), Port::OUTPUT, module, SemiModularSynth::CV_OUTPUT, &module->portTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRulerB6, rowRulerB0), Port::OUTPUT, module, SemiModularSynth::GATE1_OUTPUT, &module->portTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRulerB7, rowRulerB0), Port::OUTPUT, module, SemiModularSynth::GATE2_OUTPUT, &module->portTheme));
		
		
		// VCO
		static const int rowRulerVCO0 = 66;// Freq
		static const int rowRulerVCO1 = rowRulerVCO0 + 40;// Fine, PW
		static const int rowRulerVCO2 = rowRulerVCO1 + 35;// FM, PWM, exact value from svg
		static const int rowRulerVCO3 = rowRulerVCO2 + 46;// switches (Don't change this, tweak the switches' v offset instead since jacks lines up with this)
		static const int rowRulerSpacingJacks = 35;// exact value from svg
		static const int rowRulerVCO4 = rowRulerVCO3 + rowRulerSpacingJacks;// jack row 1
		static const int rowRulerVCO5 = rowRulerVCO4 + rowRulerSpacingJacks;// jack row 2
		static const int rowRulerVCO6 = rowRulerVCO5 + rowRulerSpacingJacks;// jack row 3
		static const int rowRulerVCO7 = rowRulerVCO6 + rowRulerSpacingJacks;// jack row 4
		static const int colRulerVCO0 = 460;
		static const int colRulerVCO1 = colRulerVCO0 + 55;// exact value from svg

		addParam(createDynamicParam<IMBigKnob>(Vec(colRulerVCO0 + offsetIMBigKnob + 55 / 2, rowRulerVCO0 + offsetIMBigKnob), module, SemiModularSynth::VCO_FREQ_PARAM, -54.0f, 54.0f, 0.0f, &module->portTheme));
		
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVCO0 + offsetIMSmallKnob, rowRulerVCO1 + offsetIMSmallKnob), module, SemiModularSynth::VCO_FINE_PARAM, -1.0f, 1.0f, 0.0f, &module->portTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVCO1 + offsetIMSmallKnob, rowRulerVCO1 + offsetIMSmallKnob), module, SemiModularSynth::VCO_PW_PARAM, 0.0f, 1.0f, 0.5f, &module->portTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVCO0 + offsetIMSmallKnob, rowRulerVCO2 + offsetIMSmallKnob), module, SemiModularSynth::VCO_FM_PARAM, 0.0f, 1.0f, 0.0f, &module->portTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVCO1 + offsetIMSmallKnob, rowRulerVCO2 + offsetIMSmallKnob), module, SemiModularSynth::VCO_PWM_PARAM, 0.0f, 1.0f, 0.0f, &module->portTheme));

		addParam(ParamWidget::create<CKSS>(Vec(colRulerVCO0 + hOffsetCKSS, rowRulerVCO3 + vOffsetCKSS), module, SemiModularSynth::VCO_MODE_PARAM, 0.0f, 1.0f, 1.0f));
		addParam(createDynamicParam<IMFivePosSmallKnob>(Vec(colRulerVCO1 + offsetIMSmallKnob, rowRulerVCO3 + offsetIMSmallKnob), module, SemiModularSynth::VCO_OCT_PARAM, -2.0f, 2.0f, 0.0f, &module->portTheme));

		addOutput(createDynamicPort<IMPort>(Vec(colRulerVCO0, rowRulerVCO4), Port::OUTPUT, module, SemiModularSynth::VCO_SIN_OUTPUT, &module->portTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerVCO1, rowRulerVCO4), Port::OUTPUT, module, SemiModularSynth::VCO_TRI_OUTPUT, &module->portTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerVCO0, rowRulerVCO5), Port::OUTPUT, module, SemiModularSynth::VCO_SAW_OUTPUT, &module->portTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerVCO1, rowRulerVCO5), Port::OUTPUT, module, SemiModularSynth::VCO_SQR_OUTPUT, &module->portTheme));		
		
		addInput(createDynamicPort<IMPort>(Vec(colRulerVCO0, rowRulerVCO6), Port::INPUT, module, SemiModularSynth::VCO_PITCH_INPUT, &module->portTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerVCO1, rowRulerVCO6), Port::INPUT, module, SemiModularSynth::VCO_FM_INPUT, &module->portTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerVCO0, rowRulerVCO7), Port::INPUT, module, SemiModularSynth::VCO_SYNC_INPUT, &module->portTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerVCO1, rowRulerVCO7), Port::INPUT, module, SemiModularSynth::VCO_PW_INPUT, &module->portTheme));

		
		// CLK
		static const int rowRulerClk0 = 41;
		static const int rowRulerClk1 = rowRulerClk0 + 45;// exact value from svg
		static const int rowRulerClk2 = rowRulerClk1 + 38;
		static const int colRulerClk0 = colRulerVCO1 + 55;// exact value from svg
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerClk0 + offsetIMSmallKnob, rowRulerClk0 + offsetIMSmallKnob), module, SemiModularSynth::CLK_FREQ_PARAM, -4.0f, 6.0f, 1.0f, &module->portTheme));// 120 BMP when default value
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerClk0 + offsetIMSmallKnob, rowRulerClk1 + offsetIMSmallKnob), module, SemiModularSynth::CLK_PW_PARAM, 0.0f, 1.0f, 0.5f, &module->portTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerClk0, rowRulerClk2), Port::OUTPUT, module, SemiModularSynth::CLK_OUT_OUTPUT, &module->portTheme));
		
		
		// VCA
		static const int colRulerVca1 = colRulerClk0 + 55;// exact value from svg
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerClk0 + offsetIMSmallKnob, rowRulerVCO3 + offsetIMSmallKnob), module, SemiModularSynth::VCA_LEVEL1_PARAM, 0.0f, 1.0f, 1.0f, &module->portTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerClk0, rowRulerVCO4), Port::INPUT, module, SemiModularSynth::VCA_LIN1_INPUT, &module->portTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerClk0, rowRulerVCO5), Port::INPUT, module, SemiModularSynth::VCA_IN1_INPUT, &module->portTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerVca1, rowRulerVCO5), Port::OUTPUT, module, SemiModularSynth::VCA_OUT1_OUTPUT, &module->portTheme));
		
		
		// ADSR
		static const int rowRulerAdsr0 = rowRulerClk0;
		static const int rowRulerAdsr3 = rowRulerVCO2 + 6;
		static const int rowRulerAdsr1 = rowRulerAdsr0 + (rowRulerAdsr3 - rowRulerAdsr0) * 1 / 3;
		static const int rowRulerAdsr2 = rowRulerAdsr0 + (rowRulerAdsr3 - rowRulerAdsr0) * 2 / 3;
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVca1 + offsetIMSmallKnob, rowRulerAdsr0 + offsetIMSmallKnob), module, SemiModularSynth::ADSR_ATTACK_PARAM, 0.0f, 1.0f, 0.1f, &module->portTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVca1 + offsetIMSmallKnob, rowRulerAdsr1 + offsetIMSmallKnob), module, SemiModularSynth::ADSR_DECAY_PARAM,  0.0f, 1.0f, 0.5f, &module->portTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVca1 + offsetIMSmallKnob, rowRulerAdsr2 + offsetIMSmallKnob), module, SemiModularSynth::ADSR_SUSTAIN_PARAM, 0.0f, 1.0f, 0.5f, &module->portTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVca1 + offsetIMSmallKnob, rowRulerAdsr3 + offsetIMSmallKnob), module, SemiModularSynth::ADSR_RELEASE_PARAM, 0.0f, 1.0f, 0.5f, &module->portTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerVca1, rowRulerVCO3), Port::OUTPUT, module, SemiModularSynth::ADSR_ENVELOPE_OUTPUT, &module->portTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerVca1, rowRulerVCO4), Port::INPUT, module, SemiModularSynth::ADSR_GATE_INPUT, &module->portTheme));

		
		// VCF
		static const int colRulerVCF0 = colRulerVca1 + 55;// exact value from svg
		static const int colRulerVCF1 = colRulerVCF0 + 55;// exact value from svg
		addParam(createDynamicParam<IMBigKnob>(Vec(colRulerVCF0 + offsetIMBigKnob + 55 / 2, rowRulerVCO0 + offsetIMBigKnob), module, SemiModularSynth::VCF_FREQ_PARAM, 0.0f, 1.0f, 0.666f, &module->portTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVCF0 + offsetIMSmallKnob + 55 / 2, rowRulerVCO1 + offsetIMSmallKnob), module, SemiModularSynth::VCF_RES_PARAM, 0.0f, 1.0f, 0.0f, &module->portTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVCF0 + offsetIMSmallKnob , rowRulerVCO2 + offsetIMSmallKnob), module, SemiModularSynth::VCF_FREQ_CV_PARAM, -1.0f, 1.0f, 0.0f, &module->portTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVCF1 + offsetIMSmallKnob , rowRulerVCO2 + offsetIMSmallKnob), module, SemiModularSynth::VCF_DRIVE_PARAM, 0.0f, 1.0f, 0.0f, &module->portTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerVCF0, rowRulerVCO3), Port::INPUT, module, SemiModularSynth::VCF_FREQ_INPUT, &module->portTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerVCF1, rowRulerVCO3), Port::INPUT, module, SemiModularSynth::VCF_RES_INPUT, &module->portTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerVCF0, rowRulerVCO4), Port::INPUT, module, SemiModularSynth::VCF_DRIVE_INPUT, &module->portTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerVCF0, rowRulerVCO5), Port::INPUT, module, SemiModularSynth::VCF_IN_INPUT, &module->portTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerVCF1, rowRulerVCO4), Port::OUTPUT, module, SemiModularSynth::VCF_HPF_OUTPUT, &module->portTheme));		
		addOutput(createDynamicPort<IMPort>(Vec(colRulerVCF1, rowRulerVCO5), Port::OUTPUT, module, SemiModularSynth::VCF_LPF_OUTPUT, &module->portTheme));
		
		
		// LFO
		static const int colRulerLfo = colRulerVCF1 + 55;// exact value from svg
		static const int rowRulerLfo0 = rowRulerAdsr0;
		static const int rowRulerLfo2 = rowRulerVCO2;
		static const int rowRulerLfo1 = rowRulerLfo0 + (rowRulerLfo2 - rowRulerLfo0) / 2;
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerLfo + offsetIMSmallKnob, rowRulerLfo0 + offsetIMSmallKnob), module, SemiModularSynth::LFO_FREQ_PARAM, -8.0f, 6.0f, -1.0f, &module->portTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerLfo + offsetIMSmallKnob, rowRulerLfo1 + offsetIMSmallKnob), module, SemiModularSynth::LFO_GAIN_PARAM, 0.0f, 1.0f, 0.5f, &module->portTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerLfo + offsetIMSmallKnob, rowRulerLfo2 + offsetIMSmallKnob), module, SemiModularSynth::LFO_OFFSET_PARAM, -1.0f, 1.0f, 0.0f, &module->portTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerLfo, rowRulerVCO3), Port::OUTPUT, module, SemiModularSynth::LFO_TRI_OUTPUT, &module->portTheme));		
		addOutput(createDynamicPort<IMPort>(Vec(colRulerLfo, rowRulerVCO4), Port::OUTPUT, module, SemiModularSynth::LFO_SIN_OUTPUT, &module->portTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerLfo, rowRulerVCO5), Port::INPUT, module, SemiModularSynth::LFO_RESET_INPUT, &module->portTheme));
	}
};

} // namespace rack_plugin_ImpromptuModular

using namespace rack_plugin_ImpromptuModular;

RACK_PLUGIN_MODEL_INIT(ImpromptuModular, SemiModularSynth) {
   Model *modelSemiModularSynth = Model::create<SemiModularSynth, SemiModularSynthWidget>("Impromptu Modular", "Semi-Modular Synth", "MISC - Semi-Modular Synth", SEQUENCER_TAG, OSCILLATOR_TAG);
   return modelSemiModularSynth;
}

/*CHANGE LOG

0.6.10:
allow main knob to also change length when length editing is active

0.6.9:
add FW2, FW3 and FW4 run modes for sequences (but not for song)
update VCF code to match new Fundamental code (existing patches may sound different)
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
dark by default when create new instance of module

0.6.5:
initial release of SMS
*/
