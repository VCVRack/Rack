#include "midi.hpp"
#include "dsp/filter.hpp"
#include "dsp/digital.hpp"
#include "moDllz.hpp"
#include <algorithm>    // std::find
#include <vector>       // std::vector

namespace rack_plugin_moDllz {

struct MIDI8MPE : Module {
	enum ParamIds {
		RESETMIDI_PARAM,
		LCURSOR_PARAM,
		RCURSOR_PARAM,
		PLUSONE_PARAM,
		MINUSONE_PARAM,
		LEARNCCA_PARAM,
		LEARNCCB_PARAM,
		LEARNCCC_PARAM,
		LEARNCCD_PARAM,
		LEARNCCE_PARAM,
		LEARNCCF_PARAM,
		SUSTHOLD_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(X_OUTPUT, 8),
		ENUMS(Y_OUTPUT, 8),
		ENUMS(Z_OUTPUT, 8),
		ENUMS(VEL_OUTPUT, 8),
		ENUMS(GATE_OUTPUT, 8),

		MMA_OUTPUT,
		MMB_OUTPUT,
		MMC_OUTPUT,
		MMD_OUTPUT,
		MME_OUTPUT,
		MMF_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		RESETMIDI_LIGHT,
		ENUMS(CH_LIGHT, 8),
		SUSTHOLD_LIGHT,
		NUM_LIGHTS
	};

	MidiInputQueue midiInput;

	enum PolyMode {
		MPE_MODE,
		ROTATE_MODE,
		REUSE_MODE,
		RESET_MODE,
		REASSIGN_MODE,
		UNISON_MODE,
		NUM_MODES
	};
	PolyMode polyMode = ROTATE_MODE;

	bool holdGates = true;
	struct NoteData {
		uint8_t velocity = 0;
		uint8_t aftertouch = 0;
	};

	NoteData noteData[128];
	
	// cachedNotes : UNISON_MODE and REASSIGN_MODE cache all played notes. The other polyModes cache stolen notes (after the 4th one).
	std::vector<uint8_t> cachedNotes;
	
	std::vector<uint8_t> cachedMPE[8];
	
	
	uint8_t notes[8] = {0};
	uint8_t vels[8] = {0};
	int16_t mpex[8] = {0};
	uint16_t mpey[8] = {0};
	uint16_t mpez[8] = {0};
	uint8_t mpeyLB[8] = {0};
	uint8_t mpezLB[8] = {0};
	
	uint8_t mpePlusLB[8] = {0};

	bool gates[8] = {false};
	uint8_t Maft = 0;
	int midiCCs[6] = {128,1,129,11,7,64};
	uint8_t midiCCsVal[6] = {0};
	uint16_t Mpit = 8192;
	float xpitch[8] = {0.f};
	
	// gates set to TRUE by pedal and current gate. FALSE by pedal.
	bool pedalgates[8] = {false};
	bool pedal = false;
	int rotateIndex = 0;
	int stealIndex = 0;
	int numVo = 8;
	int polyModeIx = 1;
	int pbMain = 12;
	int pbMPE = 96;
	int mpeYcc = 74; //cc74 (default MPE Y)
	int mpeZcc = 128; //128 = ChannelAfterTouch (default MPE Z)
	int MPEmode = 0; // Index of different MPE modes...(User and HakenPlus for now)
	int savedMidiCh = -1;//to reset channel from MPE all channels
	int MPEmasterCh = 0;// 0 ~ 15
	int MPEfirstCh = 1;// 0 ~ 15

	int displayYcc = 74;
	int displayZcc = 128;

	int learnIx = 0;

	int cursorIx = 0;
	int cursorI = 0;
	int selectedmidich = 0;
	int cursorPoly[9] = {0,1,3,7,8,9,10,11,12};
	int cursorMPE[12] = {0,2,3,4,5,6,7,8,9,10,11,12};
	int cursorMPEsub[10] = {0,2,3,4,7,8,9,10,11,12};
	float dummy = 0.f;
	float *dataKnob = &dummy;
	int frameData = 100000;

	ExponentialFilter MPExFilter[8];
	ExponentialFilter MPEyFilter[8];
	ExponentialFilter MPEzFilter[8];
	ExponentialFilter MCCsFilter[6];
	ExponentialFilter MpitFilter;
	
	// retrigger for stolen notes (when gates already open)
	PulseGenerator reTrigger[8];
	SchmittTrigger resetMidiTrigger;

	SchmittTrigger PlusOneTrigger;
	SchmittTrigger MinusOneTrigger;
	SchmittTrigger LcursorTrigger;
	SchmittTrigger RcursorTrigger;

	SchmittTrigger learnCCsTrigger[6];

	
	MIDI8MPE() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "midi", midiInput.toJson());
		json_object_set_new(rootJ, "polyMode", json_integer(polyMode));
		json_object_set_new(rootJ, "pbMain", json_integer(pbMain));
		json_object_set_new(rootJ, "pbMPE", json_integer(pbMPE));
		json_object_set_new(rootJ, "numVo", json_integer(numVo));
		json_object_set_new(rootJ, "MPEmasterCh", json_integer(MPEmasterCh));
		json_object_set_new(rootJ, "MPEfirstCh", json_integer(MPEfirstCh));
		json_object_set_new(rootJ, "midiAcc", json_integer(midiCCs[0]));
		json_object_set_new(rootJ, "midiBcc", json_integer(midiCCs[1]));
		json_object_set_new(rootJ, "midiCcc", json_integer(midiCCs[2]));
		json_object_set_new(rootJ, "midiDcc", json_integer(midiCCs[3]));
		json_object_set_new(rootJ, "midiEcc", json_integer(midiCCs[4]));
		json_object_set_new(rootJ, "midiFcc", json_integer(midiCCs[5]));
		json_object_set_new(rootJ, "mpeYcc", json_integer(mpeYcc));
		json_object_set_new(rootJ, "mpeZcc", json_integer(mpeZcc));
		json_object_set_new(rootJ, "MPEmode", json_integer(MPEmode));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiInput.fromJson(midiJ);
		json_t *polyModeJ = json_object_get(rootJ, "polyMode");
		if (polyModeJ)
			polyMode = (PolyMode) json_integer_value(polyModeJ);
		polyModeIx = polyMode;
		json_t *pbMainJ = json_object_get(rootJ, "pbMain");
		if (pbMainJ)
			pbMain = json_integer_value(pbMainJ);
		json_t *pbMPEJ = json_object_get(rootJ, "pbMPE");
		if (pbMPEJ)
			pbMPE = json_integer_value(pbMPEJ);
		json_t *numVoJ = json_object_get(rootJ, "numVo");
		if (numVoJ)
			numVo = json_integer_value(numVoJ);
		json_t *MPEmasterChJ = json_object_get(rootJ, "MPEmasterCh");
		if (MPEmasterChJ)
			MPEmasterCh = json_integer_value(MPEmasterChJ);
		json_t *MPEfirstChJ = json_object_get(rootJ, "MPEfirstCh");
		if (MPEfirstChJ)
			MPEfirstCh = json_integer_value(MPEfirstChJ);
		json_t *midiAccJ = json_object_get(rootJ, "midiAcc");
		if (midiAccJ)
			midiCCs[0] = json_integer_value(midiAccJ);
		json_t *midiBccJ = json_object_get(rootJ, "midiBcc");
		if (midiBccJ)
			midiCCs[1] = json_integer_value(midiBccJ);
		json_t *midiCccJ = json_object_get(rootJ, "midiCcc");
		if (midiCccJ)
			midiCCs[2] = json_integer_value(midiCccJ);
		json_t *midiDccJ = json_object_get(rootJ, "midiDcc");
		if (midiDccJ)
			midiCCs[3] = json_integer_value(midiDccJ);
		json_t *midiEccJ = json_object_get(rootJ, "midiEcc");
		if (midiEccJ)
			midiCCs[4] = json_integer_value(midiEccJ);
		json_t *midiFccJ = json_object_get(rootJ, "midiFcc");
		if (midiFccJ)
			midiCCs[5] = json_integer_value(midiFccJ);
		json_t *mpeYccJ = json_object_get(rootJ, "mpeYcc");
		if (mpeYccJ)
			mpeYcc = json_integer_value(mpeYccJ);
		json_t *mpeZccJ = json_object_get(rootJ, "mpeZcc");
		if (mpeZccJ)
			mpeZcc = json_integer_value(mpeZccJ);
		json_t *MPEmodeJ = json_object_get(rootJ, "MPEmode");
		if (MPEmodeJ)
			MPEmode = json_integer_value(MPEmodeJ);
		
