//***********************************************************************************************
//Impromptu Modular: Modules for VCV Rack by Marc Boulé
//***********************************************************************************************


#include "FoundrySequencerKernel.hpp"

namespace rack_plugin_ImpromptuModular {

const std::string SequencerKernel::modeLabels[NUM_MODES] = {"FWD", "REV", "PPG", "PEN", "BRN", "RND", "TKA"};


const uint64_t SequencerKernel::advGateHitMaskLow[NUM_GATES] = 
{0x0000000000FFFFFF, 0x0000FFFF0000FFFF, 0x0000FFFFFFFFFFFF, 0x0000FFFF00000000, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 
//				25%					TRI		  			50%					T23		  			75%					FUL		
 0x000000000000FFFF, 0xFFFF000000FFFFFF, 0x0000FFFF00000000, 0xFFFF000000000000, 0x0000000000000000, 0};
//  			TR1 				DUO		  			TR2 	     		D2		  			TR3  TRIG		
const uint64_t SequencerKernel::advGateHitMaskHigh[NUM_GATES] = 
{0x0000000000000000, 0x000000000000FFFF, 0x0000000000000000, 0x000000000000FFFF, 0x00000000000000FF, 0x00000000FFFFFFFF, 
//				25%					TRI		  			50%					T23		  			75%					FUL		
 0x0000000000000000, 0x00000000000000FF, 0x0000000000000000, 0x00000000000000FF, 0x000000000000FFFF, 0};
//  			TR1 				DUO		  			TR2 	     		D2		  			TR3  TRIG		


void SequencerKernel::construct(int _id, SequencerKernel *_masterKernel, bool* _holdTiedNotesPtr) {// don't want regaular constructor mechanism
	id = _id;
	ids = "id" + std::to_string(id) + "_";
	masterKernel = _masterKernel;
	holdTiedNotesPtr = _holdTiedNotesPtr;
}



void SequencerKernel::setGate(int seqn, int stepn, bool newGate, int count) {
	int endi = min(MAX_STEPS, stepn + count);
	for (int i = stepn; i < endi; i++)
		attributes[seqn][i].setGate(newGate);
	dirty[seqn] = 1;
}
void SequencerKernel::setGateP(int seqn, int stepn, bool newGateP, int count) {
	int endi = min(MAX_STEPS, stepn + count);
	for (int i = stepn; i < endi; i++)
		attributes[seqn][i].setGateP(newGateP);
	dirty[seqn] = 1;
}
void SequencerKernel::setSlide(int seqn, int stepn, bool newSlide, int count) {
	int endi = min(MAX_STEPS, stepn + count);
	for (int i = stepn; i < endi; i++)
		attributes[seqn][i].setSlide(newSlide);
	dirty[seqn] = 1;
}
void SequencerKernel::setTied(int seqn, int stepn, bool newTied, int count) {
	int endi = min(MAX_STEPS, stepn + count);
	if (!newTied) {
		for (int i = stepn; i < endi; i++)
			deactivateTiedStep(seqn, i);
	}
	else {
		for (int i = stepn; i < endi; i++)
			activateTiedStep(seqn, i);
	}
	dirty[seqn] = 1;
}

void SequencerKernel::setGatePVal(int seqn, int stepn, int gatePval, int count) {
	int endi = min(MAX_STEPS, stepn + count);
	for (int i = stepn; i < endi; i++)
		attributes[seqn][i].setGatePVal(gatePval);
	dirty[seqn] = 1;
}
void SequencerKernel::setSlideVal(int seqn, int stepn, int slideVal, int count) {
	int endi = min(MAX_STEPS, stepn + count);
	for (int i = stepn; i < endi; i++)
		attributes[seqn][i].setSlideVal(slideVal);
	dirty[seqn] = 1;
}
void SequencerKernel::setVelocityVal(int seqn, int stepn, int velocity, int count) {
	int endi = min(MAX_STEPS, stepn + count);
	for (int i = stepn; i < endi; i++)
		attributes[seqn][i].setVelocityVal(velocity);
	dirty[seqn] = 1;
}
void SequencerKernel::setGateType(int seqn, int stepn, int gateType, int count) {
	int endi = min(MAX_STEPS, stepn + count);
	for (int i = stepn; i < endi; i++)
		attributes[seqn][i].setGateType(gateType);
	dirty[seqn] = 1;
}


float SequencerKernel::applyNewOctave(int seqn, int stepn, int newOct, int count) {// does not overwrite tied steps
	float newCV = cv[seqn][stepn] + 10.0f;//to properly handle negative note voltages
	newCV = newCV - floor(newCV) + (float) (newOct - 3);
	
	writeCV(seqn, stepn, newCV, count);// also sets dirty[] to 1
	return newCV;
}
float SequencerKernel::applyNewKey(int seqn, int stepn, int newKeyIndex, int count) {// does not overwrite tied steps
	float newCV = floor(cv[seqn][stepn]) + ((float) newKeyIndex) / 12.0f;
	
	writeCV(seqn, stepn, newCV, count);// also sets dirty[] to 1
	return newCV;
}
void SequencerKernel::writeCV(int seqn, int stepn, float newCV, int count) {// does not overwrite tied steps
	int endi = min(MAX_STEPS, stepn + count);
	for (int i = stepn; i < endi; i++) {
		if (!attributes[seqn][i].getTied()) {
			cv[seqn][i] = newCV;
			propagateCVtoTied(seqn, i);
		}
	}
	dirty[seqn] = 1;
}


void SequencerKernel::initSequence(int seqn) {
	sequences[seqn].init(MAX_STEPS, MODE_FWD);
	for (int stepn = 0; stepn < MAX_STEPS; stepn++) {
		cv[seqn][stepn] = INIT_CV;
		attributes[seqn][stepn].init();
	}
	dirty[seqn] = 0;
}
void SequencerKernel::initSong() {
	runModeSong = MODE_FWD;
	songBeginIndex = 0;
	songEndIndex = 0;
	for (int phrn = 0; phrn < MAX_PHRASES; phrn++) {
		phrases[phrn].init();
	}
}


void SequencerKernel::randomizeSequence(int seqn) {
	sequences[seqn].randomize(MAX_STEPS, NUM_MODES);// code below uses lengths so this must be randomized first
	for (int stepn = 0; stepn < MAX_STEPS; stepn++) {
		cv[seqn][stepn] = ((float)(randomu32() % 7)) + ((float)(randomu32() % 12)) / 12.0f - 3.0f;
		attributes[seqn][stepn].randomize();
		// if (attributes[seqn][stepn].getTied()) {
			// activateTiedStep(seqn, stepn);
		// }	
	}
	dirty[seqn] = 1;
}
DEPRECATED void SequencerKernel::randomizeSong() {// no longer used
	runModeSong = randomu32() % NUM_MODES;
	songBeginIndex = 0;
	songEndIndex = (randomu32() % MAX_PHRASES);
	for (int phrn = 0; phrn < MAX_PHRASES; phrn++) {
		phrases[phrn].randomize(MAX_SEQS);
	}
}	


void SequencerKernel::copySequence(SeqCPbuffer* seqCPbuf, int seqn, int startCP, int countCP) {
	countCP = min(countCP, MAX_STEPS - startCP);
	for (int i = 0, stepn = startCP; i < countCP; i++, stepn++) {
		seqCPbuf->cvCPbuffer[i] = cv[seqn][stepn];
		seqCPbuf->attribCPbuffer[i] = attributes[seqn][stepn];
	}
	seqCPbuf->seqAttribCPbuffer = sequences[seqn];
	seqCPbuf->storedLength = countCP;
}
void SequencerKernel::pasteSequence(SeqCPbuffer* seqCPbuf, int seqn, int startCP) {
	int countCP = min(seqCPbuf->storedLength, MAX_STEPS - startCP);
	for (int i = 0, stepn = startCP; i < countCP; i++, stepn++) {
		cv[seqn][stepn] = seqCPbuf->cvCPbuffer[i];
		attributes[seqn][stepn] = seqCPbuf->attribCPbuffer[i];
	}
	if (startCP == 0 && countCP == MAX_STEPS)
		sequences[seqn] = seqCPbuf->seqAttribCPbuffer;
	dirty[seqn] = 1;
}
void SequencerKernel::copySong(SongCPbuffer* songCPbuf, int startCP, int countCP) {	
	countCP = min(countCP, MAX_PHRASES - startCP);
	for (int i = 0, phrn = startCP; i < countCP; i++, phrn++) {
		songCPbuf->phraseCPbuffer[i] = phrases[phrn];
	}
	songCPbuf->beginIndex = songBeginIndex;
	songCPbuf->endIndex = songEndIndex;
	songCPbuf->runModeSong = runModeSong;
	songCPbuf->storedLength = countCP;
}
void SequencerKernel::pasteSong(SongCPbuffer* songCPbuf, int startCP) {	
	int countCP = min(songCPbuf->storedLength, MAX_PHRASES - startCP);
	for (int i = 0, phrn = startCP; i < countCP; i++, phrn++) {
		phrases[phrn] = songCPbuf->phraseCPbuffer[i];
	}
	if (startCP == 0 && countCP == MAX_PHRASES) {
		songBeginIndex = songCPbuf->beginIndex;
		songEndIndex = songCPbuf->endIndex;
		runModeSong = songCPbuf->runModeSong;
	}
}


void SequencerKernel::reset() {
	initPulsesPerStep();
	initDelay();
	initSong();
	for (int seqn = 0; seqn < MAX_SEQS; seqn++) {
		initSequence(seqn);		
	}
	clockPeriod = 0ul;
	initRun();
}


void SequencerKernel::randomize(int seqn) {
	//randomizeSong();
	// for (int seqn = 0; seqn < MAX_SEQS; seqn++) {
		// randomizeSequence(seqn);
	// }
	randomizeSequence(seqn);
	initRun();
}
	

void SequencerKernel::toJson(json_t *rootJ) {
	// pulsesPerStep
	json_object_set_new(rootJ, (ids + "pulsesPerStep").c_str(), json_integer(pulsesPerStep));

	// delay
	json_object_set_new(rootJ, (ids + "delay").c_str(), json_integer(delay));

	// runModeSong
	json_object_set_new(rootJ, (ids + "runModeSong").c_str(), json_integer(runModeSong));

	// sequences (attributes of a seqs)
	json_t *sequencesJ = json_array();
	for (int i = 0; i < MAX_SEQS; i++)
		json_array_insert_new(sequencesJ, i, json_integer(sequences[i].getSeqAttrib()));
	json_object_set_new(rootJ, (ids + "sequences").c_str(), sequencesJ);

	// phrases 
	json_t *phrasesJ = json_array();
	for (int i = 0; i < MAX_PHRASES; i++)
		json_array_insert_new(phrasesJ, i, json_integer(phrases[i].getPhraseJson()));
	json_object_set_new(rootJ, (ids + "phrases").c_str(), phrasesJ);

	// CV and attributes
	json_t *seqSavedJ = json_array();		
	json_t *cvJ = json_array();
	json_t *attributesJ = json_array();
	for (int seqnRead = 0, seqnWrite = 0; seqnRead < MAX_SEQS; seqnRead++) {
		if (dirty[seqnRead] == 0) {
			json_array_insert_new(seqSavedJ, seqnRead, json_integer(0));
		}
		else {
			json_array_insert_new(seqSavedJ, seqnRead, json_integer(1));
			for (int stepn = 0; stepn < MAX_STEPS; stepn++) {
				json_array_insert_new(cvJ, stepn + (seqnWrite * MAX_STEPS), json_real(cv[seqnRead][stepn]));
				json_array_insert_new(attributesJ, stepn + (seqnWrite * MAX_STEPS), json_integer(attributes[seqnRead][stepn].getAttribute()));
			}
			seqnWrite++;
		}
	}
	json_object_set_new(rootJ, (ids + "seqSaved").c_str(), seqSavedJ);
	json_object_set_new(rootJ, (ids + "cv").c_str(), cvJ);
	json_object_set_new(rootJ, (ids + "attributes").c_str(), attributesJ);

	// songBeginIndex
	json_object_set_new(rootJ, (ids + "songBeginIndex").c_str(), json_integer(songBeginIndex));

	// songEndIndex
	json_object_set_new(rootJ, (ids + "songEndIndex").c_str(), json_integer(songEndIndex));

}


void SequencerKernel::fromJson(json_t *rootJ) {
	// pulsesPerStep
	json_t *pulsesPerStepJ = json_object_get(rootJ, (ids + "pulsesPerStep").c_str());
	if (pulsesPerStepJ)
		pulsesPerStep = json_integer_value(pulsesPerStepJ);

	// delay
	json_t *delayJ = json_object_get(rootJ, (ids + "delay").c_str());
	if (delayJ)
		delay = json_integer_value(delayJ);

	// runModeSong
	json_t *runModeSongJ = json_object_get(rootJ, (ids + "runModeSong").c_str());
	if (runModeSongJ)
		runModeSong = json_integer_value(runModeSongJ);
			
	// sequences (attributes of a seqs)
	json_t *sequencesJ = json_object_get(rootJ, (ids + "sequences").c_str());
	if (sequencesJ) {
		for (int i = 0; i < MAX_SEQS; i++)
		{
			json_t *sequencesArrayJ = json_array_get(sequencesJ, i);
			if (sequencesArrayJ)
				sequences[i].setSeqAttrib(json_integer_value(sequencesArrayJ));
		}			
	}		
	
	// phrases
	json_t *phrasesJ = json_object_get(rootJ, (ids + "phrases").c_str());
	if (phrasesJ)
		for (int i = 0; i < MAX_PHRASES; i++)
		{
			json_t *phrasesArrayJ = json_array_get(phrasesJ, i);
			if (phrasesArrayJ)
				phrases[i].setPhraseJson(json_integer_value(phrasesArrayJ));
		}
	
	// CV and attributes
	json_t *seqSavedJ = json_object_get(rootJ, (ids + "seqSaved").c_str());
	int seqSaved[MAX_SEQS];
	if (seqSavedJ) {
		int i;
		for (i = 0; i < MAX_SEQS; i++)
		{
			json_t *seqSavedArrayJ = json_array_get(seqSavedJ, i);
			if (seqSavedArrayJ)
				seqSaved[i] = json_integer_value(seqSavedArrayJ);
			else 
				break;
		}	
		if (i == MAX_SEQS) {			
			json_t *cvJ = json_object_get(rootJ, (ids + "cv").c_str());
			json_t *attributesJ = json_object_get(rootJ, (ids + "attributes").c_str());
			if (cvJ && attributesJ) {
				for (int seqnFull = 0, seqnComp = 0; seqnFull < MAX_SEQS; seqnFull++) {
					if (seqSaved[seqnFull]) {
						for (int stepn = 0; stepn < MAX_STEPS; stepn++) {
							json_t *cvArrayJ = json_array_get(cvJ, stepn + (seqnComp * MAX_STEPS));
							if (cvArrayJ)
								cv[seqnFull][stepn] = json_number_value(cvArrayJ);
							json_t *attributesArrayJ = json_array_get(attributesJ, stepn + (seqnComp * MAX_STEPS));
							if (attributesArrayJ)
								attributes[seqnFull][stepn].setAttribute(json_integer_value(attributesArrayJ));
						}
						dirty[seqnFull] = 1;
						seqnComp++;
					}
					else {
						for (int stepn = 0; stepn < MAX_STEPS; stepn++) {
							cv[seqnFull][stepn] = INIT_CV;
							attributes[seqnFull][stepn].init();
						}
						dirty[seqnFull] = 0;
					}	
				}
			}
		}
	}		
	
	// songBeginIndex
	json_t *songBeginIndexJ = json_object_get(rootJ, (ids + "songBeginIndex").c_str());
	if (songBeginIndexJ)
		songBeginIndex = json_integer_value(songBeginIndexJ);
			
	// songEndIndex
	json_t *songEndIndexJ = json_object_get(rootJ, (ids + "songEndIndex").c_str());
	if (songEndIndexJ)
		songEndIndex = json_integer_value(songEndIndexJ);
}


void SequencerKernel::initRun() {
	movePhraseIndexRun(true);// true means init 
	moveStepIndexRunIgnore = false;
	moveStepIndexRun(true);// true means init 
	
	ppqnCount = 0;
	ppqnLeftToSkip = delay;
	calcGateCodeEx();// uses stepIndexRun as the step and phraseIndexRun to determine the seq
	slideStepsRemain = 0ul;
}


bool SequencerKernel::clockStep() {
	bool phraseChange = false;
	
	if (ppqnLeftToSkip > 0) {
		ppqnLeftToSkip--;
	}
	else {
		ppqnCount++;
		int ppsFiltered = getPulsesPerStep();// must use method
		if (ppqnCount >= ppsFiltered)
			ppqnCount = 0;
		if (ppqnCount == 0) {
			float slideFromCV = getCVRun();
			if (moveStepIndexRun(false)) {// false means normal (not init)
				movePhraseIndexRun(false);// false means normal (not init)
				moveStepIndexRun(true);// true means init; must always refresh after phraseIndexRun has changed
				phraseChange = true;// only used by first track
			}

			// Slide
			StepAttributes attribRun = getAttributeRun();
			if (attribRun.getSlide()) {
				slideStepsRemain = (unsigned long) (((float)clockPeriod * ppsFiltered) * ((float)attribRun.getSlideVal() / 100.0f));
				if (slideStepsRemain != 0ul) {
					float slideToCV = getCVRun();
					slideCVdelta = (slideToCV - slideFromCV)/(float)slideStepsRemain;
				}
			}
			else
				slideStepsRemain = 0ul;
		}
		calcGateCodeEx();// uses stepIndexRun as the step and phraseIndexRun to determine the seq
	}
	clockPeriod = 0ul;
	
	return phraseChange;
}


int SequencerKernel::keyIndexToGateTypeEx(int keyIndex) {// return -1 when invalid gate type given current pps setting
	int ppsFiltered = getPulsesPerStep();// must use method
	int ret = keyIndex;
	
	if (keyIndex == 1 || keyIndex == 3 || keyIndex == 6 || keyIndex == 8 || keyIndex == 10) {// black keys
		if ((ppsFiltered % 6) != 0)
			ret = -1;
	}
	else if (keyIndex == 4 || keyIndex == 7 || keyIndex == 9) {// 75%, DUO, DU2 
		if ((ppsFiltered % 4) != 0)
			ret = -1;
	}
	else if (keyIndex == 2) {// 50%
		if ((ppsFiltered % 2) != 0)
			ret = -1;
	}
	else if (keyIndex == 0) {// 25%
		if (ppsFiltered != 1 && (ppsFiltered % 4) != 0)
			ret = -1;
	}
	//else always good: 5 (full) and 11 (trig)
	
	return ret;
}


void SequencerKernel::transposeSeq(int seqn, int delta) {
	int tVal = sequences[seqn].getTranspose();
	int oldTransposeOffset = tVal;
	tVal = clamp(tVal + delta, -99, 99);
	sequences[seqn].setTranspose(tVal);
	
	delta = tVal - oldTransposeOffset;
	if (delta != 0) { 
		float offsetCV = ((float)(delta))/12.0f;
		for (int stepn = 0; stepn < MAX_STEPS; stepn++) 
			cv[seqn][stepn] += offsetCV;
	}
	dirty[seqn] = 1;
}


void SequencerKernel::rotateSeq(int seqn, int delta) {
	int rVal = sequences[seqn].getRotate();
	int oldRotateOffset = rVal;
	rVal = clamp(rVal + delta, -99, 99);
	sequences[seqn].setRotate(rVal);
	
	delta = rVal - oldRotateOffset;
	if (delta == 0) 
		return;// if end of range, no transpose to do
	
	if (delta > 0 && delta < 201) {// Rotate right, 201 is safety (account for a delta of 2*99 when min to max in one shot)
		for (int i = delta; i > 0; i--) {
			rotateSeqByOne(seqn, true);
		}
	}
	if (delta < 0 && delta > -201) {// Rotate left, 201 is safety (account for a delta of 2*99 when min to max in one shot)
		for (int i = delta; i < 0; i++) {
			rotateSeqByOne(seqn, false);
		}
	}
	dirty[seqn] = 1;
}	


void SequencerKernel::rotateSeqByOne(int seqn, bool directionRight) {// caller sets dirty[] to 1
	float rotCV;
	StepAttributes rotAttributes;
	int iStart = 0;
	int iEnd = sequences[seqn].getLength() - 1;
	int iRot = iStart;
	int iDelta = 1;
	if (directionRight) {
		iRot = iEnd;
		iDelta = -1;
	}
	rotCV = cv[seqn][iRot];
	rotAttributes = attributes[seqn][iRot];
	for ( ; ; iRot += iDelta) {
		if (iDelta == 1 && iRot >= iEnd) break;
		if (iDelta == -1 && iRot <= iStart) break;
		cv[seqn][iRot] = cv[seqn][iRot + iDelta];
		attributes[seqn][iRot] = attributes[seqn][iRot + iDelta];
	}
	cv[seqn][iRot] = rotCV;
	attributes[seqn][iRot] = rotAttributes;
}


void SequencerKernel::activateTiedStep(int seqn, int stepn) {// caller sets dirty[] to 1
	attributes[seqn][stepn].setTied(true);
	if (stepn > 0) 
		propagateCVtoTied(seqn, stepn - 1);
	
	if (*holdTiedNotesPtr) {// new method
		attributes[seqn][stepn].setGate(true);
		for (int i = max(stepn, 1); i < MAX_STEPS && attributes[seqn][i].getTied(); i++) {
			attributes[seqn][i].setGateType(attributes[seqn][i - 1].getGateType());
			attributes[seqn][i - 1].setGateType(5);
			attributes[seqn][i - 1].setGate(true);
		}
	}
	else {// old method
		if (stepn > 0) {
			attributes[seqn][stepn] = attributes[seqn][stepn - 1];
			attributes[seqn][stepn].setTied(true);
		}
	}
}


void SequencerKernel::deactivateTiedStep(int seqn, int stepn) {// caller sets dirty[] to 1
	attributes[seqn][stepn].setTied(false);
	if (*holdTiedNotesPtr) {// new method
		int lastGateType = attributes[seqn][stepn].getGateType();
		for (int i = stepn + 1; i < MAX_STEPS && attributes[seqn][i].getTied(); i++)
			lastGateType = attributes[seqn][i].getGateType();
		if (stepn > 0)
			attributes[seqn][stepn - 1].setGateType(lastGateType);
	}
	//else old method, nothing to do
}


void SequencerKernel::calcGateCodeEx() {// uses stepIndexRun as the step and phraseIndexRun to determine the seq
	int seqn = phrases[phraseIndexRun].getSeqNum();
	StepAttributes attribute = attributes[seqn][stepIndexRun];
	int ppsFiltered = getPulsesPerStep();// must use method
	int gateType;

	if (gateCode != -1 || ppqnCount == 0) {// always calc on first ppqnCount, avoid thereafter if gate will be off for whole step
		gateType = attribute.getGateType();
		
		// -1 = gate off for whole step, 0 = gate off for current ppqn, 1 = gate on, 2 = clock high, 3 = trigger
		if ( ppqnCount == 0 && attribute.getGateP() && !(randomUniform() < ((float)attribute.getGatePVal() / 100.0f)) ) {// randomUniform is [0.0, 1.0), see include/util/common.hpp
			gateCode = -1;// must do this first in this method since it will kill all remaining pulses of the step if prob turns off the step
		}
		else if (!attribute.getGate()) {
			gateCode = 0;
		}
		else if (ppsFiltered == 1 && gateType == 0) {
			gateCode = 2;// clock high pulse
		}
		else {
			if (gateType == 11) {
				gateCode = (ppqnCount == 0 ? 3 : 0);// trig on first ppqnCount
			}
			else {
				uint64_t shiftAmt = ppqnCount * (96 / ppsFiltered);
				if (shiftAmt >= 64)
					gateCode = (int)((advGateHitMaskHigh[gateType] >> (shiftAmt - (uint64_t)64)) & (uint64_t)0x1);
				else
					gateCode = (int)((advGateHitMaskLow[gateType] >> shiftAmt) & (uint64_t)0x1);
			}
		}
	}
}
	

bool SequencerKernel::moveStepIndexRun(bool init) {	
	if (moveStepIndexRunIgnore) {
		moveStepIndexRunIgnore = false;
		return true;
	}
	
	int reps = phrases[phraseIndexRun].getReps();// 0-rep seqs should be filtered elsewhere and should never happen here. If they do, they will be played (this can be the case when all of the song has 0-rep seqs, or the song is started (reset) into a first phrase that has 0 reps)
	// assert((reps * MAX_STEPS) <= 0xFFF); // for BRN and RND run modes, history is not a span count but a step count
	int seqn = phrases[phraseIndexRun].getSeqNum();
	int runMode = sequences[seqn].getRunMode();
	int endStep = sequences[seqn].getLength() - 1;
	
	bool crossBoundary = false;
	
	if (init)
		stepIndexRunHistory = 0;
	
	switch (runMode) {
	
		// history 0x0000 is reserved for reset
		
		case MODE_REV :// reverse; history base is 0x2000
			if (stepIndexRunHistory < 0x2001 || stepIndexRunHistory > 0x2FFF)
				stepIndexRunHistory = 0x2000 + reps;
			if (init)
				stepIndexRun = endStep;
			else {
				stepIndexRun--;
				if (stepIndexRun < 0) {
					stepIndexRun = endStep;
					stepIndexRunHistory--;
					if (stepIndexRunHistory <= 0x2000)
						crossBoundary = true;
				}
			}
		break;
		
		case MODE_PPG :// forward-reverse; history base is 0x3000
			if (stepIndexRunHistory < 0x3001 || stepIndexRunHistory > 0x3FFF) // even means going forward, odd means going reverse
				stepIndexRunHistory = 0x3000 + reps * 2;
			if (init)
				stepIndexRun = 0;
			else {
				if ((stepIndexRunHistory & 0x1) == 0) {// even so forward phase
					stepIndexRun++;
					if (stepIndexRun > endStep) {
						stepIndexRun = endStep;
						stepIndexRunHistory--;
					}
				}
				else {// odd so reverse phase
					stepIndexRun--;
					if (stepIndexRun < 0) {
						stepIndexRun = 0;
						stepIndexRunHistory--;
						if (stepIndexRunHistory <= 0x3000)
							crossBoundary = true;
					}
				}
			}
		break;

		case MODE_PEN :// forward-reverse; history base is 0x4000
			if (stepIndexRunHistory < 0x4001 || stepIndexRunHistory > 0x4FFF) // even means going forward, odd means going reverse
				stepIndexRunHistory = 0x4000 + reps * 2;
			if (init)
				stepIndexRun = 0;
			else {			
				if ((stepIndexRunHistory & 0x1) == 0) {// even so forward phase
					stepIndexRun++;
					if (stepIndexRun > endStep) {
						stepIndexRun = endStep - 1;
						stepIndexRunHistory--;
						if (stepIndexRun <= 0) {// if back at start after turnaround, then no reverse phase needed
							stepIndexRun = 0;
							stepIndexRunHistory--;
							if (stepIndexRunHistory <= 0x4000)
								crossBoundary = true;
						}
					}
				}
				else {// odd so reverse phase
					stepIndexRun--;
					if (stepIndexRun > endStep)// handle song jumped
						stepIndexRun = endStep;
					if (stepIndexRun <= 0) {
						stepIndexRun = 0;
						stepIndexRunHistory--;
						if (stepIndexRunHistory <= 0x4000)
							crossBoundary = true;
					}
				}
			}
		break;
		
		case MODE_BRN :// brownian random; history base is 0x5000
			if (stepIndexRunHistory < 0x5001 || stepIndexRunHistory > 0x5FFF) 
				stepIndexRunHistory = 0x5000 + (endStep + 1) * reps;			
			if (init)
				stepIndexRun = 0;
			else {
				stepIndexRun += (randomu32() % 3) - 1;
				if (stepIndexRun > endStep)
					stepIndexRun = 0;
				if (stepIndexRun < 0)
					stepIndexRun = endStep;
				stepIndexRunHistory--;
				if (stepIndexRunHistory <= 0x5000)
					crossBoundary = true;
			}
		break;
		
		case MODE_RND :// random; history base is 0x6000
			if (stepIndexRunHistory < 0x6001 || stepIndexRunHistory > 0x6FFF)
				stepIndexRunHistory = 0x6000 + (endStep + 1) * reps;
			if (init)
				stepIndexRun = 0;
			else {
				stepIndexRun = (randomu32() % (endStep + 1));
				stepIndexRunHistory--;
				if (stepIndexRunHistory <= 0x6000)
					crossBoundary = true;
			}
		break;
		
		case MODE_TKA :// use track A's stepIndexRun; base is 0x7000
			if (masterKernel != nullptr) {
				stepIndexRunHistory = 0x7000;
				stepIndexRun = masterKernel->getStepIndexRun();
				break;
			}
			[[fallthrough]];
		default :// MODE_FWD  forward; history base is 0x1000
			if (stepIndexRunHistory < 0x1001 || stepIndexRunHistory > 0x1FFF)
				stepIndexRunHistory = 0x1000 + reps;
			if (init)
				stepIndexRun = 0;
			else {			
				stepIndexRun++;
				if (stepIndexRun > endStep) {
					stepIndexRun = 0;
					stepIndexRunHistory--;
					if (stepIndexRunHistory <= 0x1000)
						crossBoundary = true;
				}
			}
	}

	return crossBoundary;
}


void SequencerKernel::moveSongIndexBackward(bool init, bool rollover) {
	int phrn = 0;

	// search backward for next non 0-rep seq, ends up in same phrase if all reps in the song are 0
	if (init) {
		phraseIndexRun = songEndIndex;
		phrn = phraseIndexRun;
	}
	else
		phrn = min(phraseIndexRun - 1, songEndIndex);// handle song jumped
	for (; phrn >= songBeginIndex && phrases[phrn].getReps() == 0; phrn--);
	if (phrn < songBeginIndex) {
		if (rollover)
			for (phrn = songEndIndex; phrn > phraseIndexRun && phrases[phrn].getReps() == 0; phrn--);
		else
			phrn = phraseIndexRun;
		phraseIndexRunHistory--;
	}
	phraseIndexRun = phrn;
}


void SequencerKernel::moveSongIndexForeward(bool init, bool rollover) {
	int phrn = 0;
	
	// search fowrard for next non 0-rep seq, ends up in same phrase if all reps in the song are 0
	if (init) {
		phraseIndexRun = songBeginIndex;
		phrn = phraseIndexRun;
	}
	else
		phrn = max(phraseIndexRun + 1, songBeginIndex);// handle song jumped
	for (; phrn <= songEndIndex && phrases[phrn].getReps() == 0; phrn++);
	if (phrn > songEndIndex) {
		if (rollover)
			for (phrn = songBeginIndex; phrn < phraseIndexRun && phrases[phrn].getReps() == 0; phrn++);
		else
			phrn = phraseIndexRun;
		phraseIndexRunHistory--;
	}
	phraseIndexRun = phrn;
}


void SequencerKernel::moveSongIndexRandom(bool init, uint32_t randomValue) {
	int phrn = songBeginIndex;
	int tpi = 0;
	
	for (;phrn <= songEndIndex; phrn++) {
		if (phrases[phrn].getReps() != 0) {
			tempPhraseIndexes[tpi] = phrn;
			tpi++;
			if (init) break;
		}
	}
	
	if (init) {
		phraseIndexRun = (tpi == 0 ? songBeginIndex : tempPhraseIndexes[0]);
	}
	else {
		phraseIndexRun = tempPhraseIndexes[randomValue % tpi];
	}
}


void SequencerKernel::moveSongIndexBrownian(bool init, uint32_t randomValue) {	
	randomValue = randomValue % 3;// 0 = left, 1 = stay, 2 = right
	
	if (init) {
		moveSongIndexForeward(init, true);
	}
	else if (randomValue == 1) {// stay
		if (phraseIndexRun > songEndIndex || phraseIndexRun < songBeginIndex)
			moveSongIndexForeward(false, true);	
	}
	else if (randomValue == 0) {// left
		moveSongIndexBackward(false, true);
	}
	else {// right
		moveSongIndexForeward(false, true);
	}
}


void SequencerKernel::movePhraseIndexRun(bool init) {	
	if (init)
		phraseIndexRunHistory = 0;
	
	switch (runModeSong) {
	
		// history 0x0000 is reserved for reset
		
		case MODE_REV :// reverse; history base is 0x2000
			phraseIndexRunHistory = 0x2000;
			moveSongIndexBackward(init, true);
		break;
		
		case MODE_PPG :// forward-reverse; history base is 0x3000
			if (phraseIndexRunHistory < 0x3001 || phraseIndexRunHistory > 0x3002) // even means going forward, odd means going reverse
				phraseIndexRunHistory = 0x3002;
			if (phraseIndexRunHistory == 0x3002) {// even so forward phase
				moveSongIndexForeward(init, false);
			}
			else {// odd so reverse phase
				moveSongIndexBackward(false, false);
			}
		break;

		case MODE_PEN :// forward-reverse; history base is 0x4000
			if (phraseIndexRunHistory < 0x4001 || phraseIndexRunHistory > 0x4002) // even means going forward, odd means going reverse
				phraseIndexRunHistory = 0x4002;
			if (phraseIndexRunHistory == 0x4002) {// even so forward phase	
				moveSongIndexForeward(init, false);
				if (phraseIndexRunHistory == 0x4001)
					moveSongIndexBackward(false, false);
			}
			else {// odd so reverse phase
				moveSongIndexBackward(false, false);
				if (phraseIndexRunHistory == 0x4000)
					moveSongIndexForeward(false, false);
			}			
		break;
		
		case MODE_BRN :// brownian random; history base is 0x5000
			phraseIndexRunHistory = 0x5000;
			moveSongIndexBrownian(init, randomu32());
		break;
		
		case MODE_RND :// random; history base is 0x6000
			phraseIndexRunHistory = 0x6000;
			moveSongIndexRandom(init, randomu32());
		break;
		
		case MODE_TKA:// use track A's phraseIndexRun; base is 0x7000
			if (masterKernel != nullptr) {
				phraseIndexRunHistory = 0x7000;
				if (init)// only init to be done in here, rest is handled in Sequencer::clockStep()
					phraseIndexRun = masterKernel->getPhraseIndexRun();
				break;
			}
			[[fallthrough]];// TKA defaults to FWD for track A
		default :// MODE_FWD  forward; history base is 0x1000
			phraseIndexRunHistory = 0x1000;
			moveSongIndexForeward(init, true);
	}
}


 
void SeqCPbuffer::reset() {		
	for (int stepn = 0; stepn < SequencerKernel::MAX_STEPS; stepn++) {
		cvCPbuffer[stepn] = 0.0f;
		attribCPbuffer[stepn].init();
	}
	seqAttribCPbuffer.init(SequencerKernel::MAX_STEPS, SequencerKernel::MODE_FWD);
	storedLength = SequencerKernel::MAX_STEPS;// number of steps that contain actual cp data
}

void SongCPbuffer::reset() {
	for (int phrn = 0; phrn < SequencerKernel::MAX_PHRASES; phrn++)
		phraseCPbuffer[phrn].init();
	beginIndex = 0;
	endIndex = 0;
	runModeSong = SequencerKernel::MODE_FWD;
	storedLength = SequencerKernel::MAX_PHRASES;
}

} // namespace rack_plugin_ImpromptuModular
