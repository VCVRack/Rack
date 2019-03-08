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


#include "FundamentalUtil.hpp"
#include "PhraseSeqUtil.hpp"
namespace rack_plugin_ImpromptuModular {

struct SemiModularSynth : Module {
	enum ParamIds {
		// SEQUENCER
		KEYNOTE_PARAM,// 0.6.12 replaces unused
		KEYGATE_PARAM,// 0.6.12 replaces unused
		KEYGATE2_PARAM,// no longer used
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
		
		// ALL NEW
		// -- 0.6.10 ^^
		ENUMS(STEP_PHRASE_PARAMS, 16),
				
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
		ENUMS(STEP_PHRASE_LIGHTS, 16 * 3),// room for GreenRedWhite
		ENUMS(OCTAVE_LIGHTS, 7),// octaves 1 to 7
		ENUMS(KEY_LIGHTS, 12 * 2),// room for GreenRed
		RUN_LIGHT,
		RESET_LIGHT,
		ENUMS(GATE1_LIGHT, 2),// room for GreenRed
		ENUMS(GATE2_LIGHT, 2),// room for GreenRed
		SLIDE_LIGHT,
		ATTACH_LIGHT,
		ENUMS(GATE1_PROB_LIGHT, 2),// room for GreenRed
		TIE_LIGHT,
		KEYNOTE_LIGHT,
		ENUMS(KEYGATE_LIGHT, 2),// room for GreenRed
		
		// VCO, CLK, VCA
		// none
		
		NUM_LIGHTS
	};

	
	// SEQUENCER
	
	// Constants
	enum DisplayStateIds {DISP_NORMAL, DISP_MODE, DISP_LENGTH, DISP_TRANSPOSE, DISP_ROTATE};

	// Need to save
	int panelTheme = 2;
	bool autoseq;
	bool autostepLen;
	bool holdTiedNotes;
	int pulsesPerStep;// 1 means normal gate mode, alt choices are 4, 6, 12, 24 PPS (Pulses per step)
	bool running;
	SeqAttributes sequences[16];
	int runModeSong; 
	int sequence;
	int phrase[16];// This is the song (series of phases; a phrase is a patten number)
	int phrases;//1 to 16
	float cv[16][16];// [-3.0 : 3.917]. First index is patten number, 2nd index is step
	StepAttributes attributes[16][16];// First index is patten number, 2nd index is step (see enum AttributeBitMasks for details)
	bool resetOnRun;
	bool attached;

	// No need to save
	int stepIndexEdit;
	int stepIndexRun;
	int phraseIndexEdit;
	int phraseIndexRun;
	long infoCopyPaste;// 0 when no info, positive downward step counter timer when copy, negative upward when paste
	unsigned long editingGate;// 0 when no edit gate, downward step counter timer when edit gate
	float editingGateCV;// no need to initialize, this is a companion to editingGate (output this only when editingGate > 0)
	int editingGateKeyLight;// no need to initialize, this is a companion to editingGate (use this only when editingGate > 0)
	unsigned long editingType;// similar to editingGate, but just for showing remanent gate type (nothing played); uses editingGateKeyLight
	unsigned long stepIndexRunHistory;
	unsigned long phraseIndexRunHistory;
	int displayState;
	unsigned long slideStepsRemain;// 0 when no slide under way, downward step counter when sliding
	float slideCVdelta;// no need to initialize, this is a companion to slideStepsRemain
	float cvCPbuffer[16];// copy paste buffer for CVs
	StepAttributes attribCPbuffer[16];
	SeqAttributes seqAttribCPbuffer;
	bool seqCopied;
	int phraseCPbuffer[16];
	int countCP;// number of steps to paste (in case CPMODE_PARAM changes between copy and paste)
	int startCP;
	long clockIgnoreOnReset;
	unsigned long clockPeriod;// counts number of step() calls upward from last clock (reset after clock processed)
	long tiedWarning;// 0 when no warning, positive downward step counter timer when warning
	long attachedWarning;// 0 when no warning, positive downward step counter timer when warning
	int gate1Code;
	int gate2Code;
	long revertDisplay;
	long editingGateLength;// 0 when no info, positive when gate1, negative when gate2
	long lastGateEdit;
	long editingPpqn;// 0 when no info, positive downward step counter timer when editing ppqn
	int ppqnCount;
	
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
	

	unsigned int lightRefreshCounter = 0;
	float resetLight = 0.0f;
	int sequenceKnob = 0;
	Trigger resetTrigger;
	Trigger leftTrigger;
	Trigger rightTrigger;
	Trigger runningTrigger;
	Trigger clockTrigger;
	Trigger octTriggers[7];
	Trigger octmTrigger;
	Trigger gate1Trigger;
	Trigger gate1ProbTrigger;
	Trigger gate2Trigger;
	Trigger slideTrigger;
	Trigger keyTriggers[12];
	Trigger writeTrigger;
	Trigger attachedTrigger;
	Trigger copyTrigger;
	Trigger pasteTrigger;
	Trigger modeTrigger;
	Trigger rotateTrigger;
	Trigger transposeTrigger;
	Trigger tiedTrigger;
	Trigger stepTriggers[16];
	Trigger keyNoteTrigger;
	Trigger keyGateTrigger;
	HoldDetect modeHoldDetect;
	
	
	inline bool isEditingSequence(void) {return params[EDIT_PARAM].value > 0.5f;}
	
	
	LowFrequencyOscillator oscillatorClk;
	LowFrequencyOscillator oscillatorLfo;
	VoltageControlledOscillator oscillatorVco;


	SemiModularSynth() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
		
		// VCO
		oscillatorVco.soft = false;//params[VCO_SYNC_PARAM].value <= 0.0f;
		
		// CLK 
		oscillatorClk.offset = true;//(params[OFFSET_PARAM].value > 0.0f);
		oscillatorClk.invert = false;//(params[INVERT_PARAM].value <= 0.0f);
		
		// LFO
		oscillatorLfo.setPulseWidth(0.5f);//params[PW_PARAM].value + params[PWM_PARAM].value * inputs[PW_INPUT].value / 10.0f);
		oscillatorLfo.offset = false;//(params[OFFSET_PARAM].value > 0.0f);
		oscillatorLfo.invert = false;//(params[INVERT_PARAM].value <= 0.0f);
	}
	