		if (polyModeIx > 0){
			displayYcc = 129;
			displayZcc = 130;
		}else if (MPEmode > 0){
			displayYcc = 131;
			displayZcc = 132;
		}else {
			displayYcc = mpeYcc;
			displayZcc = mpeZcc;
		}
	}
///////////////////////////ON RESET
	void onReset() override {
		for (int i = 0; i < 8; i++) {
			notes[i] = 60;
			gates[i] = false;
			pedalgates[i] = false;
			mpey[i] = 0.f;
		}
		rotateIndex = -1;
		cachedNotes.clear();
		float lambdaf = 100.f * engineGetSampleTime();
		
		if (polyMode == MPE_MODE) {
			midiInput.channel = -1;
			for (int i = 0; i < 8; i++) {
				mpex[i] = 0.f;
				mpez[i] = 0.f;
				cachedMPE[i].clear();
				MPExFilter[i].lambda = lambdaf;
				MPEyFilter[i].lambda = lambdaf;
				MPEzFilter[i].lambda = lambdaf;
			}
			if (MPEmode > 0){// Haken MPE Plus
				displayYcc = 131;
				displayZcc = 132;
			}else{
				displayYcc = mpeYcc;
				displayZcc = mpeZcc;
			}
		} else {
			displayYcc = 129;
			displayZcc = 130;
		}
		learnIx = 0;
		
		MpitFilter.lambda = lambdaf;
		for (int i=0; i < 6; i++){
			MCCsFilter[i].lambda = lambdaf;
		}
		MpitFilter.lambda = lambdaf;
	}

