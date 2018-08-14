//***********************************************************************************************
//Impromptu Modular: Modules for VCV Rack by Marc Boulé
//
//Based on code from the Fundamental and AudibleInstruments plugins by Andrew Belt 
//and graphics from the Component Library by Wes Milholen 
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//***********************************************************************************************

#include "ImpromptuModular.hpp"

RACK_PLUGIN_MODEL_DECLARE(ImpromptuModular, Tact);
RACK_PLUGIN_MODEL_DECLARE(ImpromptuModular, TwelveKey);
RACK_PLUGIN_MODEL_DECLARE(ImpromptuModular, Clocked);
RACK_PLUGIN_MODEL_DECLARE(ImpromptuModular, MidiFile);
RACK_PLUGIN_MODEL_DECLARE(ImpromptuModular, PhraseSeq16);
RACK_PLUGIN_MODEL_DECLARE(ImpromptuModular, PhraseSeq32);
RACK_PLUGIN_MODEL_DECLARE(ImpromptuModular, GateSeq64);
RACK_PLUGIN_MODEL_DECLARE(ImpromptuModular, WriteSeq32);
RACK_PLUGIN_MODEL_DECLARE(ImpromptuModular, WriteSeq64);
RACK_PLUGIN_MODEL_DECLARE(ImpromptuModular, BigButtonSeq);
RACK_PLUGIN_MODEL_DECLARE(ImpromptuModular, SemiModularSynth);
RACK_PLUGIN_MODEL_DECLARE(ImpromptuModular, BlankPanel);

RACK_PLUGIN_INIT(ImpromptuModular) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/MarcBoule/ImpromptuModular");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/MarcBoule/ImpromptuModular");

	//p->addModel(modelEngTest1);
   RACK_PLUGIN_MODEL_ADD(ImpromptuModular, Tact);
   RACK_PLUGIN_MODEL_ADD(ImpromptuModular, TwelveKey);
   RACK_PLUGIN_MODEL_ADD(ImpromptuModular, Clocked);
   RACK_PLUGIN_MODEL_ADD(ImpromptuModular, MidiFile);
   RACK_PLUGIN_MODEL_ADD(ImpromptuModular, PhraseSeq16);
   RACK_PLUGIN_MODEL_ADD(ImpromptuModular, PhraseSeq32);
   RACK_PLUGIN_MODEL_ADD(ImpromptuModular, GateSeq64);
   RACK_PLUGIN_MODEL_ADD(ImpromptuModular, WriteSeq32);
   RACK_PLUGIN_MODEL_ADD(ImpromptuModular, WriteSeq64);
   RACK_PLUGIN_MODEL_ADD(ImpromptuModular, BigButtonSeq);
   RACK_PLUGIN_MODEL_ADD(ImpromptuModular, SemiModularSynth);
   RACK_PLUGIN_MODEL_ADD(ImpromptuModular, BlankPanel);
}


LEDBezelBig::LEDBezelBig() {
	float ratio = 2.13f;
	addFrame(SVG::load(assetGlobal("res/ComponentLibrary/LEDBezel.svg")));
	sw->box.size = sw->box.size.mult(ratio);
	box.size = sw->box.size;
	tw = new TransformWidget();
	removeChild(sw);
	tw->addChild(sw);
	
	addChild(tw);
	
	tw->box.size = sw->box.size; 
	tw->scale(Vec(ratio, ratio));
}


void InvisibleKeySmall::onMouseDown(EventMouseDown &e) {
	if (e.button == 1) {// if right button (see events.hpp)
		maxValue = 2.0f;
		// Simulate MomentarySwitch::onDragStart() since not called for right clicks:
		setValue(maxValue);
		EventAction eAction;
		onAction(eAction);
	}
	else 
		maxValue = 1.0f;
	//ParamWidget::onMouseDown(e);// don't want the reset() that is called in ParamWidget::onMouseDown(), so implement rest of that function here:
	e.consumed = true;
	e.target = this;
}
void InvisibleKeySmall::onMouseUp(EventMouseUp &e) {
	if (e.button == 1) {// if right button (see events.hpp)
		// Simulate MomentarySwitch::onDragEnd() since not called for right clicks:
		setValue(minValue);
	}
	ParamWidget::onMouseUp(e);
}


ScrewSilverRandomRot::ScrewSilverRandomRot() {
	float angle0_90 = randomUniform()*M_PI/2.0f;
	//float angle0_90 = randomUniform() > 0.5f ? M_PI/4.0f : 0.0f;// for testing
	
	tw = new TransformWidget();
	addChild(tw);
	
	sw = new SVGWidget();
	tw->addChild(sw);
	//sw->setSVG(SVG::load(assetPlugin(plugin, "res/Screw0.svg")));
	sw->setSVG(SVG::load(assetGlobal("res/ComponentLibrary/ScrewSilver.svg")));
	
	sc = new ScrewCircle(angle0_90);
	sc->box.size = sw->box.size;
	tw->addChild(sc);
	
	box.size = sw->box.size;
	tw->box.size = sw->box.size; 
	tw->identity();
	// Rotate SVG
	Vec center = sw->box.getCenter();
	tw->translate(center);
	tw->rotate(angle0_90);
	tw->translate(center.neg());	
}