	void onReset() override {
		// SEQUENCER
		autoseq = false;
		autostepLen = false;
		holdTiedNotes = true;
		pulsesPerStep = 1;
		running = true;
		runModeSong = MODE_FWD;
		stepIndexEdit = 0;
		phraseIndexEdit = 0;
		sequence = 0;
		phrases = 4;
		for (int i = 0; i < 16; i++) {
			for (int s = 0; s < 16; s++) {
				cv[i][s] = 0.0f;
				attributes[i][s].init();
			}
			sequences[i].init(16, MODE_FWD);
			phrase[i] = 0;
			cvCPbuffer[i] = 0.0f;
			attribCPbuffer[i].init();
			phraseCPbuffer[i] = 0;
		}
		initRun();
		seqAttribCPbuffer.init(16, MODE_FWD);
		seqCopied = true;
		countCP = 16;
		startCP = 0;
		editingGate = 0ul;
		editingType = 0ul;
		infoCopyPaste = 0l;
		displayState = DISP_NORMAL;
		slideStepsRemain = 0ul;
		attached = false;
		clockPeriod = 0ul;
		tiedWarning = 0ul;
		attachedWarning = 0l;
		revertDisplay = 0l;
		resetOnRun = false;
		editingGateLength = 0l;
		lastGateEdit = 1l;
		editingPpqn = 0l;
		
		// VCO
		// none
		
		// CLK
		clkValue = 0.0f;
		
		// VCF
		filter.reset();
	}

	
	void onRandomize() override {
		if (isEditingSequence()) {
			for (int s = 0; s < 16; s++) {
				cv[sequence][s] = ((float)(randomu32() % 7)) + ((float)(randomu32() % 12)) / 12.0f - 3.0f;
				attributes[sequence][s].randomize();
				// if (attributes[sequence][s].getTied()) {
					// activateTiedStep(sequence, s);
				// }
			}
			sequences[sequence].randomize(16, NUM_MODES - 1);
		}
	}
	
	
	void initRun() {// run button activated or run edge in run input jack
		phraseIndexRun = (runModeSong == MODE_REV ? phrases - 1 : 0);
		phraseIndexRunHistory = 0;

		int seq = (isEditingSequence() ? sequence : phrase[phraseIndexRun]);
		stepIndexRun = (sequences[seq].getRunMode() == MODE_REV ? sequences[seq].getLength() - 1 : 0);
		stepIndexRunHistory = 0;

		ppqnCount = 0;
		gate1Code = calcGate1Code(attributes[seq][stepIndexRun], 0, pulsesPerStep, params[GATE1_KNOB_PARAM].value);
		gate2Code = calcGate2Code(attributes[seq][stepIndexRun], 0, pulsesPerStep);
		slideStepsRemain = 0ul;
		clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * engineGetSampleRate());
	}
	
	
	json_t *toJson() override {
		json_t *rootJ = json_object();

		// panelTheme
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));

		// autostepLen
		json_object_set_new(rootJ, "autostepLen", json_boolean(autostepLen));
		
		// autoseq
		json_object_set_new(rootJ, "autoseq", json_boolean(autoseq));
		
		// holdTiedNotes
		json_object_set_new(rootJ, "holdTiedNotes", json_boolean(holdTiedNotes));
		
		// pulsesPerStep
		json_object_set_new(rootJ, "pulsesPerStep", json_integer(pulsesPerStep));

		// running
		json_object_set_new(rootJ, "running", json_boolean(running));
		
		// runModeSong
		json_object_set_new(rootJ, "runModeSong3", json_integer(runModeSong));

		// sequence
		json_object_set_new(rootJ, "sequence", json_integer(sequence));

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
				json_array_insert_new(attributesJ, s + (i * 16), json_integer(attributes[i][s].getAttribute()));
			}
		json_object_set_new(rootJ, "attributes", attributesJ);

		// attached
		json_object_set_new(rootJ, "attached", json_boolean(attached));

		// resetOnRun
		json_object_set_new(rootJ, "resetOnRun", json_boolean(resetOnRun));
		
		// stepIndexEdit
		json_object_set_new(rootJ, "stepIndexEdit", json_integer(stepIndexEdit));
	
		// phraseIndexEdit
		json_object_set_new(rootJ, "phraseIndexEdit", json_integer(phraseIndexEdit));

		// sequences
		json_t *sequencesJ = json_array();
		for (int i = 0; i < 16; i++)
			json_array_insert_new(sequencesJ, i, json_integer(sequences[i].getSeqAttrib()));
		json_object_set_new(rootJ, "sequences", sequencesJ);
		
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// panelTheme
		json_t *panelThemeJ = json_object_get(rootJ, "panelTheme");
		if (panelThemeJ)
			panelTheme = json_integer_value(panelThemeJ);

		// autostepLen
		json_t *autostepLenJ = json_object_get(rootJ, "autostepLen");
		if (autostepLenJ)
			autostepLen = json_is_true(autostepLenJ);

		// autoseq
		json_t *autoseqJ = json_object_get(rootJ, "autoseq");
		if (autoseqJ)
			autoseq = json_is_true(autoseqJ);

		// holdTiedNotes
		json_t *holdTiedNotesJ = json_object_get(rootJ, "holdTiedNotes");
		if (holdTiedNotesJ)
			holdTiedNotes = json_is_true(holdTiedNotesJ);
		else
			holdTiedNotes = false;// legacy
		
		// pulsesPerStep
		json_t *pulsesPerStepJ = json_object_get(rootJ, "pulsesPerStep");
		if (pulsesPerStepJ)
			pulsesPerStep = json_integer_value(pulsesPerStepJ);

		// running
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);

		
		// sequences
		json_t *sequencesJ = json_object_get(rootJ, "sequences");
		if (sequencesJ) {
			for (int i = 0; i < 16; i++)
			{
				json_t *sequencesArrayJ = json_array_get(sequencesJ, i);
				if (sequencesArrayJ)
					sequences[i].setSeqAttrib(json_integer_value(sequencesArrayJ));
			}			
		}
		else {// legacy
			int lengths[16];//1 to 16
			int runModeSeq[16]; 
			int transposeOffsets[16];

		
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
		
			// runModeSeq
			json_t *runModeSeqJ = json_object_get(rootJ, "runModeSeq3");
			if (runModeSeqJ) {
				for (int i = 0; i < 16; i++)
				{
					json_t *runModeSeqArrayJ = json_array_get(runModeSeqJ, i);
					if (runModeSeqArrayJ)
						runModeSeq[i] = json_integer_value(runModeSeqArrayJ);
				}			
			}		
			else {// legacy
				runModeSeqJ = json_object_get(rootJ, "runModeSeq2");
				if (runModeSeqJ) {
					for (int i = 0; i < 16; i++)
					{
						json_t *runModeSeqArrayJ = json_array_get(runModeSeqJ, i);
						if (runModeSeqArrayJ) {
							runModeSeq[i] = json_integer_value(runModeSeqArrayJ);
							if (runModeSeq[i] >= MODE_PEN)// this mode was not present in version runModeSeq2
								runModeSeq[i]++;
						}
					}			
				}		
			}
		
			// transposeOffsets
			json_t *transposeOffsetsJ = json_object_get(rootJ, "transposeOffsets");
			if (transposeOffsetsJ) {
				for (int i = 0; i < 16; i++)
				{
					json_t *transposeOffsetsArrayJ = json_array_get(transposeOffsetsJ, i);
					if (transposeOffsetsArrayJ)
						transposeOffsets[i] = json_integer_value(transposeOffsetsArrayJ);
				}			
			}
			
			// now write into new object
			for (int i = 0; i < 16; i++) {
				sequences[i].init(lengths[i], runModeSeq[i]);
				sequences[i].setTranspose(transposeOffsets[i]);
			}
		}

		// runModeSong
		json_t *runModeSongJ = json_object_get(rootJ, "runModeSong3");
		if (runModeSongJ)
			runModeSong = json_integer_value(runModeSongJ);
		else {// legacy
			runModeSongJ = json_object_get(rootJ, "runModeSong");
			if (runModeSongJ) {
				runModeSong = json_integer_value(runModeSongJ);
				if (runModeSong >= MODE_PEN)// this mode was not present in original version
					runModeSong++;
			}
		}
		
		// sequence
		json_t *sequenceJ = json_object_get(rootJ, "sequence");
		if (sequenceJ)
			sequence = json_integer_value(sequenceJ);
		
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
						cv[i][s] = json_number_value(cvArrayJ);
				}
		}

		// attributes
		json_t *attributesJ = json_object_get(rootJ, "attributes");
		if (attributesJ) {
			for (int i = 0; i < 16; i++)
				for (int s = 0; s < 16; s++) {
					json_t *attributesArrayJ = json_array_get(attributesJ, s + (i * 16));
					if (attributesArrayJ)
						attributes[i][s].setAttribute((unsigned short)json_integer_value(attributesArrayJ));
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

		// stepIndexEdit
		json_t *stepIndexEditJ = json_object_get(rootJ, "stepIndexEdit");
		if (stepIndexEditJ)
			stepIndexEdit = json_integer_value(stepIndexEditJ);
		
		// phraseIndexEdit
		json_t *phraseIndexEditJ = json_object_get(rootJ, "phraseIndexEdit");
		if (phraseIndexEditJ)
			phraseIndexEdit = json_integer_value(phraseIndexEditJ);
		
		// Initialize dependants after everything loaded
		initRun();
	}


	void rotateSeq(int seqNum, bool directionRight, int seqLength) {
		float rotCV;
		StepAttributes rotAttributes;
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
	

	void step() override {
		float sampleRate = engineGetSampleRate();
	
		// SEQUENCER

		static const float gateTime = 0.4f;// seconds
		static const float revertDisplayTime = 0.7f;// seconds
		static const float warningTime = 0.7f;// seconds
		static const float holdDetectTime = 2.0f;// seconds
		static const float editGateLengthTime = 3.5f;// seconds
		
		
		//********** Buttons, knobs, switches and inputs **********
		
		// Edit mode
		bool editingSequence = isEditingSequence();// true = editing sequence, false = editing song
		
		// Run button
		if (runningTrigger.process(params[RUN_PARAM].value + inputs[RUNCV_INPUT].value)) {// no input refresh here, don't want to introduce startup skew
			running = !running;
			if (running) {
				if (resetOnRun)
					initRun();
				//if (resetOnRun || clockIgnoreOnRun)
					clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * sampleRate);// always keep this since CLK gets reset when run turned on
			}
			displayState = DISP_NORMAL;
		}

		if ((lightRefreshCounter & userInputsStepSkipMask) == 0) {

			// Seq CV input
			if (inputs[SEQCV_INPUT].active) {
				sequence = (int) clamp( round(inputs[SEQCV_INPUT].value * (16.0f - 1.0f) / 10.0f), 0.0f, (16.0f - 1.0f) );
			}
			
			// Attach button
			if (attachedTrigger.process(params[ATTACH_PARAM].value)) {
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
				startCP = editingSequence ? stepIndexEdit : phraseIndexEdit;
				countCP = 16;
				if (params[CPMODE_PARAM].value > 1.5f)// all
					startCP = 0;
				else if (params[CPMODE_PARAM].value < 0.5f)// 4
					countCP = min(4, 16 - startCP);
				else// 8
					countCP = min(8, 16 - startCP);
				if (editingSequence) {
					for (int i = 0, s = startCP; i < countCP; i++, s++) {
						cvCPbuffer[i] = cv[sequence][s];
						attribCPbuffer[i] = attributes[sequence][s];
					}
					seqAttribCPbuffer.setSeqAttrib(sequences[sequence].getSeqAttrib());
					seqCopied = true;
				}
				else {
					for (int i = 0, p = startCP; i < countCP; i++, p++)
						phraseCPbuffer[i] = phrase[p];
					seqCopied = false;// so that a cross paste can be detected
				}
				infoCopyPaste = (long) (revertDisplayTime * sampleRate / displayRefreshStepSkips);
				displayState = DISP_NORMAL;
			}
			// Paste button
			if (pasteTrigger.process(params[PASTE_PARAM].value)) {
				infoCopyPaste = (long) (-1 * revertDisplayTime * sampleRate / displayRefreshStepSkips);
				startCP = 0;
				if (countCP <= 8) {
					startCP = editingSequence ? stepIndexEdit : phraseIndexEdit;
					countCP = min(countCP, 16 - startCP);
				}
				// else nothing to do for ALL

				if (editingSequence) {
					if (seqCopied) {// non-crossed paste (seq vs song)
						for (int i = 0, s = startCP; i < countCP; i++, s++) {
							cv[sequence][s] = cvCPbuffer[i];
							attributes[sequence][s] = attribCPbuffer[i];
						}
						if (params[CPMODE_PARAM].value > 1.5f) {// all
							sequences[sequence].setSeqAttrib(seqAttribCPbuffer.getSeqAttrib());
						}
					}
					else {// crossed paste to seq (seq vs song)
						if (params[CPMODE_PARAM].value > 1.5f) { // ALL (init steps)
							for (int s = 0; s < 16; s++) {
								//cv[sequence][s] = 0.0f;
								//attributes[sequence][s].init();
								attributes[sequence][s].toggleGate1();
							}
							sequences[sequence].setTranspose(0);
							sequences[sequence].setRotate(0);
						}
						else if (params[CPMODE_PARAM].value < 0.5f) {// 4 (randomize CVs)
							for (int s = 0; s < 16; s++)
								cv[sequence][s] = ((float)(randomu32() % 7)) + ((float)(randomu32() % 12)) / 12.0f - 3.0f;
							sequences[sequence].setTranspose(0);
							sequences[sequence].setRotate(0);
						}
						else {// 8 (randomize gate 1)
							for (int s = 0; s < 16; s++)
								if ( (randomu32() & 0x1) != 0)
									attributes[sequence][s].toggleGate1();
						}
						startCP = 0;
						countCP = 16;
						infoCopyPaste *= 2l;
					}
				}
				else {
					if (!seqCopied) {// non-crossed paste (seq vs song)
						for (int i = 0, p = startCP; i < countCP; i++, p++)
							phrase[p] = phraseCPbuffer[i];
					}
					else {// crossed paste to song (seq vs song)
						if (params[CPMODE_PARAM].value > 1.5f) { // ALL (init phrases)
							for (int p = 0; p < 16; p++)
								phrase[p] = 0;
						}
						else if (params[CPMODE_PARAM].value < 0.5f) {// 4 (phrases increase from 1 to 16)
							for (int p = 0; p < 16; p++)
								phrase[p] = p;						
						}
						else {// 8 (randomize phrases)
							for (int p = 0; p < 16; p++)
								phrase[p] = randomu32() % 16;
						}
						startCP = 0;
						countCP = 16;
						infoCopyPaste *= 2l;
					}					
				}
				displayState = DISP_NORMAL;
			}
			
			// Write input (must be before Left and Right in case route gate simultaneously to Right and Write for example)
			//  (write must be to correct step)
			bool writeTrig = writeTrigger.process(inputs[WRITE_INPUT].value);
			if (writeTrig) {
				if (editingSequence) {
					if (!attributes[sequence][stepIndexEdit].getTied()) {
						cv[sequence][stepIndexEdit] = inputs[CV_INPUT].value;
						propagateCVtoTied(sequence, stepIndexEdit);
					}
					editingGate = (unsigned long) (gateTime * sampleRate / displayRefreshStepSkips);
					editingGateCV = inputs[CV_INPUT].value;// cv[sequence][stepIndexEdit];
					editingGateKeyLight = -1;
					// Autostep (after grab all active inputs)
					if (params[AUTOSTEP_PARAM].value > 0.5f) {
						stepIndexEdit = moveIndex(stepIndexEdit, stepIndexEdit + 1, autostepLen ? sequences[sequence].getLength() : 16);
						if (stepIndexEdit == 0 && autoseq)
							sequence = moveIndex(sequence, sequence + 1, 16);
					}
				}
				displayState = DISP_NORMAL;
			}
			// Left and Right CV inputs
			int delta = 0;
			if (leftTrigger.process(inputs[LEFTCV_INPUT].value)) { 
				delta = -1;
				if (displayState != DISP_LENGTH)
					displayState = DISP_NORMAL;
			}
			if (rightTrigger.process(inputs[RIGHTCV_INPUT].value)) {
				delta = +1;
				if (displayState != DISP_LENGTH)
					displayState = DISP_NORMAL;
			}
			if (delta != 0) {
				if (displayState == DISP_LENGTH) {
					if (editingSequence) {
						sequences[sequence].setLength(clamp(sequences[sequence].getLength() + delta, 1, 16));
					}
					else {
						phrases = clamp(phrases + delta, 1, 16);
					}
				}
				else {
					if (!running || !attached) {// don't move heads when attach and running
						if (editingSequence) {
							stepIndexEdit = moveIndex(stepIndexEdit, stepIndexEdit + delta, 16);
							if (!attributes[sequence][stepIndexEdit].getTied()) {// play if non-tied step
								if (!writeTrig) {// in case autostep when simultaneous writeCV and stepCV (keep what was done in Write Input block above)
									editingGate = (unsigned long) (gateTime * sampleRate / displayRefreshStepSkips);
									editingGateCV = cv[sequence][stepIndexEdit];
									editingGateKeyLight = -1;
								}
							}
						}
						else {
							phraseIndexEdit = moveIndex(phraseIndexEdit, phraseIndexEdit + delta, 16);
							if (!running)
								phraseIndexRun = phraseIndexEdit;
						}
					}
				}
			}

			// Step button presses
			int stepPressed = -1;
			for (int i = 0; i < 16; i++) {
				if (stepTriggers[i].process(params[STEP_PHRASE_PARAMS + i].value))
					stepPressed = i;
			}
			if (stepPressed != -1) {
				if (displayState == DISP_LENGTH) {
					if (editingSequence)
						sequences[sequence].setLength(stepPressed + 1);
					else
						phrases = stepPressed + 1;
					revertDisplay = (long) (revertDisplayTime * sampleRate / displayRefreshStepSkips);
				}
				else {
					if (!running || !attached) {// not running or detached
						if (editingSequence) {
							stepIndexEdit = stepPressed;
							if (!attributes[sequence][stepIndexEdit].getTied()) {// play if non-tied step
								editingGate = (unsigned long) (gateTime * sampleRate / displayRefreshStepSkips);
								editingGateCV = cv[sequence][stepIndexEdit];
								editingGateKeyLight = -1;
							}
						}
						else {
							phraseIndexEdit = stepPressed;
							if (!running)
								phraseIndexRun = stepPressed;
						}
					}
					else if (attached)
						attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
					displayState = DISP_NORMAL;
				}
			} 
			
			// Mode/Length button
			if (modeTrigger.process(params[RUNMODE_PARAM].value)) {
				if (editingPpqn != 0l)
					editingPpqn = 0l;			
				if (displayState == DISP_NORMAL || displayState == DISP_TRANSPOSE || displayState == DISP_ROTATE)
					displayState = DISP_LENGTH;
				else if (displayState == DISP_LENGTH)
					displayState = DISP_MODE;
				else
					displayState = DISP_NORMAL;
				modeHoldDetect.start((long) (holdDetectTime * sampleRate / displayRefreshStepSkips));
			}
			
			// Transpose/Rotate button
			if (transposeTrigger.process(params[TRAN_ROT_PARAM].value)) {
				if (editingSequence) {
					if (displayState == DISP_NORMAL || displayState == DISP_MODE || displayState == DISP_LENGTH) {
						displayState = DISP_TRANSPOSE;
					}
					else if (displayState == DISP_TRANSPOSE) {
						displayState = DISP_ROTATE;
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
					// any changes in here should may also require right click behavior to be updated in the knob's onMouseDown()
					if (editingPpqn != 0) {
						pulsesPerStep = indexToPps(ppsToIndex(pulsesPerStep) + deltaKnob);// indexToPps() does clamping
						editingPpqn = (long) (editGateLengthTime * sampleRate / displayRefreshStepSkips);
					}
					else if (displayState == DISP_MODE) {
						if (editingSequence) {
							sequences[sequence].setRunMode(clamp(sequences[sequence].getRunMode() + deltaKnob, 0, (NUM_MODES - 1 - 1)));
						}
						else {
							runModeSong = clamp(runModeSong + deltaKnob, 0, 6 - 1);
						}
					}
					else if (displayState == DISP_LENGTH) {
						if (editingSequence) {
							sequences[sequence].setLength(clamp(sequences[sequence].getLength() + deltaKnob, 1, 16));
						}
						else {
							phrases = clamp(phrases + deltaKnob, 1, 16);
						}
					}
					else if (displayState == DISP_TRANSPOSE) {
						if (editingSequence) {
							sequences[sequence].setTranspose(clamp(sequences[sequence].getTranspose() + deltaKnob, -99, 99));
							float transposeOffsetCV = ((float)(deltaKnob))/12.0f;// Tranpose by deltaKnob number of semi-tones
							for (int s = 0; s < 16; s++) {
								cv[sequence][s] += transposeOffsetCV;
							}
						}
					}
					else if (displayState == DISP_ROTATE) {
						if (editingSequence) {
							int slength = sequences[sequence].getLength();
							sequences[sequence].setRotate(clamp(sequences[sequence].getRotate() + deltaKnob, -99, 99));
							if (deltaKnob > 0 && deltaKnob < 201) {// Rotate right, 201 is safety
								for (int i = deltaKnob; i > 0; i--) {
									rotateSeq(sequence, true, slength);
									if (stepIndexEdit < slength)
										stepIndexEdit = moveIndex(stepIndexEdit, stepIndexEdit + 1, slength);
								}
							}
							if (deltaKnob < 0 && deltaKnob > -201) {// Rotate left, 201 is safety
								for (int i = deltaKnob; i < 0; i++) {
									rotateSeq(sequence, false, slength);
									if (stepIndexEdit < slength)
										stepIndexEdit = moveIndex(stepIndexEdit, stepIndexEdit - 1, slength);
								}
							}
						}						
					}
					else {// DISP_NORMAL
						if (editingSequence) {
							if (!inputs[SEQCV_INPUT].active) {
								sequence = clamp(sequence + deltaKnob, 0, 16 - 1);
							}
						}
						else {
							if (!attached || (attached && !running))
								phrase[phraseIndexEdit] = clamp(phrase[phraseIndexEdit] + deltaKnob, 0, 16 - 1);
							else
								attachedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
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
					if (attributes[sequence][stepIndexEdit].getTied())
						tiedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
					else {			
						cv[sequence][stepIndexEdit] = applyNewOct(cv[sequence][stepIndexEdit], newOct);
						propagateCVtoTied(sequence, stepIndexEdit);
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
							if (newMode != -1) {
								editingPpqn = 0l;
								attributes[sequence][stepIndexEdit].setGateMode(newMode, editingGateLength > 0l);
								if (params[KEY_PARAMS + i].value > 1.5f) {// if right-click
									stepIndexEdit = moveIndex(stepIndexEdit, stepIndexEdit + 1, 16);
									editingType = (unsigned long) (gateTime * sampleRate / displayRefreshStepSkips);
									editingGateKeyLight = i;
									if (windowIsModPressed())
										attributes[sequence][stepIndexEdit].setGateMode(newMode, editingGateLength > 0l);
								}
							}
							else
								editingPpqn = (long) (editGateLengthTime * sampleRate / displayRefreshStepSkips);
						}
						else if (attributes[sequence][stepIndexEdit].getTied()) {
							if (params[KEY_PARAMS + i].value > 1.5f)// if right-click
								stepIndexEdit = moveIndex(stepIndexEdit, stepIndexEdit + 1, 16);
							else
								tiedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
						}
						else {			
							float newCV = floor(cv[sequence][stepIndexEdit]) + ((float) i) / 12.0f;
							cv[sequence][stepIndexEdit] = newCV;
							propagateCVtoTied(sequence, stepIndexEdit);
							editingGate = (unsigned long) (gateTime * sampleRate / displayRefreshStepSkips);
							editingGateCV = cv[sequence][stepIndexEdit];
							editingGateKeyLight = -1;
							if (params[KEY_PARAMS + i].value > 1.5f) {// if right-click
								stepIndexEdit = moveIndex(stepIndexEdit, stepIndexEdit + 1, 16);
								editingGateKeyLight = i;
								if (windowIsModPressed())
									cv[sequence][stepIndexEdit] = newCV;
							}
						}						
					}
					displayState = DISP_NORMAL;
				}
			}
			
			// Keyboard mode (note or gate type)
			if (keyNoteTrigger.process(params[KEYNOTE_PARAM].value)) {
				editingGateLength = 0l;
			}
			if (keyGateTrigger.process(params[KEYGATE_PARAM].value)) {
				if (editingGateLength == 0l) {
					editingGateLength = lastGateEdit;
				}
				else {
					editingGateLength *= -1l;
					lastGateEdit = editingGateLength;
				}
			}

			// Gate1, Gate1Prob, Gate2, Slide and Tied buttons
			if (gate1Trigger.process(params[GATE1_PARAM].value)) {
				if (editingSequence) {
					attributes[sequence][stepIndexEdit].toggleGate1();
				}
				displayState = DISP_NORMAL;
			}		
			if (gate1ProbTrigger.process(params[GATE1_PROB_PARAM].value)) {
				if (editingSequence) {
					if (attributes[sequence][stepIndexEdit].getTied())
						tiedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
					else
						attributes[sequence][stepIndexEdit].toggleGate1P();
				}
				displayState = DISP_NORMAL;
			}		
			if (gate2Trigger.process(params[GATE2_PARAM].value)) {
				if (editingSequence) {
					attributes[sequence][stepIndexEdit].toggleGate2();
				}
				displayState = DISP_NORMAL;
			}		
			if (slideTrigger.process(params[SLIDE_BTN_PARAM].value)) {
				if (editingSequence) {
					if (attributes[sequence][stepIndexEdit].getTied())
						tiedWarning = (long) (warningTime * sampleRate / displayRefreshStepSkips);
					else
						attributes[sequence][stepIndexEdit].toggleSlide();
				}
				displayState = DISP_NORMAL;
			}		
			if (tiedTrigger.process(params[TIE_PARAM].value)) {
				if (editingSequence) {
					if (attributes[sequence][stepIndexEdit].getTied()) {
						deactivateTiedStep(sequence, stepIndexEdit);
					}
					else {
						activateTiedStep(sequence, stepIndexEdit);
					}
				}
				displayState = DISP_NORMAL;
			}		
		
		}// userInputs refresh


		
		//********** Clock and reset **********
		
		// Clock
		float clockInput = inputs[CLOCK_INPUT].active ? inputs[CLOCK_INPUT].value : clkValue;// Pre-patching
		if (running && clockIgnoreOnReset == 0l) {
			if (clockTrigger.process(clockInput)) {
				ppqnCount++;
				if (ppqnCount >= pulsesPerStep)
					ppqnCount = 0;

				int newSeq = sequence;// good value when editingSequence, overwrite if not editingSequence
				if (ppqnCount == 0) {
					float slideFromCV = 0.0f;
					if (editingSequence) {
						slideFromCV = cv[sequence][stepIndexRun];
						moveIndexRunMode(&stepIndexRun, sequences[sequence].getLength(), sequences[sequence].getRunMode(), &stepIndexRunHistory);
					}
					else {
						slideFromCV = cv[phrase[phraseIndexRun]][stepIndexRun];
						if (moveIndexRunMode(&stepIndexRun, sequences[phrase[phraseIndexRun]].getLength(), sequences[phrase[phraseIndexRun]].getRunMode(), &stepIndexRunHistory)) {
							moveIndexRunMode(&phraseIndexRun, phrases, runModeSong, &phraseIndexRunHistory);
							stepIndexRun = (sequences[phrase[phraseIndexRun]].getRunMode() == MODE_REV ? sequences[phrase[phraseIndexRun]].getLength() - 1 : 0);// must always refresh after phraseIndexRun has changed
						}
						newSeq = phrase[phraseIndexRun];
					}
					
					// Slide
					if (attributes[newSeq][stepIndexRun].getSlide()) {
						slideStepsRemain =   (unsigned long) (((float)clockPeriod * pulsesPerStep) * params[SLIDE_KNOB_PARAM].value / 2.0f);
						if (slideStepsRemain != 0ul) {
							float slideToCV = cv[newSeq][stepIndexRun];
							slideCVdelta = (slideToCV - slideFromCV)/(float)slideStepsRemain;
						}
					}
					else 
						slideStepsRemain = 0ul;
				}
				else {
					if (!editingSequence)
						newSeq = phrase[phraseIndexRun];
				}
				if (gate1Code != -1 || ppqnCount == 0)
					gate1Code = calcGate1Code(attributes[newSeq][stepIndexRun], ppqnCount, pulsesPerStep, params[GATE1_KNOB_PARAM].value);
				gate2Code = calcGate2Code(attributes[newSeq][stepIndexRun], ppqnCount, pulsesPerStep);
				clockPeriod = 0ul;				
			}
			clockPeriod++;
		}	
		
		// Reset
		if (resetTrigger.process(inputs[RESET_INPUT].value + params[RESET_PARAM].value)) {
			initRun();// must be after sequence reset
			resetLight = 1.0f;
			displayState = DISP_NORMAL;
		}
		
		
		//********** Outputs and lights **********
				
		// CV and gates outputs
		int seq = editingSequence ? (sequence) : (running ? phrase[phraseIndexRun] : phrase[phraseIndexEdit]);
		int step = editingSequence ? (running ? stepIndexRun : stepIndexEdit) : (stepIndexRun);
		if (running) {
			bool muteGate1 = !editingSequence && (params[GATE1_PARAM].value > 0.5f);// live mute
			bool muteGate2 = !editingSequence && (params[GATE2_PARAM].value > 0.5f);// live mute
			float slideOffset = (slideStepsRemain > 0ul ? (slideCVdelta * (float)slideStepsRemain) : 0.0f);
			outputs[CV_OUTPUT].value = cv[seq][step] - slideOffset;
			bool retriggingOnReset = (clockIgnoreOnReset != 0l && retrigGatesOnReset);
			outputs[GATE1_OUTPUT].value = (calcGate(gate1Code, clockTrigger, clockPeriod, sampleRate) && !muteGate1 && !retriggingOnReset) ? 10.0f : 0.0f;
			outputs[GATE2_OUTPUT].value = (calcGate(gate2Code, clockTrigger, clockPeriod, sampleRate) && !muteGate2 && !retriggingOnReset) ? 10.0f : 0.0f;
		}
		else {// not running 
			outputs[CV_OUTPUT].value = (editingGate > 0ul) ? editingGateCV : cv[seq][step];
			outputs[GATE1_OUTPUT].value = (editingGate > 0ul) ? 10.0f : 0.0f;
			outputs[GATE2_OUTPUT].value = (editingGate > 0ul) ? 10.0f : 0.0f;
		}
		if (slideStepsRemain > 0ul)
			slideStepsRemain--;
		
		lightRefreshCounter++;
		if (lightRefreshCounter >= displayRefreshStepSkips) {
			lightRefreshCounter = 0;

			// Step/phrase lights
			for (int i = 0; i < 16; i++) {
				float red = 0.0f;
				float green = 0.0f;
				float white = 0.0f;
				if (infoCopyPaste != 0l) {
					if (i >= startCP && i < (startCP + countCP))
						green = 0.5f;
				}
				else if (displayState == DISP_LENGTH) {
					if (editingSequence) {
						if (i < (sequences[sequence].getLength() - 1))
							green = 0.1f;
						else if (i == (sequences[sequence].getLength() - 1))
							green = 1.0f;
					}
					else {
						if (i < phrases - 1)
							green = 0.1f;
						else
							green = (i == phrases - 1) ? 1.0f : 0.0f;
					}					
				}
				else if (displayState == DISP_TRANSPOSE) {
					red = 0.5f;
				}
				else if (displayState == DISP_ROTATE) {
					red = (i == stepIndexEdit ? 1.0f : (i < sequences[sequence].getLength() ? 0.2f : 0.0f));
				}
				else {// normal led display (i.e. not length)
					// Run cursor (green)
					if (editingSequence)
						green = ((running && (i == stepIndexRun)) ? 1.0f : 0.0f);
					else {
						green = ((running && (i == phraseIndexRun)) ? 1.0f : 0.0f);
						green += ((running && (i == stepIndexRun) && i != phraseIndexEdit) ? 0.1f : 0.0f);
						green = clamp(green, 0.0f, 1.0f);
					}
					// Edit cursor (red)
					if (editingSequence)
						red = (i == stepIndexEdit ? 1.0f : 0.0f);
					else
						red = (i == phraseIndexEdit ? 1.0f : 0.0f);
					bool gate = false;
					if (editingSequence)
						gate = attributes[sequence][i].getGate1();
					else if (!editingSequence && attached)
						gate = attributes[phrase[phraseIndexRun]][i].getGate1();
					white = ((green == 0.0f && red == 0.0f && gate && displayState != DISP_MODE) ? 0.04f : 0.0f);
					if (editingSequence && white != 0.0f) {
						green = 0.02f; white = 0.0f;
					}
					//if (white != 0.0f && attributes[sequence][i].getGate1P()) white = 0.01f;
				}
				setGreenRed(STEP_PHRASE_LIGHTS + i * 3, green, red);
				lights[STEP_PHRASE_LIGHTS + i * 3 + 2].value = white;
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
						bool warningFlashState = calcWarningFlash(tiedWarning, (long) (warningTime * sampleRate / displayRefreshStepSkips));
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
			int keyLightIndex = clamp( (int)((cvValOffset-floor(cvValOffset)) * 12.0f + 0.5f),  0,  11);
			if (editingPpqn != 0) {
				for (int i = 0; i < 12; i++) {
					if (keyIndexToGateMode(i, pulsesPerStep) != -1) {
						setGreenRed(KEY_LIGHTS + i * 2, 1.0f, 1.0f);
					}
					else {
						setGreenRed(KEY_LIGHTS + i * 2, 0.0f, 0.0f);
					}
				}
			} 
			else if (editingGateLength != 0l && editingSequence) {
				int modeLightIndex = gateModeToKeyLightIndex(attributes[sequence][stepIndexEdit], editingGateLength > 0l);
				for (int i = 0; i < 12; i++) {
					float green = editingGateLength > 0l ? 1.0f : 0.2f;
					float red = editingGateLength > 0l ? 0.2f : 1.0f;
					if (editingType > 0ul) {
						if (i == editingGateKeyLight) {
							float dimMult = ((float) editingType / (float)(gateTime * sampleRate / displayRefreshStepSkips));
							setGreenRed(KEY_LIGHTS + i * 2, green * dimMult, red * dimMult);
						}
						else
							setGreenRed(KEY_LIGHTS + i * 2, 0.0f, 0.0f);
					}
					else {
						if (i == modeLightIndex) {
							setGreenRed(KEY_LIGHTS + i * 2, green, red);
						}
						else { // show dim note if gatetype is different than note
							setGreenRed(KEY_LIGHTS + i * 2, 0.0f, (i == keyLightIndex ? 0.1f : 0.0f));
						}
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
							bool warningFlashState = calcWarningFlash(tiedWarning, (long) (warningTime * sampleRate / displayRefreshStepSkips));
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
			
			// Key mode light (note or gate type)
			lights[KEYNOTE_LIGHT].value = editingGateLength == 0l ? 10.0f : 0.0f;
			if (editingGateLength == 0l)
				setGreenRed(KEYGATE_LIGHT, 0.0f, 0.0f);
			else if (editingGateLength > 0l)
				setGreenRed(KEYGATE_LIGHT, 1.0f, 0.2f);
			else
				setGreenRed(KEYGATE_LIGHT, 0.2f, 1.0f);

			// Gate1, Gate1Prob, Gate2, Slide and Tied lights
			if (!editingSequence && (!attached || !running)) {// no oct lights when song mode and either (detached [1] or stopped [2])
											// [1] makes no sense, can't mod steps and stepping though seq that may not be playing
											// [2] CV is set to 0V when not running and in song mode, so cv[][] makes no sense to display
				setGateLight(false, GATE1_LIGHT);
				setGateLight(false, GATE2_LIGHT);
				setGreenRed(GATE1_PROB_LIGHT, 0.0f, 0.0f);
				lights[SLIDE_LIGHT].value = 0.0f;
				lights[TIE_LIGHT].value = 0.0f;
			}
			else {
				StepAttributes attributesVal = attributes[sequence][stepIndexEdit];
				if (!editingSequence)
					attributesVal = attributes[phrase[phraseIndexEdit]][stepIndexRun];
				//
				setGateLight(attributesVal.getGate1(), GATE1_LIGHT);
				setGateLight(attributesVal.getGate2(), GATE2_LIGHT);
				setGreenRed(GATE1_PROB_LIGHT, attributesVal.getGate1P() ? 1.0f : 0.0f, attributesVal.getGate1P() ? 1.0f : 0.0f);
				lights[SLIDE_LIGHT].value = attributesVal.getSlide() ? 1.0f : 0.0f;
				if (tiedWarning > 0l) {
					bool warningFlashState = calcWarningFlash(tiedWarning, (long) (warningTime * sampleRate / displayRefreshStepSkips));
					lights[TIE_LIGHT].value = (warningFlashState) ? 1.0f : 0.0f;
				}
				else
					lights[TIE_LIGHT].value = attributesVal.getTied() ? 1.0f : 0.0f;
			}

			// Attach light
			if (attachedWarning > 0l) {
				bool warningFlashState = calcWarningFlash(attachedWarning, (long) (warningTime * sampleRate / displayRefreshStepSkips));
				lights[ATTACH_LIGHT].value = (warningFlashState) ? 1.0f : 0.0f;
			}
			else
				lights[ATTACH_LIGHT].value = (attached ? 1.0f : 0.0f);
			
			// Reset light
			lights[RESET_LIGHT].value =	resetLight;	
			resetLight -= (resetLight / lightLambda) * engineGetSampleTime() * displayRefreshStepSkips;
			
			// Run light
			lights[RUN_LIGHT].value = running ? 1.0f : 0.0f;
			
			if (editingGate > 0ul)
				editingGate--;
			if (editingType > 0ul)
				editingType--;
			if (infoCopyPaste != 0l) {
				if (infoCopyPaste > 0l)
					infoCopyPaste --;
				if (infoCopyPaste < 0l)
					infoCopyPaste ++;
			}
			if (editingPpqn > 0l)
				editingPpqn--;
			if (tiedWarning > 0l)
				tiedWarning--;
			if (attachedWarning > 0l)
				attachedWarning--;
			if (modeHoldDetect.process(params[RUNMODE_PARAM].value)) {
				displayState = DISP_NORMAL;
				editingPpqn = (long) (editGateLengthTime * sampleRate / displayRefreshStepSkips);
			}
			if (revertDisplay > 0l) {
				if (revertDisplay == 1)
					displayState = DISP_NORMAL;
				revertDisplay--;
			}
		}// lightRefreshCounter
		
		if (clockIgnoreOnReset > 0l)
			clockIgnoreOnReset--;

		
		// VCO
		oscillatorVco.analog = params[VCO_MODE_PARAM].value > 0.0f;
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
		if ((lightRefreshCounter & userInputsStepSkipMask) == 0) {
			oscillatorClk.setPitch(params[CLK_FREQ_PARAM].value + log2f(pulsesPerStep));
			oscillatorClk.setPulseWidth(params[CLK_PW_PARAM].value);
		}	
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
			if ((lightRefreshCounter & userInputsStepSkipMask) == 0) {
				oscillatorLfo.setPitch(params[LFO_FREQ_PARAM].value);
			}
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
	

	inline void setGreenRed(int id, float green, float red) {
		lights[id + 0].value = green;
		lights[id + 1].value = red;
	}
	
	inline void propagateCVtoTied(int seqn, int stepn) {
		for (int i = stepn + 1; i < 16; i++) {
			if (!attributes[seqn][i].getTied())
				break;
			cv[seqn][i] = cv[seqn][i - 1];
		}	
	}

	void activateTiedStep(int seqn, int stepn) {
		attributes[seqn][stepn].setTied(true);
		if (stepn > 0) 
			propagateCVtoTied(seqn, stepn - 1);
		
		if (holdTiedNotes) {// new method
			attributes[seqn][stepn].setGate1(true);
			for (int i = max(stepn, 1); i < 16 && attributes[seqn][i].getTied(); i++) {
				attributes[seqn][i].setGate1Mode(attributes[seqn][i - 1].getGate1Mode());
				attributes[seqn][i - 1].setGate1Mode(5);
				attributes[seqn][i - 1].setGate1(true);
			}
		}
		else {// old method
			if (stepn > 0) {
				attributes[seqn][stepn] = attributes[seqn][stepn - 1];
				attributes[seqn][stepn].setTied(true);
			}
		}
	}
	
	void deactivateTiedStep(int seqn, int stepn) {
		attributes[seqn][stepn].setTied(false);
		if (holdTiedNotes) {// new method
			int lastGateType = attributes[seqn][stepn].getGate1Mode();
			for (int i = stepn + 1; i < 16 && attributes[seqn][i].getTied(); i++)
				lastGateType = attributes[seqn][i].getGate1Mode();
			if (stepn > 0)
				attributes[seqn][stepn - 1].setGate1Mode(lastGateType);
		}
		//else old method, nothing to do
	}
	
	inline void setGateLight(bool gateOn, int lightIndex) {
		if (!gateOn) {
			lights[lightIndex + 0].value = 0.0f;
			lights[lightIndex + 1].value = 0.0f;
		}
		else if (editingGateLength == 0l) {
			lights[lightIndex + 0].value = 0.0f;
			lights[lightIndex + 1].value = 1.0f;
		}
		else {
			lights[lightIndex + 0].value = lightIndex == GATE1_LIGHT ? 1.0f : 0.2f;
			lights[lightIndex + 1].value = lightIndex == GATE1_LIGHT ? 0.2f : 1.0f;
		}
	}

};



struct SemiModularSynthWidget : ModuleWidget {
	SemiModularSynth *module;
	DynamicSVGPanel *panel;

	struct SequenceDisplayWidget : TransparentWidget {
		SemiModularSynth *module;
		std::shared_ptr<Font> font;
		char displayStr[4];
		
		SequenceDisplayWidget() {
			font = Font::load(assetPlugin(plugin, "res/fonts/Segment14.ttf"));
		}
		
		void runModeToStr(int num) {
			if (num >= 0 && num < (NUM_MODES - 1))
				snprintf(displayStr, 4, "%s", modeLabels[num].c_str());
		}

		void draw(NVGcontext *vg) override {
			NVGcolor textColor = prepareDisplay(vg, &box, 18);
			nvgFontFaceId(vg, font->handle);
			bool editingSequence = module->isEditingSequence();

			Vec textPos = Vec(6, 24);
			nvgFillColor(vg, nvgTransRGBA(textColor, displayAlpha));
			nvgText(vg, textPos.x, textPos.y, "~~~", NULL);
			nvgFillColor(vg, textColor);
			if (module->infoCopyPaste != 0l) {
				if (module->infoCopyPaste > 0l)
					snprintf(displayStr, 4, "CPY");
				else {
					float cpMode = module->params[SemiModularSynth::CPMODE_PARAM].value;
					if (editingSequence && !module->seqCopied) {// cross paste to seq
						if (cpMode > 1.5f)// All = toggle gate 1
							snprintf(displayStr, 4, "TG1");
						else if (cpMode < 0.5f)// 4 = random CV
							snprintf(displayStr, 4, "RCV");
						else// 8 = random gate 1
							snprintf(displayStr, 4, "RG1");
					}
					else if (!editingSequence && module->seqCopied) {// cross paste to song
						if (cpMode > 1.5f)// All = init
							snprintf(displayStr, 4, "CLR");
						else if (cpMode < 0.5f)// 4 = increase by 1
							snprintf(displayStr, 4, "INC");
						else// 8 = random phrases
							snprintf(displayStr, 4, "RPH");
					}
					else
						snprintf(displayStr, 4, "PST");
				}
			}
			else if (module->editingPpqn != 0ul) {
				snprintf(displayStr, 4, "x%2u", (unsigned) module->pulsesPerStep);
			}
			else if (module->displayState == SemiModularSynth::DISP_MODE) {
				if (editingSequence)
					runModeToStr(module->sequences[module->sequence].getRunMode());
				else
					runModeToStr(module->runModeSong);
			}
			else if (module->displayState == SemiModularSynth::DISP_LENGTH) {
				if (editingSequence)
					snprintf(displayStr, 4, "L%2u", (unsigned) module->sequences[module->sequence].getLength());
				else
					snprintf(displayStr, 4, "L%2u", (unsigned) module->phrases);
			}
			else if (module->displayState == SemiModularSynth::DISP_TRANSPOSE) {
				snprintf(displayStr, 4, "+%2u", (unsigned) abs(module->sequences[module->sequence].getTranspose()));
				if (module->sequences[module->sequence].getTranspose() < 0)
					displayStr[0] = '-';
			}
			else if (module->displayState == SemiModularSynth::DISP_ROTATE) {
				snprintf(displayStr, 4, ")%2u", (unsigned) abs(module->sequences[module->sequence].getRotate()));
				if (module->sequences[module->sequence].getRotate() < 0)
					displayStr[0] = '(';
			}
			else {// DISP_NORMAL
				snprintf(displayStr, 4, " %2u", (unsigned) (editingSequence ? 
					module->sequence : module->phrase[module->phraseIndexEdit]) + 1 );
			}
			nvgText(vg, textPos.x, textPos.y, displayStr, NULL);
		}
	};		
	
	struct PanelThemeItem : MenuItem {
		SemiModularSynth *module;
		int panelTheme;
		void onAction(EventAction &e) override {
			module->panelTheme = panelTheme;
		}
		void step() override {
			rightText = (module->panelTheme == panelTheme) ? "✔" : "";
		}
	};
	struct ResetOnRunItem : MenuItem {
		SemiModularSynth *module;
		void onAction(EventAction &e) override {
			module->resetOnRun = !module->resetOnRun;
		}
	};
	struct AutoStepLenItem : MenuItem {
		SemiModularSynth *module;
		void onAction(EventAction &e) override {
			module->autostepLen = !module->autostepLen;
		}
	};
	struct AutoseqItem : MenuItem {
		SemiModularSynth *module;
		void onAction(EventAction &e) override {
			module->autoseq = !module->autoseq;
		}
	};
	struct HoldTiedItem : MenuItem {
		SemiModularSynth *module;
		void onAction(EventAction &e) override {
			module->holdTiedNotes = !module->holdTiedNotes;
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
		menu->addChild(classicItem);

		PanelThemeItem *lightItem = new PanelThemeItem();
		lightItem->text = "Light";
		lightItem->module = module;
		lightItem->panelTheme = 1;
		menu->addChild(lightItem);

		PanelThemeItem *darkItem = new PanelThemeItem();
		darkItem->text = darkPanelID;// ImpromptuModular.hpp
		darkItem->module = module;
		darkItem->panelTheme = 2;
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

		return menu;
	}	
	
	struct SequenceKnob : IMBigKnobInf {
		SequenceKnob() {};		
		void onMouseDown(EventMouseDown &e) override {// from ParamWidget.cpp
			SemiModularSynth* module = dynamic_cast<SemiModularSynth*>(this->module);
			if (e.button == 1) {
				// same code structure below as in sequence knob in main step()
				if (module->editingPpqn != 0) {
					module->pulsesPerStep = 1;
					//editingPpqn = (long) (editGateLengthTime * sampleRate / displayRefreshStepSkips);
				}
				else if (module->displayState == SemiModularSynth::DISP_MODE) {
					if (module->isEditingSequence()) {
						module->sequences[module->sequence].setRunMode(MODE_FWD);
					}
					else {
						module->runModeSong = MODE_FWD;
					}
				}
				else if (module->displayState == SemiModularSynth::DISP_LENGTH) {
					if (module->isEditingSequence()) {
						module->sequences[module->sequence].setLength(16);
					}
					else {
						module->phrases = 4;
					}
				}
				else if (module->displayState == SemiModularSynth::DISP_TRANSPOSE) {
					// nothing
				}
				else if (module->displayState == SemiModularSynth::DISP_ROTATE) {
					// nothing			
				}
				else {// DISP_NORMAL
					if (module->isEditingSequence()) {
						if (!module->inputs[SemiModularSynth::SEQCV_INPUT].active) {
							module->sequence = 0;;
						}
					}
					else {
						module->phrase[module->phraseIndexEdit] = 0;
					}
				}
			}
			ParamWidget::onMouseDown(e);
		}
	};		
	
	// void onHoverKey(EventHoverKey &e) override {// https://www.glfw.org/docs/latest/group__keys.html
		// SemiModularSynth* module = dynamic_cast<SemiModularSynth*>(this->module);
		// if (e.key == GLFW_KEY_SPACE) {
			// if (module->isEditingSequence()) {
				// module->attributes[module->sequence][module->stepIndexEdit].toggleGate1();
			// }			
			// e.consumed = true;
		// }
		// else
			// ModuleWidget::onHoverKey(e);
	// }
	
	SemiModularSynthWidget(SemiModularSynth *module) : ModuleWidget(module) {
		this->module = module;
		
		// SEQUENCER 
		
		// Main panel from Inkscape
        panel = new DynamicSVGPanel();
        panel->mode = &module->panelTheme;
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/SemiModular.svg")));
		panel->dupPanel();
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/dark/SemiModular_dark.svg")));
        box.size = panel->box.size;
        addChild(panel);		
		
		// Screws
		addChild(createDynamicScrew<IMScrew>(Vec(15, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(15, 365), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec((box.size.x - 90) * 1 / 3 + 30 , 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec((box.size.x - 90) * 1 / 3 + 30 , 365), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec((box.size.x - 90) * 2 / 3 + 45 , 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec((box.size.x - 90) * 2 / 3 + 45 , 365), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(box.size.x-30, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(box.size.x-30, 365), &module->panelTheme));

		
		
		// ****** Top row ******
		
		static const int rowRulerT0 = 54;
		static const int columnRulerT0 = 22;// Step LED buttons
		static const int columnRulerT3 = 404;// Attach (also used to align rest of right side of module)

		// Step/Phrase LED buttons
		int posX = columnRulerT0;
		static int spacingSteps = 20;
		static int spacingSteps4 = 4;
		for (int x = 0; x < 16; x++) {
			// First row
			addParam(createParam<LEDButton>(Vec(posX, rowRulerT0 - 7 + 3 - 4.4f), module, SemiModularSynth::STEP_PHRASE_PARAMS + x, 0.0f, 1.0f, 0.0f));
			addChild(createLight<MediumLight<GreenRedWhiteLight>>(Vec(posX + 4.4f, rowRulerT0 - 7 + 3), module, SemiModularSynth::STEP_PHRASE_LIGHTS + (x * 3)));
			// step position to next location and handle groups of four
			posX += spacingSteps;
			if ((x + 1) % 4 == 0)
				posX += spacingSteps4;
		}
		// Attach button and light
		addParam(createDynamicParam<IMPushButton>(Vec(columnRulerT3 - 4, rowRulerT0 - 6 + 2 + offsetTL1105), module, SemiModularSynth::ATTACH_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addChild(createLight<MediumLight<RedLight>>(Vec(columnRulerT3 + 12 + offsetMediumLight, rowRulerT0 - 6 + offsetMediumLight), module, SemiModularSynth::ATTACH_LIGHT));


		// Key mode LED buttons	
		static const int rowRulerKM = rowRulerT0 + 26 - 2;
		static const int colRulerKM = columnRulerT0 + 113;
		addParam(createParam<LEDButton>(Vec(colRulerKM + 112, rowRulerKM - 4.4f), module, SemiModularSynth::KEYNOTE_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(createLight<MediumLight<RedLight>>(Vec(colRulerKM + 112 + 4.4f,  rowRulerKM), module, SemiModularSynth::KEYNOTE_LIGHT));
		addParam(createParam<LEDButton>(Vec(colRulerKM, rowRulerKM - 4.4f), module, SemiModularSynth::KEYGATE_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(colRulerKM + 4.4f, rowRulerKM), module, SemiModularSynth::KEYGATE_LIGHT));
		
		
		
		// ****** Octave and keyboard area ******
		
		// Octave LED buttons
		static const float octLightsIntY = 20.0f;
		for (int i = 0; i < 7; i++) {
			addParam(createParam<LEDButton>(Vec(19 + 3, 86 + 24 + i * octLightsIntY- 4.4f), module, SemiModularSynth::OCTAVE_PARAM + i, 0.0f, 1.0f, 0.0f));
			addChild(createLight<MediumLight<RedLight>>(Vec(19 + 3 + 4.4f, 86 + 24 + i * octLightsIntY), module, SemiModularSynth::OCTAVE_LIGHTS + i));
		}
		// Keys and Key lights
		static const int keyNudgeX = 7 + 4;
		static const int KeyBlackY = 103 + 4;
		static const int KeyWhiteY = 141 + 4;
		static const int offsetKeyLEDx = 6;
		static const int offsetKeyLEDy = 16;
		// Black keys and lights
		addParam(createParam<InvisibleKeySmall>(			Vec(65+keyNudgeX, KeyBlackY), module, SemiModularSynth::KEY_PARAMS + 1, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(65+keyNudgeX+offsetKeyLEDx, KeyBlackY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 1 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(93+keyNudgeX, KeyBlackY), module, SemiModularSynth::KEY_PARAMS + 3, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(93+keyNudgeX+offsetKeyLEDx, KeyBlackY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 3 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(150+keyNudgeX, KeyBlackY), module, SemiModularSynth::KEY_PARAMS + 6, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(150+keyNudgeX+offsetKeyLEDx, KeyBlackY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 6 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(178+keyNudgeX, KeyBlackY), module, SemiModularSynth::KEY_PARAMS + 8, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(178+keyNudgeX+offsetKeyLEDx, KeyBlackY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 8 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(206+keyNudgeX, KeyBlackY), module, SemiModularSynth::KEY_PARAMS + 10, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(206+keyNudgeX+offsetKeyLEDx, KeyBlackY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 10 * 2));
		// White keys and lights
		addParam(createParam<InvisibleKeySmall>(			Vec(51+keyNudgeX, KeyWhiteY), module, SemiModularSynth::KEY_PARAMS + 0, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(51+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 0 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(79+keyNudgeX, KeyWhiteY), module, SemiModularSynth::KEY_PARAMS + 2, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(79+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 2 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(107+keyNudgeX, KeyWhiteY), module, SemiModularSynth::KEY_PARAMS + 4, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(107+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 4 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(136+keyNudgeX, KeyWhiteY), module, SemiModularSynth::KEY_PARAMS + 5, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(136+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 5 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(164+keyNudgeX, KeyWhiteY), module, SemiModularSynth::KEY_PARAMS + 7, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(164+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 7 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(192+keyNudgeX, KeyWhiteY), module, SemiModularSynth::KEY_PARAMS + 9, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(192+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 9 * 2));
		addParam(createParam<InvisibleKeySmall>(			Vec(220+keyNudgeX, KeyWhiteY), module, SemiModularSynth::KEY_PARAMS + 11, 0.0, 1.0, 0.0));
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(220+keyNudgeX+offsetKeyLEDx, KeyWhiteY+offsetKeyLEDy), module, SemiModularSynth::KEY_LIGHTS + 11 * 2));		
		
		
		
		// ****** Right side control area ******

		static const int rowRulerMK0 = 105;// Edit mode row
		static const int rowRulerMK1 = rowRulerMK0 + 56; // Run row
		static const int rowRulerMK2 = rowRulerMK1 + 54; // Reset row
		static const int columnRulerMK0 = 280;// Edit mode column
		static const int columnRulerMK1 = columnRulerMK0 + 59;// Display column
		static const int columnRulerMK2 = columnRulerT3;// Run mode column
		
		// Edit mode switch
		addParam(createParam<CKSSNoRandom>(Vec(columnRulerMK0 + hOffsetCKSS, rowRulerMK0 + vOffsetCKSS), module, SemiModularSynth::EDIT_PARAM, 0.0f, 1.0f, 1.0f));
		// Sequence display
		SequenceDisplayWidget *displaySequence = new SequenceDisplayWidget();
		displaySequence->box.pos = Vec(columnRulerMK1-15, rowRulerMK0 + 3 + vOffsetDisplay);
		displaySequence->box.size = Vec(55, 30);// 3 characters
		displaySequence->module = module;
		addChild(displaySequence);
		// Len/mode button
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerMK2 + offsetCKD6b, rowRulerMK0 + 0 + offsetCKD6b), module, SemiModularSynth::RUNMODE_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));

		// Run LED bezel and light
		addParam(createParam<LEDBezel>(Vec(columnRulerMK0 + offsetLEDbezel, rowRulerMK1 + 7 + offsetLEDbezel), module, SemiModularSynth::RUN_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(createLight<MuteLight<GreenLight>>(Vec(columnRulerMK0 + offsetLEDbezel + offsetLEDbezelLight, rowRulerMK1 + 7 + offsetLEDbezel + offsetLEDbezelLight), module, SemiModularSynth::RUN_LIGHT));
		// Sequence knob
		addParam(createDynamicParam<SequenceKnob>(Vec(columnRulerMK1 + 1 + offsetIMBigKnob, rowRulerMK0 + 55 + offsetIMBigKnob), module, SemiModularSynth::SEQUENCE_PARAM, -INFINITY, INFINITY, 0.0f, &module->panelTheme));		
		// Transpose/rotate button
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerMK2 + offsetCKD6b, rowRulerMK1 + 4 + offsetCKD6b), module, SemiModularSynth::TRAN_ROT_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		
		// Reset LED bezel and light
		addParam(createParam<LEDBezel>(Vec(columnRulerMK0 + offsetLEDbezel, rowRulerMK2 + 5 + offsetLEDbezel), module, SemiModularSynth::RESET_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(createLight<MuteLight<GreenLight>>(Vec(columnRulerMK0 + offsetLEDbezel + offsetLEDbezelLight, rowRulerMK2 + 5 + offsetLEDbezel + offsetLEDbezelLight), module, SemiModularSynth::RESET_LIGHT));
		// Copy/paste buttons
		addParam(createDynamicParam<IMPushButton>(Vec(columnRulerMK1 - 10, rowRulerMK2 + 5 + offsetTL1105), module, SemiModularSynth::COPY_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParam<IMPushButton>(Vec(columnRulerMK1 + 20, rowRulerMK2 + 5 + offsetTL1105), module, SemiModularSynth::PASTE_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Copy-paste mode switch (3 position)
		addParam(createParam<CKSSThreeInvNoRandom>(Vec(columnRulerMK2 + hOffsetCKSS + 1, rowRulerMK2 - 3 + vOffsetCKSSThree), module, SemiModularSynth::CPMODE_PARAM, 0.0f, 2.0f, 2.0f));	// 0.0f is top position

		
		
		// ****** Gate and slide section ******
		
		static const int rowRulerMB0 = 214 + 4;
		static const int columnRulerMBspacing = 70;
		static const int columnRulerMB2 = 134;// Gate2
		static const int columnRulerMB1 = columnRulerMB2 - columnRulerMBspacing;// Gate1 
		static const int columnRulerMB3 = columnRulerMB2 + columnRulerMBspacing;// Tie
		static const int posLEDvsButton = + 25;
		
		// Gate 1 light and button
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(columnRulerMB1 + posLEDvsButton + offsetMediumLight, rowRulerMB0 + 4 + offsetMediumLight), module, SemiModularSynth::GATE1_LIGHT));		
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerMB1 + offsetCKD6b, rowRulerMB0 + 4 + offsetCKD6b), module, SemiModularSynth::GATE1_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Gate 2 light and button
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(columnRulerMB2 + posLEDvsButton + offsetMediumLight, rowRulerMB0 + 4 + offsetMediumLight), module, SemiModularSynth::GATE2_LIGHT));		
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerMB2 + offsetCKD6b, rowRulerMB0 + 4 + offsetCKD6b), module, SemiModularSynth::GATE2_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Tie light and button
		addChild(createLight<MediumLight<RedLight>>(Vec(columnRulerMB3 + posLEDvsButton + offsetMediumLight, rowRulerMB0 + 4 + offsetMediumLight), module, SemiModularSynth::TIE_LIGHT));		
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
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(columnRulerB0 + posLEDvsButton + offsetMediumLight, rowRulerB1 + offsetMediumLight), module, SemiModularSynth::GATE1_PROB_LIGHT));		
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerB0 + offsetCKD6b, rowRulerB1 + offsetCKD6b), module, SemiModularSynth::GATE1_PROB_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Gate 1 probability knob
		addParam(createDynamicParam<IMSmallKnob>(Vec(columnRulerB1 + offsetIMSmallKnob, rowRulerB1 + offsetIMSmallKnob), module, SemiModularSynth::GATE1_KNOB_PARAM, 0.0f, 1.0f, 1.0f, &module->panelTheme));
		// Slide light and button
		addChild(createLight<MediumLight<RedLight>>(Vec(columnRulerB2 + posLEDvsButton + offsetMediumLight, rowRulerB1 + offsetMediumLight), module, SemiModularSynth::SLIDE_LIGHT));		
		addParam(createDynamicParam<IMBigPushButton>(Vec(columnRulerB2 + offsetCKD6b, rowRulerB1 + offsetCKD6b), module, SemiModularSynth::SLIDE_BTN_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		// Slide knob
		addParam(createDynamicParam<IMSmallKnob>(Vec(columnRulerB3 + offsetIMSmallKnob, rowRulerB1 + offsetIMSmallKnob), module, SemiModularSynth::SLIDE_KNOB_PARAM, 0.0f, 2.0f, 0.2f, &module->panelTheme));
		// Autostep
		addParam(createParam<CKSSNoRandom>(Vec(columnRulerB4 + hOffsetCKSS, rowRulerB1 + vOffsetCKSS), module, SemiModularSynth::AUTOSTEP_PARAM, 0.0f, 1.0f, 1.0f));		
		// CV in
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB5, rowRulerB1), Port::INPUT, module, SemiModularSynth::CV_INPUT, &module->panelTheme));
		// Reset
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB6, rowRulerB1), Port::INPUT, module, SemiModularSynth::RESET_INPUT, &module->panelTheme));
		// Clock
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB7, rowRulerB1), Port::INPUT, module, SemiModularSynth::CLOCK_INPUT, &module->panelTheme));

		

		// ****** Bottom row (all aligned) ******

	
		// CV control Inputs 
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB0, rowRulerB0), Port::INPUT, module, SemiModularSynth::LEFTCV_INPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB1, rowRulerB0), Port::INPUT, module, SemiModularSynth::RIGHTCV_INPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB2, rowRulerB0), Port::INPUT, module, SemiModularSynth::SEQCV_INPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB3, rowRulerB0), Port::INPUT, module, SemiModularSynth::RUNCV_INPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(columnRulerB4, rowRulerB0), Port::INPUT, module, SemiModularSynth::WRITE_INPUT, &module->panelTheme));
		// Outputs
		addOutput(createDynamicPort<IMPort>(Vec(columnRulerB5, rowRulerB0), Port::OUTPUT, module, SemiModularSynth::CV_OUTPUT, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRulerB6, rowRulerB0), Port::OUTPUT, module, SemiModularSynth::GATE1_OUTPUT, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(columnRulerB7, rowRulerB0), Port::OUTPUT, module, SemiModularSynth::GATE2_OUTPUT, &module->panelTheme));
		
		
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

		addParam(createDynamicParam<IMBigKnob>(Vec(colRulerVCO0 + offsetIMBigKnob + 55 / 2, rowRulerVCO0 + offsetIMBigKnob), module, SemiModularSynth::VCO_FREQ_PARAM, -54.0f, 54.0f, 0.0f, &module->panelTheme));
		
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVCO0 + offsetIMSmallKnob, rowRulerVCO1 + offsetIMSmallKnob), module, SemiModularSynth::VCO_FINE_PARAM, -1.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVCO1 + offsetIMSmallKnob, rowRulerVCO1 + offsetIMSmallKnob), module, SemiModularSynth::VCO_PW_PARAM, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVCO0 + offsetIMSmallKnob, rowRulerVCO2 + offsetIMSmallKnob), module, SemiModularSynth::VCO_FM_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVCO1 + offsetIMSmallKnob, rowRulerVCO2 + offsetIMSmallKnob), module, SemiModularSynth::VCO_PWM_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));

		addParam(createParam<CKSS>(Vec(colRulerVCO0 + hOffsetCKSS, rowRulerVCO3 + vOffsetCKSS), module, SemiModularSynth::VCO_MODE_PARAM, 0.0f, 1.0f, 1.0f));
		addParam(createDynamicParam<IMFivePosSmallKnob>(Vec(colRulerVCO1 + offsetIMSmallKnob, rowRulerVCO3 + offsetIMSmallKnob), module, SemiModularSynth::VCO_OCT_PARAM, -2.0f, 2.0f, 0.0f, &module->panelTheme));

		addOutput(createDynamicPort<IMPort>(Vec(colRulerVCO0, rowRulerVCO4), Port::OUTPUT, module, SemiModularSynth::VCO_SIN_OUTPUT, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerVCO1, rowRulerVCO4), Port::OUTPUT, module, SemiModularSynth::VCO_TRI_OUTPUT, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerVCO0, rowRulerVCO5), Port::OUTPUT, module, SemiModularSynth::VCO_SAW_OUTPUT, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerVCO1, rowRulerVCO5), Port::OUTPUT, module, SemiModularSynth::VCO_SQR_OUTPUT, &module->panelTheme));		
		
		addInput(createDynamicPort<IMPort>(Vec(colRulerVCO0, rowRulerVCO6), Port::INPUT, module, SemiModularSynth::VCO_PITCH_INPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerVCO1, rowRulerVCO6), Port::INPUT, module, SemiModularSynth::VCO_FM_INPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerVCO0, rowRulerVCO7), Port::INPUT, module, SemiModularSynth::VCO_SYNC_INPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerVCO1, rowRulerVCO7), Port::INPUT, module, SemiModularSynth::VCO_PW_INPUT, &module->panelTheme));

		
		// CLK
		static const int rowRulerClk0 = 41;
		static const int rowRulerClk1 = rowRulerClk0 + 45;// exact value from svg
		static const int rowRulerClk2 = rowRulerClk1 + 38;
		static const int colRulerClk0 = colRulerVCO1 + 55;// exact value from svg
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerClk0 + offsetIMSmallKnob, rowRulerClk0 + offsetIMSmallKnob), module, SemiModularSynth::CLK_FREQ_PARAM, -2.0f, 4.0f, 1.0f, &module->panelTheme));// 120 BMP when default value
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerClk0 + offsetIMSmallKnob, rowRulerClk1 + offsetIMSmallKnob), module, SemiModularSynth::CLK_PW_PARAM, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerClk0, rowRulerClk2), Port::OUTPUT, module, SemiModularSynth::CLK_OUT_OUTPUT, &module->panelTheme));
		
		
		// VCA
		static const int colRulerVca1 = colRulerClk0 + 55;// exact value from svg
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerClk0 + offsetIMSmallKnob, rowRulerVCO3 + offsetIMSmallKnob), module, SemiModularSynth::VCA_LEVEL1_PARAM, 0.0f, 1.0f, 1.0f, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerClk0, rowRulerVCO4), Port::INPUT, module, SemiModularSynth::VCA_LIN1_INPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerClk0, rowRulerVCO5), Port::INPUT, module, SemiModularSynth::VCA_IN1_INPUT, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerVca1, rowRulerVCO5), Port::OUTPUT, module, SemiModularSynth::VCA_OUT1_OUTPUT, &module->panelTheme));
		
		
		// ADSR
		static const int rowRulerAdsr0 = rowRulerClk0;
		static const int rowRulerAdsr3 = rowRulerVCO2 + 6;
		static const int rowRulerAdsr1 = rowRulerAdsr0 + (rowRulerAdsr3 - rowRulerAdsr0) * 1 / 3;
		static const int rowRulerAdsr2 = rowRulerAdsr0 + (rowRulerAdsr3 - rowRulerAdsr0) * 2 / 3;
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVca1 + offsetIMSmallKnob, rowRulerAdsr0 + offsetIMSmallKnob), module, SemiModularSynth::ADSR_ATTACK_PARAM, 0.0f, 1.0f, 0.1f, &module->panelTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVca1 + offsetIMSmallKnob, rowRulerAdsr1 + offsetIMSmallKnob), module, SemiModularSynth::ADSR_DECAY_PARAM,  0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVca1 + offsetIMSmallKnob, rowRulerAdsr2 + offsetIMSmallKnob), module, SemiModularSynth::ADSR_SUSTAIN_PARAM, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVca1 + offsetIMSmallKnob, rowRulerAdsr3 + offsetIMSmallKnob), module, SemiModularSynth::ADSR_RELEASE_PARAM, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerVca1, rowRulerVCO3), Port::OUTPUT, module, SemiModularSynth::ADSR_ENVELOPE_OUTPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerVca1, rowRulerVCO4), Port::INPUT, module, SemiModularSynth::ADSR_GATE_INPUT, &module->panelTheme));

		
		// VCF
		static const int colRulerVCF0 = colRulerVca1 + 55;// exact value from svg
		static const int colRulerVCF1 = colRulerVCF0 + 55;// exact value from svg
		addParam(createDynamicParam<IMBigKnob>(Vec(colRulerVCF0 + offsetIMBigKnob + 55 / 2, rowRulerVCO0 + offsetIMBigKnob), module, SemiModularSynth::VCF_FREQ_PARAM, 0.0f, 1.0f, 0.666f, &module->panelTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVCF0 + offsetIMSmallKnob + 55 / 2, rowRulerVCO1 + offsetIMSmallKnob), module, SemiModularSynth::VCF_RES_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVCF0 + offsetIMSmallKnob , rowRulerVCO2 + offsetIMSmallKnob), module, SemiModularSynth::VCF_FREQ_CV_PARAM, -1.0f, 1.0f, 0.0f, &module->panelTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerVCF1 + offsetIMSmallKnob , rowRulerVCO2 + offsetIMSmallKnob), module, SemiModularSynth::VCF_DRIVE_PARAM, 0.0f, 1.0f, 0.0f, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerVCF0, rowRulerVCO3), Port::INPUT, module, SemiModularSynth::VCF_FREQ_INPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerVCF1, rowRulerVCO3), Port::INPUT, module, SemiModularSynth::VCF_RES_INPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerVCF0, rowRulerVCO4), Port::INPUT, module, SemiModularSynth::VCF_DRIVE_INPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerVCF0, rowRulerVCO5), Port::INPUT, module, SemiModularSynth::VCF_IN_INPUT, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerVCF1, rowRulerVCO4), Port::OUTPUT, module, SemiModularSynth::VCF_HPF_OUTPUT, &module->panelTheme));		
		addOutput(createDynamicPort<IMPort>(Vec(colRulerVCF1, rowRulerVCO5), Port::OUTPUT, module, SemiModularSynth::VCF_LPF_OUTPUT, &module->panelTheme));
		
		
		// LFO
		static const int colRulerLfo = colRulerVCF1 + 55;// exact value from svg
		static const int rowRulerLfo0 = rowRulerAdsr0;
		static const int rowRulerLfo2 = rowRulerVCO2;
		static const int rowRulerLfo1 = rowRulerLfo0 + (rowRulerLfo2 - rowRulerLfo0) / 2;
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerLfo + offsetIMSmallKnob, rowRulerLfo0 + offsetIMSmallKnob), module, SemiModularSynth::LFO_FREQ_PARAM, -8.0f, 6.0f, -1.0f, &module->panelTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerLfo + offsetIMSmallKnob, rowRulerLfo1 + offsetIMSmallKnob), module, SemiModularSynth::LFO_GAIN_PARAM, 0.0f, 1.0f, 0.5f, &module->panelTheme));
		addParam(createDynamicParam<IMSmallKnob>(Vec(colRulerLfo + offsetIMSmallKnob, rowRulerLfo2 + offsetIMSmallKnob), module, SemiModularSynth::LFO_OFFSET_PARAM, -1.0f, 1.0f, 0.0f, &module->panelTheme));
		addOutput(createDynamicPort<IMPort>(Vec(colRulerLfo, rowRulerVCO3), Port::OUTPUT, module, SemiModularSynth::LFO_TRI_OUTPUT, &module->panelTheme));		
		addOutput(createDynamicPort<IMPort>(Vec(colRulerLfo, rowRulerVCO4), Port::OUTPUT, module, SemiModularSynth::LFO_SIN_OUTPUT, &module->panelTheme));
		addInput(createDynamicPort<IMPort>(Vec(colRulerLfo, rowRulerVCO5), Port::INPUT, module, SemiModularSynth::LFO_RESET_INPUT, &module->panelTheme));
	}
};

} // namespace rack_plugin_ImpromptuModular

