#include "Bidoo.hpp"
#include "dsp/digital.hpp"
#include "BidooComponents.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include "window.hpp"
#include <iomanip>
#include <sstream>

using namespace std;

namespace rack_plugin_Bidoo {

struct trig {

	trig() {

	}

	bool isActive = false;
	float slide = 0.0f;
	bool swing = false;
	bool isSleeping = false;
	size_t trigType = 0;
	size_t index = 0;
	size_t reference = 0;
	int trim = 0;
	size_t length = 190;
	size_t pulseCount = 1;
	size_t pulseDistance = 1;
	float VO = 0.0f;
	float CV1 = 0.0f;
	float CV2 = 0.0f;
	size_t proba = 0;
	size_t count = 100;
	size_t countReset = 1;
	size_t inCount = 0;

	float getGateValue(const float trackPosition);
	int getTrimedReference();
	float getRelativeTrackPosition(const float trackPosition);
	int getFullLength();
	float getVO();
	float getCV1();
	float getCV2();
	void init(const bool fill, const bool pre, const bool nei);
	void resetValues();
	void randomize();
	bool hasProbability();
	void copy(const trig *t);
};

inline void trig::copy(const trig *t) {
	isActive = t->isActive;
	slide = t->slide;
	swing = t->swing;
	trigType = t->trigType;
	trim = t->trim;
	length = t->length;
	pulseCount = t->pulseCount;
	pulseDistance = t->pulseDistance;
	VO = t->VO;
	CV1 = t->CV1;
	CV2 = t->CV2;
	proba = t->proba;
	count = t->count;
	countReset = t->countReset;
}

inline void trig::resetValues() {
	isActive = false;
	slide = 0.0f;
	swing = index%2 != 0;
	trigType = 0;
	trim = 0;
	length = 80;
	pulseCount = 1;
	pulseDistance = 1;
	VO = 0.0f;
	CV1 = 0.0f;
	CV2 = 0.0f;
	proba = 0;
	count = 100;
	countReset = 1;
}

inline void trig::randomize() {
	isActive = randomUniform()>0.5f;
	slide = randomUniform()*10.0f;
	swing = randomUniform()>0.5f;
	trigType = (int)(randomUniform()*2);
	trim = (int)(randomUniform()*191) * (randomUniform()>0.5f ? -1 : 1);
	length = (int)(randomUniform()*192);
	pulseCount = (int)(randomUniform()*10);
	pulseDistance = (int)(randomUniform()*700);
	VO = randomUniform()*10.0f;
	CV1 = randomUniform()*10.0f;
	CV2 = randomUniform()*10.0f;
	proba = (int)(randomUniform()*7);
	count = (int)(randomUniform()*99)+1;
	countReset = (int)(randomUniform()*99)+1;
}

inline bool trig::hasProbability() {
	return (proba != 4) && (proba != 5) && !((proba == 0) && (count == 100));
}

inline void trig::init(const bool fill, const bool pre, const bool nei) {
	switch (proba) {
		case 0:  // dice
			if (count<100) {
				isSleeping = (randomUniform()*100)>=count;
			}
			else
				isSleeping = false;
			break;
		case 1:  // COUNT
			isSleeping = (inCount != count);
			inCount = (inCount >= countReset) ? 1 : (inCount + 1);
			break;
		case 2:  // FILL
			isSleeping = !fill;
			break;
		case 3:  // !FILL
			isSleeping = fill;
			break;
		case 4:  // PRE
			isSleeping = !pre;
			break;
		case 5:  // !PRE
			isSleeping = pre;
			break;
		case 6:  // NEI
			isSleeping = !nei;
			break;
		case 7:  // !NEI
			isSleeping = nei;
			break;
		default:
			isSleeping = false;
			break;
	}
}

inline float trig::getGateValue(const float trackPosition) {
	if (isActive && !isSleeping) {
		float rTTP = getRelativeTrackPosition(trackPosition);
		if (rTTP >= 0) {
			if (rTTP<length) {
				return 10.0f;
			}
			else {
				size_t cPulses = (pulseDistance == 0) ? 0 : (int)(rTTP/(float)pulseDistance);
				return ((cPulses<pulseCount) && (rTTP>=(cPulses*pulseDistance)) && (rTTP<=((cPulses*pulseDistance)+length))) ? 10.0f : 0.0f;
			}
		}
		else
			return 0.0f;
	}
	else
		return 0.0f;
}

inline float trig::getRelativeTrackPosition(const float trackPosition) {
	return trackPosition - getTrimedReference();
}

inline int trig::getFullLength() {
	return pulseCount == 1 ? length : ((pulseCount*pulseDistance) + length);
}

inline int trig::getTrimedReference() {
	return reference + trim;
}

inline float trig::getVO() {
	return VO;
}

inline float trig::getCV1() {
	return CV1;
}

inline float trig::getCV2() {
	return CV2;
}

struct track {
	trig trigs[64];
	bool isActive = true;
	size_t length = 16;
	size_t readMode = 0;
	float trackIndex = 0.0f;
	float speed = 1.0f;
	float swing = 0.0f;
	trig *prevTrig = trigs;
	trig *memTrig = trigs;
	trig *currentTrig = trigs;
	size_t nextIndex = 0;
	bool fwd = true;
	bool pre = false;

	track() {
		for(size_t i=0;i<64;i++) {
			trigs[i].index = i;
			trigs[i].reference = i*192;
			trigs[i].swing = ((i%2) != 0);
		}
	}