////////////////////////////////////////////////////
	int getPolyIndex(int nowIndex) {
		for (int i = 0; i < numVo; i++) {
			nowIndex++;
			if (nowIndex > (numVo - 1))
				nowIndex = 0;
			if (!(gates[nowIndex] || pedalgates[nowIndex])) {
				stealIndex = nowIndex;
				return nowIndex;
			}
		}
		// All taken = steal (stealIndex always rotates)
		stealIndex++;
		if (stealIndex > (numVo - 1))
			stealIndex = 0;
		///if ((polyMode > MPE_MODE) && (polyMode < REASSIGN_MODE) && (gates[stealIndex]))
		/// cannot reach here if polyMode == MPE mode ...no need to check
		if ((polyMode < REASSIGN_MODE) && (gates[stealIndex]))
			cachedNotes.push_back(notes[stealIndex]);
		return stealIndex;
	}

	void pressNote(uint8_t channel, uint8_t note, uint8_t vel) {
		
		// Set notes and gates
		switch (polyMode) {
			case MPE_MODE: {
				//////if gate push note to mpe_buffer for legato/////
				rotateIndex = channel - MPEfirstCh;
				if ((rotateIndex < 0) || (rotateIndex > 7)) return;
				if (gates[rotateIndex]) cachedMPE[rotateIndex].push_back(notes[rotateIndex]);
				
			} break;

			case ROTATE_MODE: {
				rotateIndex = getPolyIndex(rotateIndex);
			} break;

			case REUSE_MODE: {
				bool reuse = false;
				for (int i = 0; i < numVo; i++) {
					if (notes[i] == note) {
						rotateIndex = i;
						reuse = true;
						break;
					}
				}
				if (!reuse)
					rotateIndex = getPolyIndex(rotateIndex);
			} break;

			case RESET_MODE: {
				rotateIndex = getPolyIndex(-1);
			} break;

			case REASSIGN_MODE: {
				cachedNotes.push_back(note);
				rotateIndex = getPolyIndex(-1);
			} break;

			case UNISON_MODE: {
				cachedNotes.push_back(note);
				for (int i = 0; i < numVo; i++) {
					notes[i] = note;
					vels[i] = vel;
					gates[i] = true;
					pedalgates[i] = pedal;
					reTrigger[i].trigger(1e-3);
				}
				return;
			} break;

			default: break;
		}
		// Set notes and gates
		if (gates[rotateIndex] || pedalgates[rotateIndex])
			reTrigger[rotateIndex].trigger(1e-3);
		notes[rotateIndex] = note;
		vels[rotateIndex] = vel;
		gates[rotateIndex] = true;
		pedalgates[rotateIndex] = pedal;
	}

	void releaseNote(uint8_t channel, uint8_t note, uint8_t vel) {
		
		if (polyMode > MPE_MODE) {
		// Remove the note
		auto it = std::find(cachedNotes.begin(), cachedNotes.end(), note);
		if (it != cachedNotes.end())
			cachedNotes.erase(it);
		}else{
			int i = channel - MPEfirstCh;
			if ((i < 0) || (i > 7)) return;
			auto it = std::find(cachedMPE[i].begin(), cachedMPE[i].end(), note);
			if (it != cachedMPE[i].end())
				cachedMPE[i].erase(it);
		}

		switch (polyMode) {
			case MPE_MODE: {
				int i = channel - MPEfirstCh;
				if (note == notes[i]) {
					if (pedalgates[i]) {
						gates[i] = false;
					}
					/// check for cachednotes on MPE buffers [8]...
					else if (!cachedMPE[i].empty()) {
						notes[i] = cachedMPE[i].back();
						cachedMPE[i].pop_back();
					}
					else {
						gates[i] = false;
					}
					if (vel < 128) // 128 = from NoteOn ZeroVel
						vels[i] = vel;///Rel Vel
				}
			} break;

			case REASSIGN_MODE: {
				if (vel > 128) vel = 64;
				for (int i = 0; i < numVo; i++) {
					if (i < (int) cachedNotes.size()) {
						if (!pedalgates[i])
							notes[i] = cachedNotes[i];
						pedalgates[i] = pedal;
					}
					else {
						gates[i] = false;
						mpey[i] = vel * 128;
					}
				}
			} break;

			case UNISON_MODE: {
				if (vel > 128) vel = 64;
				if (!cachedNotes.empty()) {
					uint8_t backnote = cachedNotes.back();
					for (int i = 0; i < numVo; i++) {
						notes[i] = backnote;
						gates[i] = true;
						mpey[i] = vel * 128;
					}
				}
				else {
					for (int i = 0; i < numVo; i++) {
						gates[i] = false;
						mpey[i] = vel * 128;
					}
				}
				
			} break;

			// default ROTATE_MODE REUSE_MODE RESET_MODE
			default: {
				for (int i = 0; i < numVo; i++) {
					if (notes[i] == note) {
						if (pedalgates[i]) {
							gates[i] = false;
						}
						else if (!cachedNotes.empty()) {
							notes[i] = cachedNotes.back();
							cachedNotes.pop_back();
						}
						else {
							gates[i] = false;
						}
						if (vel < 128) // 128 = from NoteOn ZeroVel
							mpey[i] = vel * 128;
						else//Fixed RelVel
							mpey[i] = 8192;
					}
				}
			} break;
		}
	}

	void pressPedal() {
		pedal = true;
		lights[SUSTHOLD_LIGHT].value = params[SUSTHOLD_PARAM].value;
		if (polyMode == MPE_MODE) {
			for (int i = 0; i < 8; i++) {
				pedalgates[i] = gates[i];
			}
		}else {
			for (int i = 0; i < numVo; i++) {
				pedalgates[i] = gates[i];
			}
		}
	}

	void releasePedal() {
		pedal = false;
		lights[SUSTHOLD_LIGHT].value = 0.f;
		// When pedal is off, recover notes for pressed keys (if any) after they were already being "cycled" out by pedal-sustained notes.
		if (polyMode == MPE_MODE) {
			for (int i = 0; i < 8; i++) {
				pedalgates[i] = false;
				if (!cachedMPE[i].empty()) {
						notes[i] = cachedMPE[i].back();
						cachedMPE[i].pop_back();
						gates[i] = true;
				}
			}
		}else{
			for (int i = 0; i < numVo; i++) {
				pedalgates[i] = false;
				if (!cachedNotes.empty()) {
					if  (polyMode < REASSIGN_MODE){
						notes[i] = cachedNotes.back();
						cachedNotes.pop_back();
						gates[i] = true;
					}
				}
			}
			if (polyMode == REASSIGN_MODE) {
				for (int i = 0; i < numVo; i++) {
					if (i < (int) cachedNotes.size()) {
						notes[i] = cachedNotes[i];
						gates[i] = true;
					}
					else {
						gates[i] = false;
					}
				}
			}
		}
	}
	
	void onSampleRateChange() override {
		onReset();
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////
//////   STEP START
///////////////////////
	
	
	void step() override {

		MidiMessage msg;
		while (midiInput.shift(&msg)) {
			processMessage(msg);
		}

		float pbVo = 0.f;
		if (Mpit < 8192){
			pbVo = MpitFilter.process(rescale(Mpit, 0, 8192, -5.f, 0.f));
		} else {
			pbVo = MpitFilter.process(rescale(Mpit, 8192, 16383, 0.f, 5.f));
		}
//		outputs[MMA_OUTPUT].value = pbVo;
		bool sustainHold = (params[SUSTHOLD_PARAM].value > .5 );

		if (polyMode > PolyMode::MPE_MODE){
			for (int i = 0; i < numVo; i++) {
				float lastGate = ((gates[i] || (sustainHold && pedalgates[i])) && (!(reTrigger[i].process(engineGetSampleTime()))))? 10.f : 0.f;
				outputs[GATE_OUTPUT + i].value = lastGate;
				outputs[X_OUTPUT + i].value = ((notes[i] - 60) / 12.f) + (pbVo * static_cast<float>(pbMain) / 60.f);
				outputs[VEL_OUTPUT + i].value = rescale(vels[i], 0, 127, 0.f, 10.f);
				outputs[Y_OUTPUT + i].value = rescale(mpey[i], 0, 16383, 0.f, 10.f);
				outputs[Z_OUTPUT + i].value = rescale(noteData[notes[i]].aftertouch, 0, 127, 0.f, 10.f);
				lights[CH_LIGHT + i].value = ((i == rotateIndex)? 0.2f : 0.f) + (lastGate * .08f);
			}
		} else {/// MPE MODE!!!
			for (int i = 0; i < 8; i++) {
				float lastGate = ((gates[i] || (sustainHold && pedalgates[i])) && (!(reTrigger[i].process(engineGetSampleTime())))) ? 10.f : 0.f;
				outputs[GATE_OUTPUT + i].value = lastGate ;
				if ( mpex[i] < 0){
					xpitch[i] = (MPExFilter[i].process(rescale(mpex[i], -8192 , 0, -5.f, 0.f))) * pbMPE / 60.f;
				} else {
					xpitch[i] = (MPExFilter[i].process(rescale(mpex[i], 0, 8191, 0.f, 5.f))) * pbMPE / 60.f;
				}
				outputs[X_OUTPUT + i].value = xpitch[i] + ((notes[i] - 60) / 12.f) + (pbVo * static_cast<float>(pbMain) / 60.f);
				outputs[VEL_OUTPUT + i].value = rescale(vels[i], 0, 127, 0.f, 10.f);
				outputs[Y_OUTPUT + i].value = MPEyFilter[i].process(rescale(mpey[i], 0, 16383, 0.f, 10.f));
				outputs[Z_OUTPUT + i].value = MPEzFilter[i].process(rescale(mpez[i], 0, 16383, 0.f, 10.f));
				lights[CH_LIGHT + i].value = ((i == rotateIndex)? 0.2f : 0.f) + (lastGate * .08f);
			}
		}
		for (int i = 0; i < 6; i++){
			if (midiCCs[i] == 128)
				outputs[MMA_OUTPUT + i].value = pbVo;
			else if (midiCCs[i] == 129)
				outputs[MMA_OUTPUT + i].value = MCCsFilter[i].process(rescale(Maft, 0, 127, 0.f, 10.f));
			else
				outputs[MMA_OUTPUT + i].value = MCCsFilter[i].process(rescale(midiCCsVal[i], 0, 127, 0.f, 10.f));
		}

		//// PANEL KNOB AND BUTTONS
		float f_dataKnob = *dataKnob;
		if ( f_dataKnob > 0.07f){
			int knobInterval = static_cast<int>(0.05 * static_cast<float>(engineGetSampleRate()) / f_dataKnob);
			if (frameData ++ > knobInterval){
				frameData = 0;
				dataPlus();
			}
		}else if(f_dataKnob < -0.07f){
			int knobInterval = static_cast<int>(0.05 * static_cast<float>(engineGetSampleRate()) / -f_dataKnob);
			if (frameData ++ > knobInterval){
				frameData = 0;
				dataMinus();
			}
		}
		
		if (PlusOneTrigger.process(params[PLUSONE_PARAM].value)) {
			dataPlus();
			return;
		}
		if (MinusOneTrigger.process(params[MINUSONE_PARAM].value)) {
			dataMinus();
			return;
		}
		if (LcursorTrigger.process(params[LCURSOR_PARAM].value)) {
			if (polyMode == MPE_MODE){
				if (MPEmode > 0){
					if (cursorI > 0) cursorI --;
					else cursorI = 9;
					cursorIx = cursorMPEsub[cursorI];
				}else{
					if (cursorI > 0) cursorI --;
					else cursorI = 11;
					cursorIx = cursorMPE[cursorI];
				}
			}else{
				if (cursorI > 0) cursorI --;
				else cursorI = 8;
				cursorIx = cursorPoly[cursorI];
			}
			learnIx = 0;
			return;
		}
		if (RcursorTrigger.process(params[RCURSOR_PARAM].value)) {
			if (polyMode == MPE_MODE){
				if (MPEmode > 0){
					if (cursorI < 9) cursorI ++;
					else cursorI = 0;
					cursorIx = cursorMPEsub[cursorI];
				}else{
					if (cursorI < 11) cursorI ++;
					else cursorI = 0;
					cursorIx = cursorMPE[cursorI];
				}
			}else{
				if (cursorI < 8) cursorI ++;
				else cursorI = 0;
				cursorIx = cursorPoly[cursorI];
			}
			learnIx = 0;
			return;
		}
		for (int i = 0; i < 6; i++){
			if (learnCCsTrigger[i].process(params[LEARNCCA_PARAM + i].value)) {
				if (learnIx == i + 1)
					learnIx = 0;
				else{
					learnIx = i + 1;
					//cursorIx = i + 7;
				}
				return;
			}
		}
		
		///// RESET MIDI
		if (resetMidiTrigger.process(params[RESETMIDI_PARAM].value)) {
			lights[RESETMIDI_LIGHT].value= 1.0f;
			onReset();
			return;
		}
		if (lights[RESETMIDI_LIGHT].value > 0.0001f){
			lights[RESETMIDI_LIGHT].value -= 0.0001f ; // fade out light
		}
	}
///////////////////////
//////   STEP END
///////////////////////

	void dataPlus(){
		switch (cursorIx){
			case 0: {
				if (polyMode == MPE_MODE){
					if (MPEmode < 1){
						MPEmode ++;
						onReset();
					}else{//last MPE submode... go to Poly
						polyMode = (PolyMode) (1);
						onReset();
						midiInput.channel = savedMidiCh;//restore channel
					}
				}else if (polyMode < UNISON_MODE) {
					polyMode = (PolyMode) (polyMode + 1);
					onReset();
				}else {
					polyMode = MPE_MODE;
					MPEmode = 0; // no MPE submode...
					savedMidiCh = midiInput.channel;// save Poly MIDI channel
					onReset();
				}
				polyModeIx = polyMode;
					
			}break;
			case 1: {
				if (numVo < 8) numVo ++;
				//else numVo = 2;
				onReset();
			}break;
			case 2: {
				if (MPEfirstCh < 8){
					MPEfirstCh ++;
					MPEmasterCh = MPEfirstCh - 1;
				}else{
					MPEfirstCh = 0;
					MPEmasterCh = 15;
				}
				onReset();
			}break;
			case 3: {
				if (pbMain < 96) pbMain ++;
			}break;
			case 4: {
				if (pbMPE < 96) pbMPE ++;
			}break;
			case 5: {
				if (mpeYcc <128)
					mpeYcc ++;
				else
					mpeYcc = 0;
				displayYcc = mpeYcc;
			}break;
			case 6: {
				if (mpeZcc <128)
					mpeZcc ++;
				else
					mpeZcc = 0;
				displayZcc = mpeZcc;
			}break;
			default: {
				if (midiCCs[cursorIx - 7] < 129)
					midiCCs[cursorIx - 7]  ++;
				else
					midiCCs[cursorIx - 7] = 0;
			}break;
		}
		learnIx = 0;;
		return;
	}
	void dataMinus(){
		switch (cursorIx){
			case 0: {
				if (polyMode > MPE_MODE) {
					polyMode = (PolyMode) (polyMode - 1);
					MPEmode = 1;
					savedMidiCh = midiInput.channel;
					onReset();
				}else if (MPEmode > 0){
					MPEmode --;
					onReset();
				}else {//last MPE submode... go to Poly
					polyMode = UNISON_MODE;
					onReset();
					midiInput.channel = savedMidiCh;//restore channel
				}
				polyModeIx = polyMode;
			}break;
			case 1: {
				if (numVo > 2) numVo --;
				//else numVo = 8;
				onReset();
			}break;
			case 2:{
				if (MPEfirstCh > 1){
					MPEfirstCh -- ;
					MPEmasterCh = MPEfirstCh - 1;
				}else if (MPEfirstCh == 1){
					MPEfirstCh = 0;
					MPEmasterCh = 15;
				}else {
					MPEfirstCh = 8;
					MPEmasterCh = 7;
				}
				onReset();
			}break;
			case 3: {
				if (pbMain > 0) pbMain --;
			}break;
			case 4: {
				if (pbMPE > 0) pbMPE --;
			}break;
			case 5: {
				if (mpeYcc > 0)
					mpeYcc --;
				else
					mpeYcc = 128;
				displayYcc = mpeYcc;
			}break;
			case 6: {
				if (mpeZcc > 0)
					mpeZcc --;
				else
					mpeZcc = 128;
				displayZcc = mpeZcc;
			}break;
			default: {
				if (midiCCs[cursorIx - 7] > 0)
					midiCCs[cursorIx - 7] --;
				else
					midiCCs[cursorIx - 7] = 129;
			}break;
		}
		learnIx = 0;
		return;
	}
	void processMessage(MidiMessage msg) {


		switch (msg.status()) {
			// note off
			case 0x8: {
				if ((polyMode == MPE_MODE) && (msg.channel() == MPEmasterCh)) return;
				releaseNote(msg.channel(), msg.note(), msg.value());
			} break;
			// note on
			case 0x9: {
				if ((polyMode == MPE_MODE) && (msg.channel() == MPEmasterCh)) return;
				if (msg.value() > 0) {
					//noteData[msg.note()].velocity = msg.value();
					pressNote(msg.channel(), msg.note(), msg.value());
				}
				else {
					releaseNote(msg.channel(), msg.note(), 128);//128 to bypass Release vel on Vel Outputs
				}
			} break;
			// note (poly) aftertouch
			case 0xa: {
				if (polyMode == MPE_MODE) return;
				noteData[msg.note()].aftertouch = msg.value();
			} break;
				
			// channel aftertouch
			case 0xd: {
				if (learnIx > 0) {// learn enabled ???
					midiCCs[learnIx - 1] = 129;
					learnIx = 0;
					return;
				}////////////////////////////////////////
				else if (polyMode == MPE_MODE){
					if (msg.channel() == MPEmasterCh){
						Maft = msg.data1;
					}else if (MPEmode == 1){
						mpez[msg.channel() - MPEfirstCh] =  msg.data1 * 128 + mpePlusLB[msg.channel() - MPEfirstCh];
						mpePlusLB[msg.channel() - MPEfirstCh] = 0;
					}else {
						if (mpeZcc == 128)
							mpez[msg.channel() - MPEfirstCh] = msg.data1 * 128;
						if (mpeYcc == 128)
							mpey[msg.channel() - MPEfirstCh] = msg.data1 * 128;
						}
				}else{
					Maft = msg.data1;
				}
			} break;
				// pitch Bend
			case 0xe:{
				if (learnIx > 0) {// learn enabled ???
					midiCCs[learnIx - 1] = 128;
					learnIx = 0;
					return;
				}////////////////////////////////////////
				else if (polyMode == MPE_MODE){
					if (msg.channel() == MPEmasterCh){
						Mpit = msg.data2 * 128 + msg.data1;
					}else{
						mpex[msg.channel() - MPEfirstCh] = msg.data2 * 128 + msg.data1 - 8192;
					}
				}else{
					Mpit = msg.data2 * 128 + msg.data1; //14bit Pitch Bend
				}
			} break;
			// cc
			case 0xb: {
				///////// LEARN CC   ???
				if (learnIx > 0) {
					midiCCs[learnIx - 1] = msg.note();
					learnIx = 0;
					return;
				}else if (polyMode == MPE_MODE){
					if (msg.channel() == MPEmasterCh){
						processCC(msg);
					}else if (MPEmode == 1){ //Continuum
						if (msg.note() == 87){
							mpePlusLB[msg.channel() - MPEfirstCh] = msg.data2;
						}else if (msg.note() == 74){
							mpey[msg.channel() - MPEfirstCh] =  msg.data2 * 128 + mpePlusLB[msg.channel() - MPEfirstCh];
							mpePlusLB[msg.channel() - MPEfirstCh] = 0;
						}

					}else if (msg.note() == mpeYcc){
					//cc74 0x4a default
						mpey[msg.channel() - MPEfirstCh] = msg.data2 * 128;
					}else if (msg.note() == mpeZcc){
						mpez[msg.channel() - MPEfirstCh] = msg.data2 * 128;
					}
				}else{
					processCC(msg);
				}
			} break;
			default: break;
		}
	}

	void processCC(MidiMessage msg) {
		if (msg.note() ==  0x40) { //internal sust pedal
			if (msg.value() >= 64)
				pressPedal();
			else
				releasePedal();
		}
		for (int i = 0; i < 6; i++){
			if (midiCCs[i] == msg.note()){
				midiCCsVal[i] = msg.value();
				return;
			}
		}
	}
	void MidiPanic() {
		onReset();
		pedal = false;
		lights[SUSTHOLD_LIGHT].value = 0.f;
		for (int i = 0; i < 8; i++){
			notes[i] = 0;
			vels[i] = 0;
			mpex[i] = 0;
			mpey[i] = 0;
			mpez[i] = 0;
			gates[i] = false;
			xpitch[i] = {0.f};
		}
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			outputs[i].value = 0.f;
		}
	}
};
// Main Display
struct PolyModeDisplay : TransparentWidget {
	PolyModeDisplay(){
		font = Font::load(mFONT_FILE);
	}
	int pointerinit = 0;
	float mdfontSize = 12.f;
	std::string sMode ="";
	std::string sVo ="";
	std::string sPBM ="";
	std::string sPBMPE ="";
	std::string sMPEmidiCh = "";
	std::string yyDisplay = "";
	std::string zzDisplay = "";
	std::shared_ptr<Font> font;
	std::string polyModeStr[6] = {
		"M. P. E.",
		"C Y C L E",
		"R E U S E",
		"R E S E T",
		"R E A S S I G N",
		"U N I S O N",
	};
	int drawFrame = 0;
	int *p_polyMode = &pointerinit;
	int polyModeI = -1;
	int *p_numVo = &pointerinit;
	int numVoI = -1;
	int *p_pbMain = &pointerinit;
	int pbMainI = -1;
	int *p_pbMPE = &pointerinit;
	int pbMPEI = -1;
	int *p_MPEmasterCh = &pointerinit;
	int MPEmasterChI = -1;
	int *p_MPEfirstCh = &pointerinit;
	int MPEfirstChI = -1;
	int *p_MPEmode = &pointerinit;
	int MPEmodeI;
	int *p_YccNumber = &pointerinit;
	int YccNumber = -1;
	int *p_ZccNumber = &pointerinit;
	int ZccNumber = -1;
	int *p_cursorIx = &pointerinit;
	int cursorIxI = 0;
	int flashFocus = 0;
	void draw(NVGcontext* vg) {
		if (drawFrame ++ > 5){
			drawFrame = 0;
			
			if (MPEmodeI != *p_MPEmode){
				MPEmodeI = *p_MPEmode;
				//if (MPEmodeI > 1) sMode = "M. P. E. w RelVel";///
				if (MPEmodeI == 1) sMode = "M. P. E. Plus";/// Continuum Hi Res YZ
				else sMode = polyModeStr[polyModeI];
			}
			if (polyModeI !=  *p_polyMode) {
				polyModeI = *p_polyMode;
				if (polyModeI < 1) {
					if (MPEmodeI == 1) sMode = "M. P. E. Plus";/// Continuum Hi Res YZ
					else sMode = polyModeStr[polyModeI];
				}else{
					sMode = polyModeStr[polyModeI];
				}
			}

			if (numVoI != *p_numVo){
				numVoI = *p_numVo;
				sVo = "Poly "+ std::to_string(numVoI) +" Vo outs";
			}
			if (pbMainI != *p_pbMain){
				pbMainI = *p_pbMain;
				sPBM = "PBend:" + std::to_string(pbMainI);
				
			}
			if (pbMPEI != *p_pbMPE){
				pbMPEI = *p_pbMPE;
				sPBMPE = " CH PBend:" + std::to_string(pbMPEI);
			}

			if  ((MPEmasterChI != *p_MPEmasterCh) || (MPEfirstChI != *p_MPEfirstCh)){
				MPEmasterChI = *p_MPEmasterCh;
				MPEfirstChI = *p_MPEfirstCh;
				sMPEmidiCh = "channels M:" + std::to_string(MPEmasterChI + 1) + " Vo:" + std::to_string(MPEfirstChI + 1) + "++";
			}
			if  (YccNumber != *p_YccNumber){
				YccNumber = *p_YccNumber;
				switch (YccNumber) {
					case 129 :{//(locked)  Rel Vel
						yyDisplay = "rVel";
					}break;
					case 131 :{//HiRes MPE Y
						yyDisplay = "cc74+";
					}break;
					default :{
						yyDisplay = "cc" + std::to_string(YccNumber);
					}
				}
			}
			if  (ZccNumber != *p_ZccNumber){
				ZccNumber = *p_ZccNumber;
				switch (ZccNumber) {
					case 128 :{
						zzDisplay = "chnAT";
					}break;
					case 130 :{//(locked)  note AfterT
						zzDisplay = "nteAT";
					}break;
					case 132 :{//HiRes MPE Z
						zzDisplay = "chAT+";
					}break;
					default :{
						zzDisplay = "cc" + std::to_string(ZccNumber);
					}
				}
			}
			if (cursorIxI != *p_cursorIx){
				cursorIxI = *p_cursorIx;
				flashFocus = 64;
			}
		}
		nvgFontSize(vg, mdfontSize);
		nvgFontFaceId(vg, font->handle);
		nvgFillColor(vg, nvgRGB(0xcc, 0xcc, 0xcc));//Text

		//nvgGlobalCompositeOperation(vg, NVG_SOURCE_OUT);
		nvgTextAlign(vg, NVG_ALIGN_CENTER);
		nvgTextBox(vg, 4.f, 11.0f,124.f, sMode.c_str(), NULL);
	
		nvgTextBox(vg, 50.f, 52.f, 31.f, yyDisplay.c_str(), NULL);// YY
		nvgTextBox(vg, 82.f, 52.f, 31.f, zzDisplay.c_str(), NULL);// ZZ
		
		if (polyModeI < 1){
			nvgTextBox(vg, 4.f, 24.f,124.f, sMPEmidiCh.c_str(), NULL);// MPE Channels
			nvgTextAlign(vg, NVG_ALIGN_LEFT);
			nvgTextBox(vg, 58.f, 37.f,66.f, sPBMPE.c_str(), NULL);//MPE PitchBend
		} else {
			nvgTextBox(vg, 4.f, 24.f,124.f, sVo.c_str(), NULL);
		}
		
		nvgTextAlign(vg, NVG_ALIGN_LEFT);
		nvgTextBox(vg, 4.f, 37.0f, 50.f, sPBM.c_str(), NULL);
		
		nvgGlobalCompositeBlendFunc(vg,  NVG_ONE , NVG_ONE);
		
		nvgBeginPath(vg);
		switch (cursorIxI){
			case 0:{ // PolyMode
				nvgRoundedRect(vg, 1.f, 1.f, 130.f, 12.f, 3.f);
			}break;
			case 1:{ //numVoices Poly
				nvgRoundedRect(vg, 1.f, 14.f, 130.f, 12.f, 3.f);
			}break;
			case 2:{ //MPE channels
				nvgRoundedRect(vg, 1.f, 14.f, 130.f, 12.f, 3.f);
			}break;
			case 3:{//mainPB
				nvgRoundedRect(vg, 1.f, 27.f, 52.f, 12.f, 3.f);
			}break;
			case 4:{//mpePB
				nvgRoundedRect(vg, 54.f, 27.f, 77.f, 12.f, 3.f);
			}break;
			case 5:{//YY
				nvgRoundedRect(vg, 50.f, 42.f, 31, 13.f, 3.f);
			}break;
			case 6:{//ZZ
				nvgRoundedRect(vg, 82.f, 42.f, 31, 13.f, 3.f);
			}break;
		}

		if (flashFocus > 0)
			flashFocus -= 2;
		int rgbint = 0x55 + flashFocus;
		nvgFillColor(vg, nvgRGB(rgbint,rgbint,rgbint)); //SELECTED
		nvgFill(vg);
		
	}
};

