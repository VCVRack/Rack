//***********************************************************************************************
//Impromptu Modular: Modules for VCV Rack by Marc Boulé
//***********************************************************************************************


#include "FoundrySequencer.hpp"

namespace rack_plugin_ImpromptuModular {

void Sequencer::construct(bool* _holdTiedNotesPtr, int* _velocityModePtr) {// don't want regaular constructor mechanism
	velocityModePtr = _velocityModePtr;
	sek[0].construct(0, nullptr, _holdTiedNotesPtr);
	for (int trkn = 1; trkn < NUM_TRACKS; trkn++)
		sek[trkn].construct(trkn, &sek[0], _holdTiedNotesPtr);
}


void Sequencer::setVelocityVal(int trkn, int intVel, int multiStepsCount, bool multiTracks) {
	sek[trkn].setVelocityVal(seqIndexEdit, stepIndexEdit, intVel, multiStepsCount);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trkn) continue;
			sek[i].setVelocityVal(seqIndexEdit, stepIndexEdit, intVel, multiStepsCount);
		}
	}
}
void Sequencer::setLength(int length, bool multiTracks) {
	sek[trackIndexEdit].setLength(seqIndexEdit, length);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setLength(seqIndexEdit, length);
		}
	}
}
void Sequencer::setBegin(bool multiTracks) {
	sek[trackIndexEdit].setBegin(phraseIndexEdit);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setBegin(phraseIndexEdit);
		}
	}
}
void Sequencer::setEnd(bool multiTracks) {
	sek[trackIndexEdit].setEnd(phraseIndexEdit);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setEnd(phraseIndexEdit);
		}
	}
}
bool Sequencer::setGateType(int keyn, int multiSteps, float sampleRate, bool autostepClick, bool multiTracks) {// Third param is for right-click autostep. Returns success
	int newMode = keyIndexToGateTypeEx(keyn);
	if (newMode == -1) 
		return false;
	sek[trackIndexEdit].setGateType(seqIndexEdit, stepIndexEdit, newMode, multiSteps);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setGateType(seqIndexEdit, stepIndexEdit, newMode, multiSteps);
		}
	}
	if (autostepClick){ // if right-click then move to next step
		moveStepIndexEdit(1, false);
		editingGateKeyLight = keyn;
		editingType = (unsigned long) (gateTime * sampleRate / displayRefreshStepSkips);
		if (windowIsModPressed() && multiSteps < 2)
			setGateType(keyn, 1, sampleRate, false, multiTracks);
	}
	return true;
}


void Sequencer::initSlideVal(int multiStepsCount, bool multiTracks) {
	sek[trackIndexEdit].setSlideVal(seqIndexEdit, stepIndexEdit, StepAttributes::INIT_SLIDE, multiStepsCount);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setSlideVal(seqIndexEdit, stepIndexEdit, StepAttributes::INIT_SLIDE, multiStepsCount);
		}
	}		
}
void Sequencer::initGatePVal(int multiStepsCount, bool multiTracks) {
	sek[trackIndexEdit].setGatePVal(seqIndexEdit, stepIndexEdit, StepAttributes::INIT_PROB, multiStepsCount);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setGatePVal(seqIndexEdit, stepIndexEdit, StepAttributes::INIT_PROB, multiStepsCount);
		}
	}		
}
void Sequencer::initVelocityVal(int multiStepsCount, bool multiTracks) {
	sek[trackIndexEdit].setVelocityVal(seqIndexEdit, stepIndexEdit, StepAttributes::INIT_VELOCITY, multiStepsCount);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setVelocityVal(seqIndexEdit, stepIndexEdit, StepAttributes::INIT_VELOCITY, multiStepsCount);
		}
	}		
}
void Sequencer::initPulsesPerStep(bool multiTracks) {
	sek[trackIndexEdit].initPulsesPerStep();
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].initPulsesPerStep();
		}
	}		
}
void Sequencer::initDelay(bool multiTracks) {
	sek[trackIndexEdit].initDelay();
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].initDelay();
		}
	}		
}
void Sequencer::initRunModeSong(bool multiTracks) {
	sek[trackIndexEdit].setRunModeSong(SequencerKernel::MODE_FWD);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setRunModeSong(SequencerKernel::MODE_FWD);
		}
	}		
}
void Sequencer::initRunModeSeq(bool multiTracks) {
	sek[trackIndexEdit].setRunModeSeq(seqIndexEdit, SequencerKernel::MODE_FWD);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setRunModeSeq(seqIndexEdit, SequencerKernel::MODE_FWD);
		}
	}		
}
void Sequencer::initLength(bool multiTracks) {
	sek[trackIndexEdit].setLength(seqIndexEdit, SequencerKernel::MAX_STEPS);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setLength(seqIndexEdit, SequencerKernel::MAX_STEPS);
		}
	}		
}
void Sequencer::initPhraseReps(bool multiTracks) {
	sek[trackIndexEdit].setPhraseReps(phraseIndexEdit, 1);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setPhraseReps(phraseIndexEdit, 1);
		}
	}		
}
void Sequencer::initPhraseSeqNum(bool multiTracks) {
	sek[trackIndexEdit].setPhraseSeqNum(phraseIndexEdit, 0);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setPhraseSeqNum(phraseIndexEdit, 0);
		}
	}		
}