	trig getCurrentTrig();
	trig getMemTrig();
	float getGate();
	float getVO();
	float getCV1();
	float getCV2();
	void reset(const bool fill, const bool pNei);
	void moveNext(const bool fill, const bool pNei);
	void moveNextForward(const bool fill, const bool pNei);
	void moveNextBackward(const bool fill, const bool pNei);
	void moveNextPendulum(const bool fill, const bool pNei);
	void moveNextRandom(const bool fill, const bool pNei);
	void moveNextBrownian(const bool fill, const bool pNei);
	size_t getNextIndex();
	void resetValues();
	void randomize();
	void copy(const track *t);
};

inline void track::copy(const track *t) {
	isActive = t->isActive;
	length = t->length;
	readMode = t->readMode;
	speed = t->speed;
	swing = t->swing;
	for (size_t i = 0; i < 64; i++) {
		trigs[i].copy(t->trigs + i);
	}
}

inline void track::resetValues() {
	isActive = true;
	length = 16;
	readMode = 0;
	speed = 1.0f;
	swing = 0.0f;
	pre = false;
}

inline void track::randomize() {
	isActive = randomUniform()>0.5f;
	length = (int)(randomUniform()*64.0f);
	readMode = (int)(randomUniform()*4.0f);
	speed = (int)(randomUniform()*16.0f) + (randomUniform()>0.5f ? 0.5f : 0.0f);
	swing = randomUniform()*100.0f;
}

inline trig track::getCurrentTrig() {
	return *currentTrig;
}

inline trig track::getMemTrig() {
	return *memTrig;
}

inline float track::getGate() {
	return memTrig->getGateValue(trackIndex);
}

inline float track::getVO() {
	if (memTrig->slide == 0.0f) {
		return memTrig->VO;
	}
	else
	{
		float subPhase = (float)memTrig->getRelativeTrackPosition(trackIndex);
		float fullLength = (float)memTrig->getFullLength();
		if (fullLength > 0.0f) {
			subPhase = subPhase/fullLength;
		}
		else {
			subPhase = subPhase / 192.0f;
		}
		return memTrig->VO - (1.0f - powf(subPhase, memTrig->slide/11.0f)) * (memTrig->VO - prevTrig->VO);
	}
}

inline float track::getCV1() {
	return memTrig->getCV1();
}

inline float track::getCV2() {
	return memTrig->getCV2();
}

inline void track::reset(const bool fill, const bool pNei) {
	pre = false;
	if (readMode != 1)
	{
		fwd = true;
		currentTrig = trigs;
		currentTrig->init(fill,pre,pNei);
		trackIndex = speed;
		if (currentTrig->isActive && !currentTrig->isSleeping) {
			prevTrig = memTrig;
			memTrig = currentTrig;
		}
		nextIndex = getNextIndex();
	}
	else
	{
		currentTrig = trigs+length-1;
		currentTrig->init(fill,pre,pNei);
		trackIndex = currentTrig->reference + speed;
		if (currentTrig->isActive && !currentTrig->isSleeping) {
			prevTrig = memTrig;
			memTrig = currentTrig;
		}
		nextIndex = getNextIndex();
	}
}

inline size_t track::getNextIndex() {
	switch (readMode) {
		case 0:
				return (currentTrig->index+1) > (length-1) ? 0 : (currentTrig->index+1);
		case 1:
				return ((int)currentTrig->index-1) < 0 ? (length-1) : (currentTrig->index-1);
		case 2:
			if(fwd) {
				if (currentTrig->index == (length-1)) {
					fwd=!fwd;
					return ((int)currentTrig->index-1) < 0 ? (length-1) : (currentTrig->index-1);
				}
				else
					return (currentTrig->index+1)%(length);
				}
			else {
				if (currentTrig->index == 0) {
					fwd=!fwd;
					return (currentTrig->index+1)%(length);
				}
				else
					return ((int)currentTrig->index-1) < 0 ? (length-1) : (currentTrig->index-1);
			}
		case 3: return (size_t)(randomUniform()*(length-1));
		case 4:
		{
			float dice = randomUniform();
			if (dice>=0.5f)
				return (currentTrig->index+1) > (length-1) ? 0 : (currentTrig->index+1);
			else if (dice<=0.25f)
				return currentTrig->index == 0 ? (length-1) : (currentTrig->index-1);
			else
				return currentTrig->index;
		}
		default : return 0;
	}
}

inline void track::moveNextForward(const bool fill, const bool pNei) {
	if ((length>1) && (trackIndex>192) && (currentTrig->index==0) && (nextIndex==0)) nextIndex = getNextIndex();
	if (trackIndex>(length*192)) trackIndex=0.0f;
	if (((nextIndex != 0) && (nextIndex<64) && (trigs[nextIndex].getRelativeTrackPosition(trackIndex) >= 0)) || (trackIndex == 0.f)) {
		if ((nextIndex != currentTrig->index) || (length == 1)) {
			pre = (currentTrig->isActive && currentTrig->hasProbability()) ? !currentTrig->isSleeping : pre;
			trigs[nextIndex].init(fill,pre,pNei);
			currentTrig = trigs + nextIndex;
			if (currentTrig->isActive && !currentTrig->isSleeping) {
				prevTrig = memTrig;
				memTrig = currentTrig;
			}
			nextIndex = getNextIndex();
		}
	}
}

inline void track::moveNextBackward(const bool fill, const bool pNei) {
	if (trackIndex>(currentTrig->reference+191)) {
		pre = (currentTrig->isActive && currentTrig->hasProbability()) ? !currentTrig->isSleeping : pre;
		trigs[nextIndex].init(fill,pre,pNei);
		currentTrig = trigs + nextIndex;
		if (currentTrig->isActive && !currentTrig->isSleeping) {
			prevTrig = memTrig;
			memTrig = currentTrig;
		}
		trackIndex = currentTrig->reference;
		nextIndex = getNextIndex();
	}
}

inline void track::moveNextPendulum(const bool fill, const bool pNei){
	if (fwd) {
		moveNextForward(fill, pNei);
		if (currentTrig->index == length-1) fwd = false;
	}
	else {
		moveNextBackward(fill, pNei);
		if (currentTrig->index == 0) fwd = true;
	}
}

inline void track::moveNextRandom(const bool fill, const bool pNei){
	moveNextBackward(fill, pNei);
}

inline void track::moveNextBrownian(const bool fill, const bool pNei){
	moveNextBackward(fill, pNei);
}

void track::moveNext(const bool fill, const bool pNei) {
		trackIndex += speed;
		switch (readMode) {
			case 0: moveNextForward(fill, pNei); break;
			case 1: moveNextBackward(fill, pNei); break;
			case 2: moveNextPendulum(fill, pNei); break;
			case 3: moveNextRandom(fill, pNei); break;
			case 4: moveNextBrownian(fill, pNei); break;
		}
}

struct pattern {
	track tracks[8];

	pattern() {

	};

	void moveNext(const bool fill);
	void reset(const bool fill);
	void copy(const pattern *p);
};

inline void pattern::copy(const pattern *p) {
	for (size_t i = 0; i < 8; i++) {
		tracks[i].copy(p->tracks + i);
	}
}

inline void pattern::moveNext(const bool fill) {
	for (size_t i = 0; i < 8; i++) {
		tracks[i].moveNext(fill, i==0?false:tracks[i-1].pre);
	}
}

inline void pattern::reset(const bool fill) {
	for (size_t i = 0; i < 8; i++) {
		tracks[i].reset(fill, i==0?false:tracks[i-1].pre);
	}
}

struct ZOUMAI : Module {
	enum ParamIds {
		STEPS_PARAMS,
		TRACK_PARAMS = STEPS_PARAMS + 16,
		FREE_PARAMS = TRACK_PARAMS + 8,
		TRIG_PAGE_PARAM = FREE_PARAMS + 8,
		FILL_PARAM = TRIG_PAGE_PARAM + 4,
		PATTERN_PARAM,
		COPY_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		EXT_CLOCK_INPUT,
		RESET_INPUT,
		TRACK_RESET_INPUTS,
		G1_INPUT = TRACK_RESET_INPUTS + 8,
		G2_INPUT,
		PATTERN_INPUT,
		TRACK_ACTIVE_INPUTS,
		FILL_INPUT = TRACK_ACTIVE_INPUTS + 8,
		NUM_INPUTS
	};

	enum OutputIds {
		GATE_OUTPUTS,
		VO_OUTPUTS = GATE_OUTPUTS + 8,
		CV1_OUTPUTS = VO_OUTPUTS + 8,
		CV2_OUTPUTS = CV1_OUTPUTS + 8,
		NUM_OUTPUTS = CV2_OUTPUTS + 8
	};

	enum LightIds {
		STEPS_LIGHTS,
		TRACKS_LIGHTS = STEPS_LIGHTS + 16*3,
		TRIG_PAGE_LIGHTS = TRACKS_LIGHTS + 8*3,
		FILL_LIGHT = TRIG_PAGE_LIGHTS + 4*3,
		COPY_LIGHT,
		NUM_LIGHTS
	};

	SchmittTrigger clockTrigger;
	SchmittTrigger resetTrigger;
	SchmittTrigger trigPageTriggers[4];
	SchmittTrigger trackResetTriggers[8];
	SchmittTrigger trackActiveTriggers[8];
	SchmittTrigger fillTrigger;
	SchmittTrigger copyPasteTrigger;

	pattern patterns[8];
	float lastTickCount = 0.0f;
	float currentTickCount = 0.0f;
	float phase = 0.0f;
	size_t currentPattern = 0;
	size_t currentTrack = 0;
	size_t currentTrig = 0;
	size_t selector = 1;
	size_t trigPage = 0;
	size_t ppqn = 0;
	size_t pageIndex = 0;
	bool fill = false;
	size_t nextPattern = 0;
	int copyTrackId = -1;
	int copyPatternId = -1;