struct MidiccDisplay : TransparentWidget {
	MidiccDisplay(){
		font = Font::load(mFONT_FILE);
	}
	float mdfontSize = 12.f;
	std::string sDisplay = "";
	int pointerinit = 0;
	int *p_cursor = &pointerinit;
	int cursorI = -1;
	int displayID = 0;//set on each instance
	int *p_ccNumber = &pointerinit;
	int ccNumber = -1;
	bool learnOn = false;
	bool learnChanged = false;
	int *p_learnIx = &pointerinit;
	int flashFocus = 0;
	int displayFrames = 0;
	std::shared_ptr<Font> font;
	void draw(NVGcontext* vg) {
		if(displayFrames ++ > 5){
			displayFrames = 0;
			learnOn = (displayID - 6 == *p_learnIx);
			if (learnOn){
				learnChanged = true;
				sDisplay = "LRN";
			}else if ((ccNumber != *p_ccNumber) || (learnChanged)){
				learnChanged = false;
				ccNumber = *p_ccNumber;
				switch (ccNumber) {
					case 128 :{
						sDisplay = "PBnd";
					}break;
					case 129 :{
						sDisplay = "chAT";
					}break;
					case 1 :{
						sDisplay = "Mod";
					}break;
					case 2 :{
						sDisplay = "BrC";
					}break;
					case 7 :{
						sDisplay = "Vol";
					}break;
					case 10 :{
						sDisplay = "Pan";
					}break;
					case 11 :{
						sDisplay = "Expr";
					}break;
					case 64 :{
						sDisplay = "Sust";
					}break;
					default :{
						sDisplay = "c" + std::to_string(ccNumber);
					}
				}
			}
		}
		if (learnOn) {
			nvgBeginPath(vg);
			nvgRoundedRect(vg, 0.f, 0.f, box.size.x, box.size.y,3.f);
			nvgStrokeColor(vg, nvgRGB(0xdd, 0x0, 0x0));
			nvgStroke(vg);
			nvgRoundedRect(vg, 0.f, 0.f, box.size.x, box.size.y,3.f);

			nvgFillColor(vg, nvgRGBA(0xcc, 0x0, 0x0,0x64));
			nvgFill(vg);
			///text color
			nvgFillColor(vg, nvgRGB(0xff, 0x00, 0x00));//LEARN
		}else{
			///text color
			nvgFillColor(vg, nvgRGB(0xcc, 0xcc, 0xcc));
		}
		nvgFontSize(vg, mdfontSize);
		nvgFontFaceId(vg, font->handle);
		nvgTextAlign(vg, NVG_ALIGN_CENTER);
		nvgTextBox(vg, 0.f, 10.f,box.size.x, sDisplay.c_str(), NULL);

		if (cursorI != *p_cursor){
			cursorI = *p_cursor;
			if (*p_cursor == displayID)
				flashFocus = 64;
		}
		if ((displayID == cursorI) && (!learnOn)){
			nvgGlobalCompositeBlendFunc(vg,  NVG_ONE , NVG_ONE);
			nvgBeginPath(vg);
			nvgRoundedRect(vg, 0.f, 0.f, box.size.x, box.size.y,3.f);
//			nvgStrokeColor(vg, nvgRGB(0x66, 0x66, 0x66));
//			nvgStroke(vg);
			if (flashFocus > 0)
				flashFocus -= 2;
			int rgbint = 0x55 + flashFocus;
			nvgFillColor(vg, nvgRGB(rgbint,rgbint,rgbint)); //SELECTED
			nvgFill(vg);
		}
	}
};