void Sequencer::copySequence(int countCP) {
	int startCP = stepIndexEdit;
	sek[trackIndexEdit].copySequence(&seqCPbuf, seqIndexEdit, startCP, countCP);
}
void Sequencer::pasteSequence(bool multiTracks) {
	int startCP = stepIndexEdit;
	sek[trackIndexEdit].pasteSequence(&seqCPbuf, seqIndexEdit, startCP);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].pasteSequence(&seqCPbuf, seqIndexEdit, startCP);
		}
	}
}
void Sequencer::copySong(int startCP, int countCP) {
	sek[trackIndexEdit].copySong(&songCPbuf, startCP, countCP);
}
void Sequencer::pasteSong(bool multiTracks) {
	sek[trackIndexEdit].pasteSong(&songCPbuf, phraseIndexEdit);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].pasteSong(&songCPbuf, phraseIndexEdit);
		}
	}
}


void Sequencer::writeCV(int trkn, float cvVal, int multiStepsCount, float sampleRate, bool multiTracks) {
	sek[trkn].writeCV(seqIndexEdit, stepIndexEdit, cvVal, multiStepsCount);
	editingGateCV[trkn] = cvVal;
	editingGateCV2[trkn] = sek[trkn].getVelocityVal(seqIndexEdit, stepIndexEdit);
	editingGate[trkn] = (unsigned long) (gateTime * sampleRate / displayRefreshStepSkips);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trkn) continue;
			sek[i].writeCV(seqIndexEdit, stepIndexEdit, cvVal, multiStepsCount);
		}
	}
}
void Sequencer::autostep(bool autoseq, bool autostepLen) {
	moveStepIndexEdit(1, autostepLen);
	if (stepIndexEdit == 0 && autoseq)
		seqIndexEdit = moveIndex(seqIndexEdit, seqIndexEdit + 1, SequencerKernel::MAX_SEQS);	
}	