	ZOUMAI() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		patterns[currentPattern].reset(fill);
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "currentPattern", json_integer(currentPattern));
		for (int i = 0; i<8; i++) {
			json_t *patternJ = json_object();
			for (int j = 0; j < 8; j++) {
				json_t *trackJ = json_object();
				json_object_set_new(trackJ, "isActive", json_boolean(patterns[i].tracks[j].isActive));
				json_object_set_new(trackJ, "length", json_integer(patterns[i].tracks[j].length));
				json_object_set_new(trackJ, "speed", json_real(patterns[i].tracks[j].speed));
				json_object_set_new(trackJ, "readMode", json_integer(patterns[i].tracks[j].readMode));
				json_object_set_new(trackJ, "trackIndex", json_integer(patterns[i].tracks[j].trackIndex));
				json_object_set_new(trackJ, "swing", json_real(patterns[i].tracks[j].swing));
				for (int k = 0; k < 64; k++) {
					json_t *trigJ = json_object();
					json_object_set_new(trigJ, "isActive", json_boolean(patterns[i].tracks[j].trigs[k].isActive));
					json_object_set_new(trigJ, "slide", json_real(patterns[i].tracks[j].trigs[k].slide));
					json_object_set_new(trigJ, "trigType", json_integer(patterns[i].tracks[j].trigs[k].trigType));
					json_object_set_new(trigJ, "index", json_integer(patterns[i].tracks[j].trigs[k].index));
					json_object_set_new(trigJ, "reference", json_integer(patterns[i].tracks[j].trigs[k].reference));
					json_object_set_new(trigJ, "trim", json_integer(patterns[i].tracks[j].trigs[k].trim));
					json_object_set_new(trigJ, "length", json_integer(patterns[i].tracks[j].trigs[k].length));
					json_object_set_new(trigJ, "pulseCount", json_integer(patterns[i].tracks[j].trigs[k].pulseCount));
					json_object_set_new(trigJ, "pulseDistance", json_integer(patterns[i].tracks[j].trigs[k].pulseDistance));
					json_object_set_new(trigJ, "proba", json_integer(patterns[i].tracks[j].trigs[k].proba));
					json_object_set_new(trigJ, "count", json_integer(patterns[i].tracks[j].trigs[k].count));
					json_object_set_new(trigJ, "countReset", json_integer(patterns[i].tracks[j].trigs[k].countReset));
					json_object_set_new(trigJ, "VO", json_real(patterns[i].tracks[j].trigs[k].VO));
					json_object_set_new(trigJ, "CV1", json_real(patterns[i].tracks[j].trigs[k].CV1));
					json_object_set_new(trigJ, "CV2", json_real(patterns[i].tracks[j].trigs[k].CV2));
					json_object_set_new(trackJ, ("trig" + to_string(k)).c_str() , trigJ);
				}
				json_object_set_new(patternJ, ("track" + to_string(j)).c_str() , trackJ);
			}
			json_object_set_new(rootJ, ("pattern" + to_string(i)).c_str(), patternJ);
		}
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *currentPatternJ = json_object_get(rootJ, "currentPattern");
		if (currentPatternJ)
			currentPattern = json_integer_value(currentPatternJ);

		for (int i=0; i<8;i++) {
			json_t *patternJ = json_object_get(rootJ, ("pattern" + to_string(i)).c_str());
			if (patternJ){
				for(int j=0; j<8;j++) {
					json_t *trackJ = json_object_get(patternJ, ("track" + to_string(j)).c_str());
					if (trackJ){
						json_t *isActiveJ = json_object_get(trackJ, "isActive");
						if (isActiveJ)
							patterns[i].tracks[j].isActive = json_is_true(isActiveJ) ? 1 : 0;
						json_t *lengthJ = json_object_get(trackJ, "length");
						if (lengthJ)
							patterns[i].tracks[j].length = json_integer_value(lengthJ);
						json_t *speedJ = json_object_get(trackJ, "speed");
						if (speedJ)
							patterns[i].tracks[j].speed = json_number_value(speedJ);
						json_t *readModeJ = json_object_get(trackJ, "readMode");
						if (readModeJ)
							patterns[i].tracks[j].readMode = json_integer_value(readModeJ);
						json_t *trackIndexJ = json_object_get(trackJ, "trackIndex");
						if (trackIndexJ)
							patterns[i].tracks[j].trackIndex = json_integer_value(trackIndexJ);
						json_t *swingJ = json_object_get(trackJ, "swing");
						if (swingJ)
							patterns[i].tracks[j].swing = json_number_value(swingJ);
						}
						for(int k=0;k<64;k++) {
							json_t *trigJ = json_object_get(trackJ, ("trig" + to_string(k)).c_str());
							if (trigJ) {
								json_t *isActiveJ = json_object_get(trigJ, "isActive");
								if (isActiveJ)
									patterns[i].tracks[j].trigs[k].isActive = json_is_true(isActiveJ) ? 1 : 0;
								json_t *slideJ = json_object_get(trigJ, "slide");
								if (slideJ)
									patterns[i].tracks[j].trigs[k].slide = json_number_value(slideJ);
								json_t *trigTypeJ = json_object_get(trigJ, "trigType");
								if (trigTypeJ)
									patterns[i].tracks[j].trigs[k].trigType = json_integer_value(trigTypeJ);
								json_t *indexJ = json_object_get(trigJ, "index");
								if (indexJ)
									patterns[i].tracks[j].trigs[k].index = json_integer_value(indexJ);
								json_t *referenceJ = json_object_get(trigJ, "reference");
								if (referenceJ)
									patterns[i].tracks[j].trigs[k].reference = json_integer_value(referenceJ);
								json_t *trimJ = json_object_get(trigJ, "trim");
								if (trimJ)
									patterns[i].tracks[j].trigs[k].trim = json_integer_value(trimJ);
								json_t *lengthJ = json_object_get(trigJ, "length");
								if (lengthJ)
									patterns[i].tracks[j].trigs[k].length = json_integer_value(lengthJ);
								json_t *pulseCountJ = json_object_get(trigJ, "pulseCount");
								if (pulseCountJ)
									patterns[i].tracks[j].trigs[k].pulseCount = json_integer_value(pulseCountJ);
								json_t *pulseDistanceJ = json_object_get(trigJ, "pulseDistance");
								if (pulseDistanceJ)
									patterns[i].tracks[j].trigs[k].pulseDistance = json_integer_value(pulseDistanceJ);
								json_t *probaJ = json_object_get(trigJ, "proba");
								if (probaJ)
									patterns[i].tracks[j].trigs[k].proba = json_integer_value(probaJ);
								json_t *countJ = json_object_get(trigJ, "count");
								if (countJ)
									patterns[i].tracks[j].trigs[k].count = json_integer_value(countJ);
								json_t *countResetJ = json_object_get(trigJ, "countReset");
								if (countResetJ)
									patterns[i].tracks[j].trigs[k].countReset = json_integer_value(countResetJ);
								json_t *VOJ = json_object_get(trigJ, "VO");
								if (VOJ)
									patterns[i].tracks[j].trigs[k].VO = json_number_value(VOJ);
								json_t *CV1J = json_object_get(trigJ, "CV1");
								if (CV1J)
									patterns[i].tracks[j].trigs[k].CV1 = json_number_value(CV1J);
								json_t *CV2J = json_object_get(trigJ, "CV2");
								if (CV2J)
									patterns[i].tracks[j].trigs[k].CV2 = json_number_value(CV2J);
							}
						}
					}
				}
			}
	}

	void randomize() override {
		//patterns[currentPattern].tracks[currentTrack].randomize();
		for (size_t i = 0; i < 64; i++) {
			patterns[currentPattern].tracks[currentTrack].trigs[i].randomize();
		}
	}

	void reset() override {
		//patterns[currentPattern].tracks[currentTrack].resetValues();
		for (size_t i = 0; i < 64; i++) {
			patterns[currentPattern].tracks[currentTrack].trigs[i].resetValues();
		}
	}

	void step() override;

};