ScrewHole::ScrewHole(Vec posGiven) {
	box.size = Vec(16, 7);
	box.pos = Vec(posGiven.x, posGiven.y + 4);// nudgeX for realism, 0 = no nudge
}
void ScrewHole::draw(NVGcontext *vg) {
	NVGcolor backgroundColor = nvgRGB(0x10, 0x10, 0x10); 
	NVGcolor borderColor = nvgRGB(0x20, 0x20, 0x20);
	nvgBeginPath(vg);
	nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 2.5f);
	nvgFillColor(vg, backgroundColor);
	nvgFill(vg);
	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, borderColor);
	nvgStroke(vg);
}


NVGcolor prepareDisplay(NVGcontext *vg, Rect *box) {
	NVGcolor backgroundColor = nvgRGB(0x38, 0x38, 0x38); 
	NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
	nvgBeginPath(vg);
	nvgRoundedRect(vg, 0.0, 0.0, box->size.x, box->size.y, 5.0);
	nvgFillColor(vg, backgroundColor);
	nvgFill(vg);
	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, borderColor);
	nvgStroke(vg);
	nvgFontSize(vg, 18);
	NVGcolor textColor = nvgRGB(0xaf, 0xd2, 0x2c);
	return textColor;
}


int moveIndex(int index, int indexNext, int numSteps) {
	if (indexNext < 0)
		index = numSteps - 1;
	else
	{
		if (indexNext - index >= 0) { // if moving right or same place
			if (indexNext >= numSteps)
				index = 0;
			else
				index = indexNext;
		}
		else { // moving left 
			if (indexNext >= numSteps)
				index = numSteps - 1;
			else
				index = indexNext;
		}
	}
	return index;
}


bool moveIndexRunMode(int* index, int numSteps, int runMode, int* history) {		
	bool crossBoundary = false;
	int numRuns;// for FWx
	
	switch (runMode) {
	
		case MODE_REV :// reverse; history base is 1000 (not needed)
			(*history) = 1000;
			(*index)--;
			if ((*index) < 0) {
				(*index) = numSteps - 1;
				crossBoundary = true;
			}
		break;
		
		case MODE_PPG :// forward-reverse; history base is 2000
			if ((*history) != 2000 && (*history) != 2001) // 2000 means going forward, 2001 means going reverse
				(*history) = 2000;
			if ((*history) == 2000) {// forward phase
				(*index)++;
				if ((*index) >= numSteps) {
					(*index) = numSteps - 1;
					(*history) = 2001;
				}
			}
			else {// it is 2001; reverse phase
				(*index)--;
				if ((*index) < 0) {
					(*index) = 0;
					(*history) = 2000;
					crossBoundary = true;
				}
			}
		break;
		
		case MODE_BRN :// brownian random; history base is 3000
			if ( (*history) < 3000 || ((*history) > (3000 + numSteps)) ) 
				(*history) = 3000 + numSteps;
			(*index) += (randomu32() % 3) - 1;
			if ((*index) >= numSteps) {
				(*index) = 0;
			}
			if ((*index) < 0) {
				(*index) = numSteps - 1;
			}
			(*history)--;
			if ((*history) <= 3000) {
				(*history) = 3000 + numSteps;
				crossBoundary = true;
			}
		break;
		
		case MODE_RND :// random; history base is 4000
			if ( (*history) < 4000 || ((*history) > (4000 + numSteps)) ) 
				(*history) = 4000 + numSteps;
			(*index) = (randomu32() % numSteps) ;
			(*history)--;
			if ((*history) <= 4000) {
				(*history) = 4000 + numSteps;
				crossBoundary = true;
			}
		break;
		
		case MODE_FW2 :// forward twice
		case MODE_FW3 :// forward three times
		case MODE_FW4 :// forward four times
			numRuns = 5002 + runMode - MODE_FW2;
			if ( (*history) < 5000 || (*history) >= numRuns ) // 5000 means first pass, 5001 means 2nd pass, etc...
				(*history) = 5000;
			(*index)++;
			if ((*index) >= numSteps) {
				(*index) = 0;
				(*history)++;
				if ((*history) >= numRuns) {
					(*history) = 5000;
					crossBoundary = true;
				}				
			}
		break;

		default :// MODE_FWD  forward; history base is 0 (not needed)
			(*history) = 0;
			(*index)++;
			if ((*index) >= numSteps) {
				(*index) = 0;
				crossBoundary = true;
			}
	}

	return crossBoundary;
}

bool calcWarningFlash(long count, long countInit) {
		bool warningFlashState = true;
		if (count > (countInit * 2l / 4l) && count < (countInit * 3l / 4l))
			warningFlashState = false;
		else if (count < (countInit * 1l / 4l))
			warningFlashState = false;
		return warningFlashState;
	}	