bool Sequencer::applyNewOctave(int octn, int multiSteps, float sampleRate, bool multiTracks) { // returns true if tied
	if (sek[trackIndexEdit].getTied(seqIndexEdit, stepIndexEdit))
		return true;
	editingGateCV[trackIndexEdit] = sek[trackIndexEdit].applyNewOctave(seqIndexEdit, stepIndexEdit, octn, multiSteps);
	editingGateCV2[trackIndexEdit] = sek[trackIndexEdit].getVelocityVal(seqIndexEdit, stepIndexEdit);
	editingGate[trackIndexEdit] = (unsigned long) (gateTime * sampleRate / displayRefreshStepSkips);
	editingGateKeyLight = -1;
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].applyNewOctave(seqIndexEdit, stepIndexEdit, octn, multiSteps);
		}
	}
	return false;
}
bool Sequencer::applyNewKey(int keyn, int multiSteps, float sampleRate, bool autostepClick, bool multiTracks) { // returns true if tied
	bool ret = false;
	if (sek[trackIndexEdit].getTied(seqIndexEdit, stepIndexEdit)) {
		if (autostepClick)
			moveStepIndexEdit(1, false);
		else
			ret = true;
	}
	else {
		editingGateCV[trackIndexEdit] = sek[trackIndexEdit].applyNewKey(seqIndexEdit, stepIndexEdit, keyn, multiSteps);
		editingGateCV2[trackIndexEdit] = sek[trackIndexEdit].getVelocityVal(seqIndexEdit, stepIndexEdit);
		editingGate[trackIndexEdit] = (unsigned long) (gateTime * sampleRate / displayRefreshStepSkips);
		editingGateKeyLight = -1;
		if (multiTracks) {
			for (int i = 0; i < NUM_TRACKS; i++) {
				if (i == trackIndexEdit) continue;
				sek[i].applyNewKey(seqIndexEdit, stepIndexEdit, keyn, multiSteps);
			}
		}
		if (autostepClick) {// if right-click then move to next step
			moveStepIndexEdit(1, false);
			if (windowIsModPressed() && multiSteps < 2) // if ctrl-right-click and SEL is off
				writeCV(trackIndexEdit, editingGateCV[trackIndexEdit], 1, sampleRate, multiTracks);// copy CV only to next step
			editingGateKeyLight = keyn;
		}
	}
	return ret;
}

void Sequencer::moveStepIndexEditWithEditingGate(int delta, bool writeTrig, float sampleRate) {
	moveStepIndexEdit(delta, false);
	for (int trkn = 0; trkn < NUM_TRACKS; trkn++) {
		if (!sek[trkn].getTied(seqIndexEdit, stepIndexEdit)) {// play if non-tied step
			if (!writeTrig) {// in case autostep when simultaneous writeCV and stepCV (keep what was done in Write Input block above)
				editingGate[trkn] = (unsigned long) (gateTime * sampleRate / displayRefreshStepSkips);
				editingGateCV[trkn] = sek[trkn].getCV(seqIndexEdit, stepIndexEdit);
				editingGateCV2[trkn] = sek[trkn].getVelocityVal(seqIndexEdit, stepIndexEdit);
				editingGateKeyLight = -1;
			}
		}
	}
}