void ZOUMAI::step() {

	currentPattern = (int)clamp((inputs[PATTERN_INPUT].active ? rescale(clamp(inputs[PATTERN_INPUT].value, 0.0f, 10.0f),0.0f,10.0f,0.0f,7.0f) : 0) + (int)params[PATTERN_PARAM].value, 0.0f, 7.0f);

	if (inputs[FILL_INPUT].active) {
		if (((inputs[FILL_INPUT].value > 0.0f) && !fill) || ((inputs[FILL_INPUT].value == 0.0f) && fill)) fill=!fill;
	}
	if (fillTrigger.process(params[FILL_PARAM].value)) {
		fill = !fill;
	}
	lights[FILL_LIGHT].value = fill ? 10.0f : 0.0f;

	if (inputs[EXT_CLOCK_INPUT].active) {
		currentTickCount++;
		if (lastTickCount > 0.0f) {
			phase = currentTickCount / lastTickCount;
		}
		else {
			phase += engineGetSampleTime();
		}
		if (clockTrigger.process(inputs[EXT_CLOCK_INPUT].value)) {
			lastTickCount = currentTickCount;
			currentTickCount = 0.0f;
			phase = 0.0f;
			ppqn = 0;
		}
		if (resetTrigger.process(inputs[RESET_INPUT].value)) {
			phase = 0.0f;
			currentTickCount = 0.0f;
			for (size_t i = 0; i<8; i++) {
				patterns[i].reset(fill);
			}
			ppqn = 0;
		}
		else if (phase*192>=ppqn) {
			for (size_t i = 0; i<8; i++) {
				patterns[i].moveNext(fill);
			}
			ppqn++;
		}
	}

	for (size_t i = 0; i<4; i++) {
		if (trigPageTriggers[i].process(params[TRIG_PAGE_PARAM + i].value)) trigPage = i;
		if (trigPage == i) {
			lights[TRIG_PAGE_LIGHTS+(i*3)].value = 0.0f;
			lights[TRIG_PAGE_LIGHTS+(i*3)+1].value = 0.0f;
			lights[TRIG_PAGE_LIGHTS+(i*3)+2].value = 1.0f;
		}
		else if ((patterns[currentPattern].tracks[currentTrack].getCurrentTrig().index >= (i*16)) && (patterns[currentPattern].tracks[currentTrack].getCurrentTrig().index<(16*(i+1)-1))) {
			lights[TRIG_PAGE_LIGHTS+(i*3)].value = 1.0f*(1-phase);
			lights[TRIG_PAGE_LIGHTS+(i*3)+1].value = 0.5f*(1-phase);
			lights[TRIG_PAGE_LIGHTS+(i*3)+2].value = 0.0f*(1-phase);
		}
		else {
			lights[TRIG_PAGE_LIGHTS+(i*3)].value = 0.0f;
			lights[TRIG_PAGE_LIGHTS+(i*3)+1].value = 0.0f;
			lights[TRIG_PAGE_LIGHTS+(i*3)+2].value = 0.0f;
		}
	}

	lights[COPY_LIGHT].value = copyTrackId > -1 ? 10.0f : 0.0f;

	for (size_t i = 0; i<8; i++) {

		if (trackResetTriggers[i].process(inputs[TRACK_RESET_INPUTS+i].value)) {
			for (size_t j = 0; j<8; j++) {
				patterns[j].tracks[i].reset(fill,i==0?false:patterns[j].tracks[i-1].pre);
			}
		}

		if (trackActiveTriggers[i].process(inputs[TRACK_ACTIVE_INPUTS+i].value)) {
			patterns[currentPattern].tracks[i].isActive = !patterns[currentPattern].tracks[i].isActive;
		}

		if (patterns[currentPattern].tracks[i].isActive) {
			if (currentTrack == i) {
				lights[TRACKS_LIGHTS+(i*3)].value = 1.0f;
				lights[TRACKS_LIGHTS+(i*3)+1].value = 1.0f;
				lights[TRACKS_LIGHTS+(i*3)+2].value = 1.0f;
			}
			else {
				lights[TRACKS_LIGHTS+(i*3)].value = 1.0f;
				lights[TRACKS_LIGHTS+(i*3)+1].value = 0.91f;
				lights[TRACKS_LIGHTS+(i*3)+2].value = 0.35f;
			}
		}
		else {
			if (currentTrack == i) {
				lights[TRACKS_LIGHTS+(i*3)].value = 0.3f;
				lights[TRACKS_LIGHTS+(i*3)+1].value = 0.3f;
				lights[TRACKS_LIGHTS+(i*3)+2].value = 0.3f;
			}
			else {
				lights[TRACKS_LIGHTS+(i*3)].value = 0.0f;
				lights[TRACKS_LIGHTS+(i*3)+1].value = 0.0f;
				lights[TRACKS_LIGHTS+(i*3)+2].value = 0.0f;
			}
		}

		if (patterns[currentPattern].tracks[i].isActive){
			float gate = patterns[currentPattern].tracks[i].getGate();
			if (gate>0.0f) {
				if (patterns[currentPattern].tracks[i].getMemTrig().trigType == 0)
					outputs[GATE_OUTPUTS + i].value = gate;
				else if (patterns[currentPattern].tracks[i].getMemTrig().trigType == 1)
					outputs[GATE_OUTPUTS + i].value = inputs[G1_INPUT].value;
				else if (patterns[currentPattern].tracks[i].getMemTrig().trigType == 2)
					outputs[GATE_OUTPUTS + i].value = inputs[G2_INPUT].value;
				else
					outputs[GATE_OUTPUTS + i].value = 0.0f;
			}
			else
				outputs[GATE_OUTPUTS + i].value = 0.0f;
		}
		else
			outputs[GATE_OUTPUTS + i].value = 0.0f;

		outputs[VO_OUTPUTS + i].value = patterns[currentPattern].tracks[i].getVO();
		outputs[CV1_OUTPUTS + i].value = patterns[currentPattern].tracks[i].getCV1();
		outputs[CV2_OUTPUTS + i].value = patterns[currentPattern].tracks[i].getCV2();
	}

	for (size_t i = 0; i<16; i++) {
		size_t shiftedIndex = i + (trigPage*16);
		if (patterns[currentPattern].tracks[currentTrack].getCurrentTrig().index == shiftedIndex) {
			lights[STEPS_LIGHTS+(i*3)].value = 1.0f;
			lights[STEPS_LIGHTS+(i*3)+1].value = 0.5f;
			lights[STEPS_LIGHTS+(i*3)+2].value = 0.0f;
		}
		else if (patterns[currentPattern].tracks[currentTrack].trigs[shiftedIndex].isActive) {
			if (currentTrig == shiftedIndex) {
				lights[STEPS_LIGHTS+(i*3)].value = 1.0f;
				lights[STEPS_LIGHTS+(i*3)+1].value = 1.0f;
				lights[STEPS_LIGHTS+(i*3)+2].value = 1.0f;
			}
			else {
				lights[STEPS_LIGHTS+(i*3)].value = 1.0f;
				lights[STEPS_LIGHTS+(i*3)+1].value = 0.91f;
				lights[STEPS_LIGHTS+(i*3)+2].value = 0.35f;
			}
		}
		else if (currentTrig == shiftedIndex) {
			lights[STEPS_LIGHTS+(i*3)].value = 0.5f;
			lights[STEPS_LIGHTS+(i*3)+1].value = 0.5f;
			lights[STEPS_LIGHTS+(i*3)+2].value = 0.5f;
		}
		else {
			lights[STEPS_LIGHTS+(i*3)].value = 0.0f;
			lights[STEPS_LIGHTS+(i*3)+1].value = 0.0f;
			lights[STEPS_LIGHTS+(i*3)+2].value = 0.0f;
		}
	}
}