using namespace rack_plugin_ImpromptuModular;

RACK_PLUGIN_MODEL_INIT(ImpromptuModular, SemiModularSynth) {
   Model *modelSemiModularSynth = Model::create<SemiModularSynth, SemiModularSynthWidget>("Impromptu Modular", "Semi-Modular Synth", "MISC - Semi-Modular Synth", SEQUENCER_TAG, OSCILLATOR_TAG);
   return modelSemiModularSynth;
}

/*CHANGE LOG

0.6.16:
add gate status feedback in steps (white lights)

0.6.15:
add right-click menu option to bound AutoStep writes by sequence lengths

0.6.14: 
rotate offsets are now persistent and stored in the sequencer
allow ctrl-right-click of notes to copy note/gate-type over to next step (not just move to next step)

0.6.13:
fix run mode bug (history not reset when hard reset)
fix slide bug when reset happens during a slide and run stays on
add live mute on Gate1 and Gate2 buttons in song mode
fix initRun() timing bug when turn off-and-then-on running button (it was resetting ppqnCount)
allow pulsesPerStep setting of 1 and all even values from 2 to 24, and allow all gate types that work in these
fix tied bug that prevented correct tied propagation when editing beyond sequence length less than 16
implement held tied notes option
clear all attributes (gates, gatep, tied, slide) when cross-paste to seq ALL (CVs not affected)
implement right-click initialization on main knob

0.6.12:
input refresh optimization
add buttons for note vs advanced-gate selection (remove timeout method)
transposition amount stays persistent and is saved (reset to 0 on module init or paste ALL)

0.6.11:
step optimization of lights refresh
implement copy-paste in song mode
implement cross paste trick for init and randomize seq/song
remove length and arrow buttons and make steps with LED buttons
add advanced gate mode
add AutoSeq option when writing via CV inputs 

0.6.10:
unlock gates when tied (turn off when press tied, but allow to be turned back on)
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
