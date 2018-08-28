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
   RACK_PLUGIN_INIT_VERSION("0.6.11");

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

bool calcWarningFlash(long count, long countInit) {
	bool warningFlashState = true;
	if (count > (countInit * 2l / 4l) && count < (countInit * 3l / 4l))
		warningFlashState = false;
	else if (count < (countInit * 1l / 4l))
		warningFlashState = false;
	return warningFlashState;
}	