struct ZOUMAIDisplay : TransparentWidget {
	ZOUMAI *module;
	int frame = 0;
	shared_ptr<Font> font;
	const float noteC = 0.0f;
	const float noteCs = 1.0f/12.0f;
	const float noteD = 2.0f/12.0f;
	const float noteDs = 3.0f/12.0f;
	const float noteE = 4.0f/12.0f;
	const float noteF = 5.0f/12.0f;
	const float noteFs = 6.0f/12.0f;
	const float noteG = 7.0f/12.0f;
	const float noteGs = 8.0f/12.0f;
	const float noteA = 9.0f/12.0f;
	const float noteAs = 10.0f/12.0f;
	const float noteB = 11.0f/12.0f;



	ZOUMAIDisplay() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	void draw(NVGcontext *vg) override {
		nvgBeginPath(vg);
		nvgRect(vg, 0.0f, 0.0f, 205.0f, 120.0f);
		nvgClosePath(vg);
		nvgLineCap(vg, NVG_MITER);
		nvgStrokeWidth(vg, 0);
		nvgStroke(vg);
		nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
		nvgFill(vg);

		static const float portX0[4] = {30.0f, 78.0f, 125.0f, 174.0f};
		static const float portY0[4] = {40.0f, 60.0f, 90.0f, 110.0f};

		nvgFillColor(vg, YELLOW_BIDOO);

		if (module->selector == 0) {
			nvgFontSize(vg, 14.0f);
			nvgText(vg, 10.0f, 17.0f, ("Pattern " + to_string(module->currentPattern + 1) + " : Track " + to_string(module->currentTrack + 1)).c_str(), NULL);
			nvgFontSize(vg, 12.0f);
			nvgTextAlign(vg, NVG_ALIGN_CENTER);
			nvgText(vg, portX0[0], portY0[0], "Len", NULL);
			nvgText(vg, portX0[0], portY0[1], to_string(module->patterns[module->currentPattern].tracks[module->currentTrack].length).c_str(), NULL);
			nvgText(vg, portX0[1], portY0[0], "Speed", NULL);
			stringstream stream;
			stream << fixed << setprecision(2) << module->patterns[module->currentPattern].tracks[module->currentTrack].speed;
			nvgText(vg, portX0[1], portY0[1], ("x" + stream.str()).c_str(), NULL);
			nvgText(vg, portX0[2], portY0[0], "Read", NULL);
			nvgText(vg, portX0[2], portY0[1], (displayReadMode(module->patterns[module->currentPattern].tracks[module->currentTrack].readMode)).c_str(), NULL);
			nvgText(vg, portX0[3], portY0[0], "Swing", NULL);
			stream.str("");
			stream << fixed << setprecision(2) << module->patterns[module->currentPattern].tracks[module->currentTrack].swing;
			nvgText(vg, portX0[3], portY0[1], stream.str().c_str(), NULL);
		}
		else if (module->selector == 1) {
			nvgFontSize(vg, 14.0f);
			nvgText(vg, 10.0f, 17.0f, ("Pattern " + to_string(module->currentPattern + 1) + " : Track " + to_string(module->currentTrack + 1) + " : Trig " + to_string(module->currentTrig + 1)).c_str(), NULL);
			nvgFontSize(vg, 12.0f);
			nvgTextAlign(vg, NVG_ALIGN_CENTER);
			if (module->pageIndex==0) {
				nvgText(vg, portX0[0], portY0[0], "Len", NULL);
				stringstream stream;
				stream << fixed << setprecision(2) << (float)module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].length/192.0f;
				nvgText(vg, portX0[0], portY0[1], stream.str().c_str(), NULL);
				nvgText(vg, portX0[1], portY0[0], "Puls.", NULL);
				nvgText(vg, portX0[1], portY0[1], to_string(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].pulseCount).c_str(), NULL);
				nvgText(vg, portX0[2], portY0[0], "Dist.", NULL);
				stream.str("");
				stream << fixed << setprecision(2) << (float)module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].pulseDistance/192.0f;
				nvgText(vg, portX0[2], portY0[1], stream.str().c_str(), NULL);
				nvgText(vg, portX0[3], portY0[0], "Type", NULL);
				nvgText(vg, portX0[3], portY0[1], displayTrigType(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].trigType).c_str(), NULL);

				nvgText(vg, portX0[0], portY0[2], "V/Oct", NULL);
				nvgText(vg, portX0[0], portY0[3], displayNote(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].VO).c_str(), NULL);
				nvgText(vg, portX0[1], portY0[2], "Slide", NULL);
				stream.str("");
				stream << fixed << setprecision(2) << module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].slide;
				nvgText(vg, portX0[1], portY0[3], stream.str().c_str(), NULL);
				nvgText(vg, portX0[2], portY0[2], "CV1", NULL);
				stream.str("");
				stream << fixed << setprecision(2) << module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].CV1;
				nvgText(vg, portX0[2], portY0[3], stream.str().c_str(), NULL);
				nvgText(vg, portX0[3], portY0[2], "CV2", NULL);
				stream.str("");
				stream << fixed << setprecision(2) << module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].CV2;
				nvgText(vg, portX0[3], portY0[3], stream.str().c_str(), NULL);
			}
			else {
				nvgText(vg, portX0[0], portY0[0], "Trim", NULL);
				nvgText(vg, portX0[0], portY0[1], to_string(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].trim).c_str(), NULL);
				nvgText(vg, portX0[1], portY0[0], "Prob.", NULL);
				nvgText(vg, portX0[1], portY0[1], displayProba(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].proba).c_str(), NULL);
				if (module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].proba < 2)
				{
					nvgText(vg, portX0[2], portY0[0], "Val.", NULL);
					nvgText(vg, portX0[2], portY0[1], to_string(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].count).c_str(), NULL);
				}
				if (module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].proba == 1)
				{
					nvgText(vg, portX0[3], portY0[0], "Base", NULL);
					nvgText(vg, portX0[3], portY0[1], to_string(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].countReset).c_str(), NULL);
				}
			}
		}
	}

	string displayReadMode(int value) {
		switch(value){
			case 0: return "►";
			case 1: return "◄";
			case 2: return "►◄";
			case 3: return "►*";
			case 4: return "►?";
			default: return "";
		}
	}

	string displayTrigType(int value) {
		switch(value){
			case 0: return "INT";
			case 1: return "GTE1";
			case 2: return "GTE2";
			default: return "";
		}
	}

	string displayProba(int value) {
		switch(value){
			case 0: return "DICE";
			case 1: return "COUNT";
			case 2: return "FILL";
			case 3: return "!FILL";
			case 4: return "PRE";
			case 5: return "!PRE";
			case 6: return "NEI";
			case 7: return "!NEI";
			default: return "";
		}
	}

	string displayNote(float value) {
		double fractpart, intpart;
		fractpart = modf (value , &intpart);
		string result = "";
		if (fractpart < noteCs)
			return "C" + to_string((int)intpart);
		else if (fractpart < noteD)
			return "C#" + to_string((int)intpart);
		else if (fractpart < noteDs)
			return "D" + to_string((int)intpart);
		else if (fractpart < noteE)
			return "D#" + to_string((int)intpart);
		else if (fractpart < noteF)
			return "E" + to_string((int)intpart);
		else if (fractpart < noteFs)
			return "F" + to_string((int)intpart);
		else if (fractpart < noteG)
			return "F#" + to_string((int)intpart);
		else if (fractpart < noteGs)
			return "G" + to_string((int)intpart);
		else if (fractpart < noteA)
			return "G#" + to_string((int)intpart);
		else if (fractpart < noteAs)
			return "A" + to_string((int)intpart);
		else if (fractpart < noteB)
			return "A#" + to_string((int)intpart);
		else
			return "B" + to_string((int)intpart);
	}
};