struct BlockChannel : OpaqueWidget {
	int *p_polyMode;
	void draw(NVGcontext* vg) {
		if ( *p_polyMode > 0) {
				box.size = Vec(0.f,0.f);
			}else{
				box.size = Vec(94.f,13.f);
				NVGcolor ledColor = nvgRGBA(0x00, 0x00, 0x00,0xaa);
				nvgBeginPath(vg);
				nvgRoundedRect(vg, 0.f, 0.f, 94.f, 13.f,3.f);
				nvgFillColor(vg, ledColor);
				nvgFill(vg);
			}
	}
};

///MIDIlearnMCC
struct learnMccButton : SVGSwitch, MomentarySwitch {
	learnMccButton() {
		box.size = Vec(26, 13);
		addFrame(SVG::load(assetPlugin(plugin, "res/learnMcc_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/learnMcc_1.svg")));
	}
};


struct springDataKnob : SVGKnob {
		int *p_frameData;

	springDataKnob() {
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/dataKnob.svg")));
		shadow->opacity = 0.f;
	}
		void onMouseUp(EventMouseUp &e){
			this->value = 0.f;
			*p_frameData = 100000; //reset frame Counter to start (over sampleRate counter)
		}
};

struct TranspOffRedLight : TranspOffLight {
	TranspOffRedLight() {
		addBaseColor(nvgRGBA(0xff, 0x00, 0x00, 0x88));//borderColor = nvgRGBA(0, 0, 0, 0x60);
	}
};