void Sequencer::modSlideVal(int deltaVelKnob, int mutliStepsCount, bool multiTracks) {
	int sVal = sek[trackIndexEdit].modSlideVal(seqIndexEdit, stepIndexEdit, deltaVelKnob, mutliStepsCount);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setSlideVal(seqIndexEdit, stepIndexEdit, sVal, mutliStepsCount);
		}
	}		
}
void Sequencer::modGatePVal(int deltaVelKnob, int mutliStepsCount, bool multiTracks) {
	int gpVal = sek[trackIndexEdit].modGatePVal(seqIndexEdit, stepIndexEdit, deltaVelKnob, mutliStepsCount);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setGatePVal(seqIndexEdit, stepIndexEdit, gpVal, mutliStepsCount);
		}
	}		
}
void Sequencer::modVelocityVal(int deltaVelKnob, int mutliStepsCount, bool multiTracks) {
	int upperLimit = ((*velocityModePtr) == 0 ? 200 : 127);
	int vVal = sek[trackIndexEdit].modVelocityVal(seqIndexEdit, stepIndexEdit, deltaVelKnob, upperLimit, mutliStepsCount);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setVelocityVal(seqIndexEdit, stepIndexEdit, vVal, mutliStepsCount);
		}
	}		
}
void Sequencer::modRunModeSong(int deltaPhrKnob, bool multiTracks) {
	int newRunMode = sek[trackIndexEdit].modRunModeSong(deltaPhrKnob);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setRunModeSong(newRunMode);
		}
	}		
}
void Sequencer::modPulsesPerStep(int deltaSeqKnob, bool multiTracks) {
	int newPPS = sek[trackIndexEdit].modPulsesPerStep(deltaSeqKnob);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setPulsesPerStep(newPPS);
		}
	}		
}
void Sequencer::modDelay(int deltaSeqKnob, bool multiTracks) {
	int newDelay = sek[trackIndexEdit].modDelay(deltaSeqKnob);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setDelay(newDelay);
		}
	}		
}
void Sequencer::modRunModeSeq(int deltaSeqKnob, bool multiTracks) {
	int newRunMode = sek[trackIndexEdit].modRunModeSeq(seqIndexEdit, deltaSeqKnob);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setRunModeSeq(seqIndexEdit, newRunMode);
		}
	}		
}
void Sequencer::modLength(int deltaSeqKnob, bool multiTracks) {
	int newLength = sek[trackIndexEdit].modLength(seqIndexEdit, deltaSeqKnob);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setLength(seqIndexEdit, newLength);
		}
	}		
}
void Sequencer::modPhraseReps(int deltaSeqKnob, bool multiTracks) {
	int newReps = sek[trackIndexEdit].modPhraseReps(phraseIndexEdit, deltaSeqKnob);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setPhraseReps(phraseIndexEdit, newReps);
		}
	}		
}
void Sequencer::modPhraseSeqNum(int deltaSeqKnob, bool multiTracks) {
	int newSeqn = sek[trackIndexEdit].modPhraseSeqNum(phraseIndexEdit, deltaSeqKnob);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].setPhraseSeqNum(phraseIndexEdit, newSeqn);
		}
	}		
}
void Sequencer::transposeSeq(int deltaSeqKnob, bool multiTracks) {
	sek[trackIndexEdit].transposeSeq(seqIndexEdit, deltaSeqKnob);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].transposeSeq(seqIndexEdit, deltaSeqKnob);
		}
	}		
}
void Sequencer::unTransposeSeq(bool multiTracks) {
	sek[trackIndexEdit].unTransposeSeq(seqIndexEdit);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].unTransposeSeq(seqIndexEdit);
		}
	}		
}
void Sequencer::rotateSeq(int deltaSeqKnob, bool multiTracks) {
	sek[trackIndexEdit].rotateSeq(seqIndexEdit, deltaSeqKnob);
	if (stepIndexEdit < getLength())
		moveStepIndexEdit(deltaSeqKnob, true);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].rotateSeq(seqIndexEdit, deltaSeqKnob);
		}
	}		
}
void Sequencer::unRotateSeq(bool multiTracks) {
	sek[trackIndexEdit].unRotateSeq(seqIndexEdit);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;
			sek[i].unRotateSeq(seqIndexEdit);
		}
	}		
}
void Sequencer::toggleGate(int multiSteps, bool multiTracks) {
	bool newGate = sek[trackIndexEdit].toggleGate(seqIndexEdit, stepIndexEdit, multiSteps);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;			
			sek[i].setGate(seqIndexEdit, stepIndexEdit, newGate, multiSteps);
		}
	}		
}
bool Sequencer::toggleGateP(int multiSteps, bool multiTracks) { // returns true if tied
	if (sek[trackIndexEdit].getTied(seqIndexEdit,stepIndexEdit))
		return true;
	bool newGateP = sek[trackIndexEdit].toggleGateP(seqIndexEdit, stepIndexEdit, multiSteps);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;			
			sek[i].setGateP(seqIndexEdit, stepIndexEdit, newGateP, multiSteps);
		}
	}				
	return false;
}
bool Sequencer::toggleSlide(int multiSteps, bool multiTracks) { // returns true if tied
	if (sek[trackIndexEdit].getTied(seqIndexEdit,stepIndexEdit))
		return true;
	bool newSlide = sek[trackIndexEdit].toggleSlide(seqIndexEdit, stepIndexEdit, multiSteps);
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;			
			sek[i].setSlide(seqIndexEdit, stepIndexEdit, newSlide, multiSteps);
		}
	}				
	return false;
}
void Sequencer::toggleTied(int multiSteps, bool multiTracks) {
	bool newTied = sek[trackIndexEdit].toggleTied(seqIndexEdit, stepIndexEdit, multiSteps);// will clear other attribs if new state is on
	if (multiTracks) {
		for (int i = 0; i < NUM_TRACKS; i++) {
			if (i == trackIndexEdit) continue;			
			sek[i].setTied(seqIndexEdit, stepIndexEdit, newTied, multiSteps);
		}
	}						
}