struct ZOUMAIWidget : ModuleWidget {
	ParamWidget *stepsParam[16], *tracksParam[8], *freeParam[8], *patternParam;
	TransparentWidget *selector[2];
	ZOUMAIWidget(ZOUMAI *module);
};

struct ZOUMAIPageSelector : TransparentWidget {
	ZOUMAI *module;
	size_t index = 0;
	shared_ptr<Font> font;
	string text;

	ZOUMAIPageSelector() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	void draw(NVGcontext *vg) override {

		if (module->pageIndex == index) {
			nvgLineCap(vg, NVG_MITER);
			nvgStrokeWidth(vg, 0);
			nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
			{
				nvgBeginPath(vg);
				nvgMoveTo(vg, 0, 0);
				nvgLineTo(vg, 10, 0);
				nvgLineTo(vg, 10,25);
				nvgLineTo(vg, 0,25);
				nvgClosePath(vg);
			}
			nvgStroke(vg);
			nvgFill(vg);
		}
		else {
			nvgLineCap(vg, NVG_MITER);
			nvgStrokeWidth(vg, 4);
			nvgStrokeColor(vg, nvgRGBA(180,180,180,255));
			nvgFillColor(vg,nvgRGBA(180,180,180,255));
			{
				nvgBeginPath(vg);
				nvgMoveTo(vg, 0, 2);
				nvgLineTo(vg, 8, 2);
				nvgLineTo(vg, 8,23);
				nvgLineTo(vg, 0,23);
				nvgClosePath(vg);
			}
			nvgStroke(vg);
			nvgFill(vg);
		}
	}

	virtual void onMouseDown(EventMouseDown &e) override {
		TransparentWidget::onMouseDown(e);
		ZOUMAIWidget *parent = dynamic_cast<ZOUMAIWidget*>(this->parent);
		module->pageIndex = index;
		if (index == 0) {
			parent->freeParam[0]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].length+0.5f,0.0f,3000.0f,1.0f,5.0f));
			parent->freeParam[1]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].pulseCount+0.5f,1.0f,192.0f,1.0f,5.0f));
			parent->freeParam[2]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].pulseDistance+0.5f,1.0f,768.0f,1.0f,5.0f));
			parent->freeParam[3]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].trigType+0.5f,0.0f,2.0f,1.0f,5.0f));
			parent->freeParam[4]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].VO+0.001f,0.0f,10.0f,1.0f,5.0f));
			parent->freeParam[5]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].slide,0.0f,10.0f,1.0f,5.0f));
			parent->freeParam[6]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].CV1,0.0f,10.0f,1.0f,5.0f));
			parent->freeParam[7]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].CV2,0.0f,10.0f,1.0f,5.0f));
			parent->freeParam[0]->visible = true;
			parent->freeParam[1]->visible = true;
			parent->freeParam[2]->visible = true;
			parent->freeParam[3]->visible = true;
			parent->freeParam[4]->visible = true;
			parent->freeParam[5]->visible = true;
			parent->freeParam[6]->visible = true;
			parent->freeParam[7]->visible = true;
		}
		else
		{
			parent->freeParam[0]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].trim+0.5f,-191.0f,191.0f,1.0f,5.0f));
			parent->freeParam[1]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].proba+0.5f,0.0f,7.0f,1.0f,5.0f));
			parent->freeParam[2]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].count+0.5f,1.0f,100.0f,1.0f,5.0f));
			parent->freeParam[3]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].countReset+0.5f,1.0f,100.0f,1.0f,5.0f));
			parent->freeParam[0]->visible = true;
			parent->freeParam[1]->visible = true;
			if (module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].proba == 0) {
				parent->freeParam[2]->visible = true;
				parent->freeParam[3]->visible = false;
			}
			else if (module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].proba == 1) {
				parent->freeParam[2]->visible = true;
				parent->freeParam[3]->visible = true;
			}
			else {
				parent->freeParam[2]->visible = false;
				parent->freeParam[3]->visible = false;
			}

			parent->freeParam[4]->visible = false;
			parent->freeParam[5]->visible = false;
			parent->freeParam[6]->visible = false;
			parent->freeParam[7]->visible = false;
		}
	}

};

template <typename BASE>
struct ZOUMAILight : BASE {
	ZOUMAILight() {
		this->box.size = mm2px(Vec(6.0f, 6.0f));
	}
};

struct ZOUMAITRIGLEDBezel : LEDBezel {
	size_t index;
	virtual void onMouseDown(EventMouseDown &e) override {
		LEDBezel::onMouseDown(e);
		ZOUMAIWidget *parent = dynamic_cast<ZOUMAIWidget*>(this->parent);
		ZOUMAI *module = dynamic_cast<ZOUMAI*>(this->module);
		if (parent && module) {
			if ((e.button == RACK_MOUSE_BUTTON_MIDDLE) || (e.button == RACK_MOUSE_BUTTON_RIGHT))  {
				module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[index + (module->trigPage * 16)].isActive = !module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[index + (module->trigPage * 16)].isActive;
			}
			else if (e.button == RACK_MOUSE_BUTTON_LEFT) {
				module->currentTrig = index + (module->trigPage * 16);
				module->selector = 1;

				parent->selector[0]->visible = true;
				parent->selector[1]->visible = true;

				if (module->pageIndex == 0) {
					parent->freeParam[0]->visible = true;
					parent->freeParam[1]->visible = true;
					parent->freeParam[2]->visible = true;
					parent->freeParam[3]->visible = true;
					parent->freeParam[4]->visible = true;
					parent->freeParam[5]->visible = true;
					parent->freeParam[6]->visible = true;
					parent->freeParam[7]->visible = true;

					parent->freeParam[0]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].length+0.5f,0.0f,3000.0f,1.0f,5.0f));
					parent->freeParam[1]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].pulseCount+0.5f,1.0f,192.0f,1.0f,5.0f));
					parent->freeParam[2]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].pulseDistance+0.5f,1.0f,768.0f,1.0f,5.0f));
					parent->freeParam[3]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].trigType+0.5f,0.0f,2.0f,1.0f,5.0f));
					parent->freeParam[4]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].VO+0.001f,0.0f,10.0f,1.0f,5.0f));
					parent->freeParam[5]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].slide,0.0f,10.0f,1.0f,5.0f));
					parent->freeParam[6]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].CV1,0.0f,10.0f,1.0f,5.0f));
					parent->freeParam[7]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].CV2,0.0f,10.0f,1.0f,5.0f));

				}
				else {
					parent->freeParam[0]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].trim+0.5f,-191.0f,191.0f,1.0f,5.0f));
					parent->freeParam[1]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].proba+0.5f,0.0f,7.0f,1.0f,5.0f));
					parent->freeParam[2]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].count+0.5f,1.0f,100.0f,1.0f,5.0f));
					parent->freeParam[3]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].countReset+0.5f,1.0f,100.0f,1.0f,5.0f));
					parent->freeParam[0]->visible = true;
					parent->freeParam[1]->visible = true;
					if (module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].proba == 0) {
						parent->freeParam[2]->visible = true;
						parent->freeParam[3]->visible = false;
					}
					else if (module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].proba == 1) {
						parent->freeParam[2]->visible = true;
						parent->freeParam[3]->visible = true;
					}
					else {
						parent->freeParam[2]->visible = false;
						parent->freeParam[3]->visible = false;
					}

					parent->freeParam[4]->visible = false;
					parent->freeParam[5]->visible = false;
					parent->freeParam[6]->visible = false;
					parent->freeParam[7]->visible = false;
				}

			}
		}
	}
};