struct MIDI8MPEWidget : ModuleWidget {
	MIDI8MPEWidget(MIDI8MPE *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin,"res/MIDI8MPE.svg")));
		//Screws
		addChild(Widget::create<ScrewBlack>(Vec(0, 0)));
		addChild(Widget::create<ScrewBlack>(Vec(180, 0)));
		addChild(Widget::create<ScrewBlack>(Vec(0, 365)));
		addChild(Widget::create<ScrewBlack>(Vec(180, 365)));
		
		float xPos = 8.f;//61;
		float yPos = 18.f;
		{
			MidiWidget *midiWidget = Widget::create<MidiWidget>(Vec(xPos,yPos));
			midiWidget->box.size = Vec(132.f,41.f);
			midiWidget->midiIO = &module->midiInput;
			
			midiWidget->driverChoice->box.size.y = 12.f;
			midiWidget->deviceChoice->box.size.y = 12.f;
			midiWidget->channelChoice->box.size.y = 12.f;

			midiWidget->driverChoice->box.pos = Vec(0.f, 2.f);
			midiWidget->deviceChoice->box.pos = Vec(0.f, 15.f);
			midiWidget->channelChoice->box.pos = Vec(0.f, 28.f);

			midiWidget->driverSeparator->box.pos = Vec(0.f, 15.f);
			midiWidget->deviceSeparator->box.pos = Vec(0.f, 28.f);

			midiWidget->driverChoice->font = Font::load(mFONT_FILE);
			midiWidget->deviceChoice->font = Font::load(mFONT_FILE);
			midiWidget->channelChoice->font = Font::load(mFONT_FILE);

			midiWidget->driverChoice->textOffset = Vec(2.f,10.f);
			midiWidget->deviceChoice->textOffset = Vec(2.f,10.f);
			midiWidget->channelChoice->textOffset = Vec(2.f,10.f);
			
			midiWidget->driverChoice->color = nvgRGB(0xcc, 0xcc, 0xcc);
			midiWidget->deviceChoice->color = nvgRGB(0xcc, 0xcc, 0xcc);
			midiWidget->channelChoice->color = nvgRGB(0xcc, 0xcc, 0xcc);
			addChild(midiWidget);
		}
		BlockChannel *blockChannel = Widget::create<BlockChannel>(Vec(8.f,46.f));
		blockChannel->p_polyMode = &(module->polyModeIx);
		addChild(blockChannel);
		
		xPos = 102.f;
		yPos = 47.f;
		addParam(ParamWidget::create<moDllzMidiPanic>(Vec(xPos, yPos), module, MIDI8MPE::RESETMIDI_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(xPos+3.f, yPos+3.f), module, MIDI8MPE::RESETMIDI_LIGHT));
		
		xPos = 8.f;
		yPos = 62.f;
		
		{
			PolyModeDisplay *polyModeDisplay = new PolyModeDisplay();
			polyModeDisplay->box.pos = Vec(xPos, yPos);
			polyModeDisplay->box.size = {132.f, 54.f};
			polyModeDisplay->p_polyMode = &(module->polyModeIx);
			polyModeDisplay->p_MPEmode = &(module->MPEmode);
			polyModeDisplay->p_numVo = &(module->numVo);
			polyModeDisplay->p_pbMain = &(module->pbMain);
			polyModeDisplay->p_pbMPE = &(module->pbMPE);
			polyModeDisplay->p_MPEmasterCh = &(module->MPEmasterCh);
			polyModeDisplay->p_MPEfirstCh = &(module->MPEfirstCh);
			polyModeDisplay->p_YccNumber = &(module->displayYcc);
			polyModeDisplay->p_ZccNumber = &(module->displayZcc);
			polyModeDisplay->p_cursorIx = &(module->cursorIx);
			addChild(polyModeDisplay);
		}
		
		yPos = 20.f;
		xPos = 145.f;
		addParam(ParamWidget::create<minusButton>(Vec(xPos, yPos), module, MIDI8MPE::MINUSONE_PARAM, 0.0f, 1.0f, 0.0f));
		xPos = 169.f;
		addParam(ParamWidget::create<plusButton>(Vec(xPos, yPos), module, MIDI8MPE::PLUSONE_PARAM, 0.0f, 1.0f, 0.0f));

		
		xPos = 147.f;
		yPos = 40.f;