void Sequencer::toJson(json_t *rootJ) {
	// stepIndexEdit
	json_object_set_new(rootJ, "stepIndexEdit", json_integer(stepIndexEdit));

	// seqIndexEdit
	json_object_set_new(rootJ, "seqIndexEdit", json_integer(seqIndexEdit));

	// phraseIndexEdit
	json_object_set_new(rootJ, "phraseIndexEdit", json_integer(phraseIndexEdit));

	// trackIndexEdit
	json_object_set_new(rootJ, "trackIndexEdit", json_integer(trackIndexEdit));

	for (int trkn = 0; trkn < NUM_TRACKS; trkn++)
		sek[trkn].toJson(rootJ);
}


void Sequencer::fromJson(json_t *rootJ) {
	// stepIndexEdit
	json_t *stepIndexEditJ = json_object_get(rootJ, "stepIndexEdit");
	if (stepIndexEditJ)
		stepIndexEdit = json_integer_value(stepIndexEditJ);
	
	// phraseIndexEdit
	json_t *phraseIndexEditJ = json_object_get(rootJ, "phraseIndexEdit");
	if (phraseIndexEditJ)
		phraseIndexEdit = json_integer_value(phraseIndexEditJ);
	
	// seqIndexEdit
	json_t *seqIndexEditJ = json_object_get(rootJ, "seqIndexEdit");
	if (seqIndexEditJ)
		seqIndexEdit = json_integer_value(seqIndexEditJ);
	
	// trackIndexEdit
	json_t *trackIndexEditJ = json_object_get(rootJ, "trackIndexEdit");
	if (trackIndexEditJ)
		trackIndexEdit = json_integer_value(trackIndexEditJ);
	
	for (int trkn = 0; trkn < NUM_TRACKS; trkn++)
		sek[trkn].fromJson(rootJ);
}


void Sequencer::reset() {
	stepIndexEdit = 0;
	phraseIndexEdit = 0;
	seqIndexEdit = 0;
	trackIndexEdit = 0;
	seqCPbuf.reset();
	songCPbuf.reset();

	for (int trkn = 0; trkn < NUM_TRACKS; trkn++) {
		editingGate[trkn] = 0ul;
		sek[trkn].reset();
	}
	editingType = 0ul;
}


void Sequencer::clockStep(int trkn) {
	bool phraseChange = sek[trkn].clockStep();
	if (trkn == 0 && phraseChange) {
		for (int tkbcd = 1; tkbcd < NUM_TRACKS; tkbcd++) {// check for song run mode slaving
			if (sek[tkbcd].getRunModeSong() == SequencerKernel::MODE_TKA) {
				sek[tkbcd].setPhraseIndexRun(sek[0].getPhraseIndexRun());
				// The code below is to make it such that stepIndexRun should re-init upon phraseChange
				//   example for phrase jump does not reset stepIndexRun in B
					//A (FWD): 1 = 1x10, 2 = 1x10
					//B (TKA): 1 = 1x20, 2 = 1x20
				// next line will not work, it will result in a double move of stepIndexRun since clock will move it also
				//sek[tkbcd].moveStepIndexRun(true); 
				// this next mechanism works, the chain of events will make moveStepIndexRun(true) happen automatically and no double move will occur
				sek[tkbcd].setMoveStepIndexRunIgnore();// 
			}
		}
	}
}

} // namespace rack_plugin_ImpromptuModular