struct ZOUMAITRACKLEDBezel : LEDBezel {
	size_t index;

	virtual void onMouseDown(EventMouseDown &e) override {
		LEDBezel::onMouseDown(e);
		ZOUMAIWidget *parent = dynamic_cast<ZOUMAIWidget*>(this->parent);
		ZOUMAI *module = dynamic_cast<ZOUMAI*>(this->module);
		if (parent && module) {
			if ((e.button == RACK_MOUSE_BUTTON_MIDDLE) || (e.button == RACK_MOUSE_BUTTON_RIGHT)) {
				module->patterns[module->currentPattern].tracks[index].isActive = !module->patterns[module->currentPattern].tracks[index].isActive;
			}
			else if (e.button == RACK_MOUSE_BUTTON_LEFT) {
				module->currentTrack = index;
				module->selector = 0;

				parent->freeParam[0]->visible = true;
				parent->freeParam[1]->visible = true;
				parent->freeParam[2]->visible = true;
				parent->freeParam[3]->visible = true;
				parent->freeParam[4]->visible = false;
				parent->freeParam[5]->visible = false;
				parent->freeParam[6]->visible = false;
				parent->freeParam[7]->visible = false;

				parent->selector[0]->visible = false;
				parent->selector[1]->visible = false;

				parent->freeParam[0]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].length+0.5f,1.0f,64.0f,1.0f,5.0f));
				parent->freeParam[1]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].speed,0.5f,16.0f,1.0f,5.0f));
				parent->freeParam[2]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].readMode+0.5f,0.0f,4.0f,1.0f,5.0f));
				parent->freeParam[3]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].swing,0.0f,100.0f,1.0f,5.0f));
			}
		}
	}
};

struct ZOUMAIFREEPARAMBlueKnob : BidooBlueKnob {
		size_t index;
		virtual void onChange(EventChange &e) override {
			BidooBlueKnob::onChange(e);
			ZOUMAIWidget *parent = dynamic_cast<ZOUMAIWidget*>(this->parent);
			ZOUMAI *module = dynamic_cast<ZOUMAI*>(this->module);
			if (parent && module) {
				if (module->selector == 0) {
					switch (index) {
						case 0 :
							module->patterns[module->currentPattern].tracks[module->currentTrack].length = (int)rescale(value,1.0f,5.0f,1.0f,64.0f);
							break;
						case 1 :
							{
								float val = rescale(value,1.0f,5.0f,0.5f,16.0f);
								int i = (int)val;
								float f = (val-i) < 0.5f ? 0.0f : 0.5f;
								module->patterns[module->currentPattern].tracks[module->currentTrack].speed = (float)i + f;
								break;
							}
						case 2 :
							module->patterns[module->currentPattern].tracks[module->currentTrack].readMode = (int)rescale(value,1.0f,5.0f,0.0f,4.0f);
							break;
						case 3 :
							module->patterns[module->currentPattern].tracks[module->currentTrack].swing = rescale(value,1.0f,5.0f,0.0f,100.0f);
							break;
					}
				}
				else if (module->selector == 1) {
					if (module->pageIndex == 0) {
						switch (index) {
							case 0 :
								module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].length = (int)rescale(value,1.0f,5.0f,0.0f,3000.0f);
								break;
							case 1 :
								module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].pulseCount = (int)rescale(value,1.0f,5.0f,1.0f,192.0f);
								break;
							case 2 :
								module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].pulseDistance = (int)rescale(value,1.0f,5.0f,1.0f,768.0f);
								break;
							case 3 :
								module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].trigType = (int)rescale(value,1.0f,5.0f,0.0f,2.0f);
								break;
							case 4 : {
								float val = rescale(value,1.0f,5.0f,0.0f,10.0f);
								int r = (int)val;
								float n = ((int)rescale(val-r,0.0f,1.0f,0.0f,12.0f))/12.0f;
								module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].VO = (float)r + n;
								break;
							}
							case 5 :
								module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].slide = rescale(value,1.0f,5.0f,0.0f,10.0f);
								break;
							case 6 :
								module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].CV1 = rescale(value,1.0f,5.0f,0.0f,10.0f);
								break;
							case 7 :
								module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].CV2 = rescale(value,1.0f,5.0f,0.0f,10.0f);
								break;
						}
					}
					else {
						switch (index) {
							case 0 :
								module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].trim = (int)rescale(value,1.0f,5.0f,-191.0f,191.0f);
								break;
							case 1 : {
								module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].proba = (int)rescale(value,1.0f,5.0f,0.0f,7.0f);

								if (module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].proba == 0) {
									parent->freeParam[2]->visible = true;
									parent->freeParam[3]->visible = false;
								}
								else if (module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].proba == 1) {
									parent->freeParam[2]->visible = true;
									parent->freeParam[3]->visible = true;
								}
								else {
									parent->freeParam[2]->visible = false;
									parent->freeParam[3]->visible = false;
								}
								break;
							}
							case 2 :
								module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].count = (int)rescale(value,1.0f,5.0f,1.0f,100.0f);
								break;
							case 3 :
								module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].countReset = (int)rescale(value,1.0f,5.0f,1.0f,100.0f);
								break;
						}
					}
			}
		}
	}
};

struct ZOUMAIPatternRoundBlackSnapKnob : RoundBlackSnapKnob {
	void onChange(EventChange &e) override {
			RoundBlackSnapKnob::onChange(e);

	}
};

struct ZOUMAICOPYPASTECKD6 : BlueCKD6 {
	void onMouseDown(EventMouseDown &e) override {
		// ZOUMAIWidget *widget = dynamic_cast<ZOUMAIWidget*>(this->parent);
		ZOUMAI *module = dynamic_cast<ZOUMAI*>(this->module);
		if ((module->copyTrackId == -1) && (module->copyPatternId == -1)) {
			module->copyTrackId = module->currentTrack;
			module->copyPatternId = module->currentPattern;
		}
		else if ((module->copyTrackId > -1) && (module->copyPatternId > -1) && ((module->copyTrackId != (int)module->currentTrack) || (module->copyPatternId != (int)module->currentPattern)))
		{
			((module->patterns + module->currentPattern)->tracks + module->currentTrack)->copy((module->patterns + module->copyPatternId)->tracks + module->copyTrackId);
			module->copyTrackId = -1;
			module->copyPatternId = -1;
		}
		BlueCKD6::onMouseDown(e);
	}
};