////DATA KNOB
		
		{   springDataKnob *sDataKnob = new springDataKnob();
			sDataKnob->box.pos = Vec(xPos, yPos);
			sDataKnob->box.size = {36.f, 36.f};
			sDataKnob->minValue = -1.f;
			sDataKnob->maxValue = 1.f;
			sDataKnob->defaultValue = 0.f;
			sDataKnob->p_frameData = &(module->frameData);
			module->dataKnob = &(sDataKnob->value);
			addChild(sDataKnob);
		}
		
		yPos = 85.f;
		xPos = 145.5f;
		addParam(ParamWidget::create<moDllzcursorL>(Vec(xPos, yPos), module, MIDI8MPE::LCURSOR_PARAM, 0.0f, 1.0f, 0.0f));
		xPos = 165.5f;
		addParam(ParamWidget::create<moDllzcursorR>(Vec(xPos, yPos), module, MIDI8MPE::RCURSOR_PARAM, 0.0f, 1.0f, 0.0f));

//		yPos = 104.f;
//		xPos = 59.f;
//		{
//			MidiccDisplay *mpeYDisplay = new MidiccDisplay();
//			mpeYDisplay->box.pos = Vec(xPos, yPos);
//			mpeYDisplay->box.size = {29.5f, 13.f};
//			mpeYDisplay->displayID = 5;
//			mpeYDisplay->p_cursor = &(module->cursorIx);
//			mpeYDisplay->p_ccNumber = &(module->displayYcc);
//			mpeYDisplay->p_learnOn = &(module->learnYcc);
//			addChild(mpeYDisplay);
//		}
//
//		addParam(ParamWidget::create<learnMccButton>(Vec(xPos, yPos), module, MIDI8MPE::LEARNCCY_PARAM, 0.0, 1.0, 0.0));
//		xPos = 89.5f;
//		{
//			MidiccDisplay *mpeZDisplay = new MidiccDisplay();
//			mpeZDisplay->box.pos = Vec(xPos, yPos);
//			mpeZDisplay->box.size = {29.5f, 13.f};
//			mpeZDisplay->displayID = 6;
//			mpeZDisplay->p_cursor = &(module->cursorIx);
//			mpeZDisplay->p_ccNumber = &(module->displayZcc);
//			mpeZDisplay->p_learnOn = &(module->learnZcc);
//			addChild(mpeZDisplay);
//		}
//		addParam(ParamWidget::create<learnMccButton>(Vec(xPos, yPos), module, MIDI8MPE::LEARNCCZ_PARAM, 0.0, 1.0, 0.0));
		
		
		
		yPos = 118.f;
		float const xOffset = 32.f;
		for (int i = 0; i < 8; i++){
			xPos = 30.f;
			addChild(ModuleLightWidget::create<TinyLight<RedLight>>(Vec(xPos-7.f, yPos+10.f), module, MIDI8MPE::CH_LIGHT + i));
			addOutput(Port::create<moDllzPortDark>(Vec(xPos, yPos), Port::OUTPUT, module, MIDI8MPE::X_OUTPUT + i));
			xPos += xOffset;
			addOutput(Port::create<moDllzPortDark>(Vec(xPos, yPos), Port::OUTPUT, module, MIDI8MPE::Y_OUTPUT + i));
			xPos += xOffset;
			addOutput(Port::create<moDllzPortDark>(Vec(xPos, yPos), Port::OUTPUT, module, MIDI8MPE::Z_OUTPUT + i));
			xPos += xOffset;
			addOutput(Port::create<moDllzPortDark>(Vec(xPos, yPos), Port::OUTPUT, module, MIDI8MPE::VEL_OUTPUT + i));
			xPos += xOffset;
			addOutput(Port::create<moDllzPortDark>(Vec(xPos, yPos), Port::OUTPUT, module, MIDI8MPE::GATE_OUTPUT + i));
			yPos += 25.f;
		}
		yPos = 336.f;
		xPos = 10.5f;
		for ( int i = 0; i < 6; i++){
			addOutput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::OUTPUT, module, MIDI8MPE::MMA_OUTPUT + i));
			xPos += 27.f;
		}
		
		yPos = 322.f;
		xPos = 9.f;
		{
			MidiccDisplay *MccADisplay = new MidiccDisplay();
			MccADisplay->box.pos = Vec(xPos, yPos);
			MccADisplay->box.size = {26.f, 13.f};
			MccADisplay->displayID = 7;
			MccADisplay->p_cursor = &(module->cursorIx);
			MccADisplay->p_ccNumber = &(module->midiCCs[0]);
			MccADisplay->p_learnIx = &(module->learnIx);
			addChild(MccADisplay);
		}
		addParam(ParamWidget::create<learnMccButton>(Vec(xPos, yPos), module, MIDI8MPE::LEARNCCA_PARAM, 0.0, 1.0, 0.0));
		xPos += 27.f;
		{
			MidiccDisplay *MccBDisplay = new MidiccDisplay();
			MccBDisplay->box.pos = Vec(xPos, yPos);
			MccBDisplay->box.size = {26.f, 13.f};
			MccBDisplay->displayID = 8;
			MccBDisplay->p_cursor = &(module->cursorIx);
			MccBDisplay->p_ccNumber = &(module->midiCCs[1]);
			MccBDisplay->p_learnIx = &(module->learnIx);
			addChild(MccBDisplay);
		}
		addParam(ParamWidget::create<learnMccButton>(Vec(xPos, yPos), module, MIDI8MPE::LEARNCCB_PARAM, 0.0, 1.0, 0.0));
		xPos += 27.f;
		{
			MidiccDisplay *MccCDisplay = new MidiccDisplay();
			MccCDisplay->box.pos = Vec(xPos, yPos);
			MccCDisplay->box.size = {26.f, 13.f};
			MccCDisplay->displayID = 9;
			MccCDisplay->p_cursor = &(module->cursorIx);
			MccCDisplay->p_ccNumber = &(module->midiCCs[2]);
			MccCDisplay->p_learnIx = &(module->learnIx);
			addChild(MccCDisplay);
		}
		addParam(ParamWidget::create<learnMccButton>(Vec(xPos, yPos), module, MIDI8MPE::LEARNCCC_PARAM, 0.0, 1.0, 0.0));
		xPos += 27.f;
		{
			MidiccDisplay *MccDDisplay = new MidiccDisplay();
			MccDDisplay->box.pos = Vec(xPos, yPos);
			MccDDisplay->box.size = {26.f, 13.f};
			MccDDisplay->displayID = 10;
			MccDDisplay->p_cursor = &(module->cursorIx);
			MccDDisplay->p_ccNumber = &(module->midiCCs[3]);
			MccDDisplay->p_learnIx = &(module->learnIx);
			addChild(MccDDisplay);
		}
		addParam(ParamWidget::create<learnMccButton>(Vec(xPos, yPos), module, MIDI8MPE::LEARNCCD_PARAM, 0.0, 1.0, 0.0));
		xPos += 27.f;
		{
			MidiccDisplay *MccEDisplay = new MidiccDisplay();
			MccEDisplay->box.pos = Vec(xPos, yPos);
			MccEDisplay->box.size = {26.f, 13.f};
			MccEDisplay->displayID = 11;
			MccEDisplay->p_cursor = &(module->cursorIx);
			MccEDisplay->p_ccNumber = &(module->midiCCs[4]);
			MccEDisplay->p_learnIx = &(module->learnIx);
			addChild(MccEDisplay);
		}
		addParam(ParamWidget::create<learnMccButton>(Vec(xPos, yPos), module, MIDI8MPE::LEARNCCE_PARAM, 0.0, 1.0, 0.0));
		xPos += 27.f;
		{
			MidiccDisplay *MccFDisplay = new MidiccDisplay();
			MccFDisplay->box.pos = Vec(xPos, yPos);
			MccFDisplay->box.size = {26.f, 13.f};
			MccFDisplay->displayID = 12;
			MccFDisplay->p_cursor = &(module->cursorIx);
			MccFDisplay->p_ccNumber = &(module->midiCCs[5]);
			MccFDisplay->p_learnIx = &(module->learnIx);
			addChild(MccFDisplay);
		}
		addParam(ParamWidget::create<learnMccButton>(Vec(xPos, yPos), module, MIDI8MPE::LEARNCCF_PARAM, 0.0, 1.0, 0.0));
		
		
		///Sustain hold notes		
		xPos = 173.f;
		yPos = 338.f;
		addParam(ParamWidget::create<moDllzSwitchLed>(Vec(xPos, yPos), module, MIDI8MPE::SUSTHOLD_PARAM, 0.0, 1.0, 1.0));
		addChild(ModuleLightWidget::create<TranspOffRedLight>(Vec(xPos, yPos), module, MIDI8MPE::SUSTHOLD_LIGHT));
		
		
//		{
//		        testDisplay *mDisplay = new testDisplay();
//		        mDisplay->box.pos = Vec(0.0f, 360.0f);
//		        mDisplay->box.size = {165.0f, 20.0f};
//		        mDisplay->valP = module->dataKnob;
//		        addChild(mDisplay);
//		    }
	}
};

} // namespace rack_plugin_moDllz

using namespace rack_plugin_moDllz;

RACK_PLUGIN_MODEL_INIT(moDllz, MIDI8MPE) {
   Model *modelMIDI8MPE = Model::create<MIDI8MPE, MIDI8MPEWidget>("moDllz", "MIDI8MPE", "MIDI 8cv MPE", MIDI_TAG, EXTERNAL_TAG, MULTIPLE_TAG);
   return modelMIDI8MPE;
}