ZOUMAIWidget::ZOUMAIWidget(ZOUMAI *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/ZOUMAI.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	{
		ZOUMAIPageSelector *tab1 = new ZOUMAIPageSelector();
		tab1->module = module;
		tab1->index = 0;
		tab1->box.pos = Vec(335.0f, 55.0f);
		tab1->box.size = Vec(12.0f, 30.0f);
		selector[0] = tab1;
		addChild(tab1);
	}

	{
		ZOUMAIPageSelector *tab2 = new ZOUMAIPageSelector();
		tab2->module = module;
		tab2->index = 1;
		tab2->box.pos = Vec(335.0f, 84.0f);
		tab2->box.size = Vec(12.0f, 30.0f);
		selector[1] = tab2;
		addChild(tab2);
	}

	{
		ZOUMAIDisplay *display = new ZOUMAIDisplay();
		display->module = module;
		display->box.pos = Vec(130.0f, 55.0f);
		display->box.size = Vec(205.0f, 120.0f);
		addChild(display);
	}

	static const float portY0[7] = {50.0f, 90.0f, 130.0f, 170.0f, 210.0f, 250.0f, 290.0f};

	addInput(Port::create<PJ301MPort>(Vec(10.0f, portY0[0]), Port::INPUT, module, ZOUMAI::EXT_CLOCK_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(10.0f, portY0[1]), Port::INPUT, module, ZOUMAI::RESET_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(10.0f, portY0[2]), Port::INPUT, module, ZOUMAI::G1_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(10.0f, portY0[3]), Port::INPUT, module, ZOUMAI::G2_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(10.0f, portY0[4]), Port::INPUT, module, ZOUMAI::PATTERN_INPUT));

	addInput(Port::create<PJ301MPort>(Vec(10.0f, portY0[6]), Port::INPUT, module, ZOUMAI::FILL_INPUT));
	addParam(ParamWidget::create<BlueCKD6>(Vec(40.0f, portY0[6]-1.0f), module, ZOUMAI::FILL_PARAM, 0.0f, 10.0f, 0.0f));
	addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(73.0f, portY0[6]+10.0f), module, ZOUMAI::FILL_LIGHT));

	static const float portX0[4] = {389.0f, 404.0f, 419.0f, 434.0f};
	addParam(ParamWidget::create<MiniLEDButton>(Vec(portX0[0], 312.0f), module, ZOUMAI::TRIG_PAGE_PARAM, 0.0f, 10.0f,  0.0f));
	addParam(ParamWidget::create<MiniLEDButton>(Vec(portX0[1], 312.0f), module, ZOUMAI::TRIG_PAGE_PARAM+1, 0.0f, 10.0f,  0.0f));
	addParam(ParamWidget::create<MiniLEDButton>(Vec(portX0[2], 312.0f), module, ZOUMAI::TRIG_PAGE_PARAM+2, 0.0f, 10.0f,  0.0f));
	addParam(ParamWidget::create<MiniLEDButton>(Vec(portX0[3], 312.0f), module, ZOUMAI::TRIG_PAGE_PARAM+3, 0.0f, 10.0f,  0.0f));

	addChild(ModuleLightWidget::create<SmallLight<RedGreenBlueLight>>(Vec(portX0[0], 312.0f), module, ZOUMAI::TRIG_PAGE_LIGHTS));
	addChild(ModuleLightWidget::create<SmallLight<RedGreenBlueLight>>(Vec(portX0[1], 312.0f), module, ZOUMAI::TRIG_PAGE_LIGHTS+3));
	addChild(ModuleLightWidget::create<SmallLight<RedGreenBlueLight>>(Vec(portX0[2], 312.0f), module, ZOUMAI::TRIG_PAGE_LIGHTS+6));
	addChild(ModuleLightWidget::create<SmallLight<RedGreenBlueLight>>(Vec(portX0[3], 312.0f), module, ZOUMAI::TRIG_PAGE_LIGHTS+9));

	patternParam = ParamWidget::create<ZOUMAIPatternRoundBlackSnapKnob>(Vec(115.0f,287.0f), module, ZOUMAI::PATTERN_PARAM, 0.0f, 7.0f, 0.0f);
	addParam(patternParam);

	addParam(ParamWidget::create<ZOUMAICOPYPASTECKD6>(Vec(400.0f, 270.0f), module, ZOUMAI::COPY_PARAM, 0.0f, 1.0f, 0.0f));
	addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(433.0f,281.0f), module, ZOUMAI::COPY_LIGHT));

	for (size_t i=0;i<16;i++){
		stepsParam[i] = ParamWidget::create<ZOUMAITRIGLEDBezel>(Vec(12.0f+ 28.0f*i, 330.0f), module, ZOUMAI::STEPS_PARAMS + i, 0.0f, 2.0f, 0.0f);
		ZOUMAITRIGLEDBezel *btnTrig = dynamic_cast<ZOUMAITRIGLEDBezel*>(stepsParam[i]);
		btnTrig->index = i;
		addParam(stepsParam[i]);
		addChild(ModuleLightWidget::create<ZOUMAILight<RedGreenBlueLight>>(Vec(14.0f+ 28.0f*i, 332.0f), module, ZOUMAI::STEPS_LIGHTS + i*3));
	}

	for (size_t i=0;i<8;i++){
		addInput(Port::create<TinyPJ301MPort>(Vec(50.0f, 55.0f + i*28.0f), Port::INPUT, module, ZOUMAI::TRACK_ACTIVE_INPUTS + i));
		addInput(Port::create<TinyPJ301MPort>(Vec(70.0f, 55.0f + i*28.0f), Port::INPUT, module, ZOUMAI::TRACK_RESET_INPUTS + i));
		tracksParam[i] = ParamWidget::create<ZOUMAITRACKLEDBezel>(Vec(90.0f , 51.0f + i*28.0f), module, ZOUMAI::TRACK_PARAMS + i, 0.0f, 2.0f, 10.0f);
		ZOUMAITRACKLEDBezel *btnTrack = dynamic_cast<ZOUMAITRACKLEDBezel*>(tracksParam[i]);
		btnTrack->index = i;
		addParam(tracksParam[i]);
		addChild(ModuleLightWidget::create<ZOUMAILight<RedGreenBlueLight>>(Vec(92.0f, 53.0f + i*28.0f), module, ZOUMAI::TRACKS_LIGHTS + i*3));

		freeParam[i] = ParamWidget::create<ZOUMAIFREEPARAMBlueKnob>(Vec(147.0f + i*48 + (i>3?-192.0f:0.0f), 190.0f + (i>3?50.0f:0.0f)), module, ZOUMAI::FREE_PARAMS + i, 1.0f, 5.0f, 10.0f);
		ZOUMAIFREEPARAMBlueKnob *btnFree = dynamic_cast<ZOUMAIFREEPARAMBlueKnob*>(freeParam[i]);
		btnFree->index = i;
		addParam(freeParam[i]);

		addOutput(Port::create<TinyPJ301MPort>(Vec(375.0f, 55.0f + i * 22.0f), Port::OUTPUT, module, ZOUMAI::GATE_OUTPUTS + i));
		addOutput(Port::create<TinyPJ301MPort>(Vec(395.0f, 55.0f + i * 22.0f), Port::OUTPUT, module, ZOUMAI::VO_OUTPUTS + i));
		addOutput(Port::create<TinyPJ301MPort>(Vec(415.0f, 55.0f + i * 22.0f), Port::OUTPUT, module, ZOUMAI::CV1_OUTPUTS + i));
		addOutput(Port::create<TinyPJ301MPort>(Vec(435.0f, 55.0f + i * 22.0f), Port::OUTPUT, module, ZOUMAI::CV2_OUTPUTS + i));
	}

	freeParam[0]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].length+0.5f,0.0f,3000.0f,1.0f,5.0f));
	freeParam[1]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].pulseCount+0.5f,1.0f,192.0f,1.0f,5.0f));
	freeParam[2]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].pulseDistance+0.5f,1.0f,768.0f,1.0f,5.0f));
	freeParam[3]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].trigType+0.5f,0.0f,2.0f,1.0f,5.0f));
	freeParam[4]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].VO,0.0f,10.0f,1.0f,5.0f));
	freeParam[5]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].slide,0.0f,10.0f,1.0f,5.0f));
	freeParam[6]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].CV1,0.0f,10.0f,1.0f,5.0f));
	freeParam[7]->setValue(rescale(module->patterns[module->currentPattern].tracks[module->currentTrack].trigs[module->currentTrig].CV2,0.0f,10.0f,1.0f,5.0f));
}

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, ZOUMAI) {
   Model *modelZOUMAI = Model::create<ZOUMAI, ZOUMAIWidget>("Bidoo", "zOù MAï", "zOù MAï sequencer", SEQUENCER_TAG);
   return modelZOUMAI;
}
