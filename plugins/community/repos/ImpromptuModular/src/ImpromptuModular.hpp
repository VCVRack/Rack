//***********************************************************************************************
//Impromptu Modular: Modules for VCV Rack by Marc Boulé
//
//Based on code from the Fundamental and AudibleInstruments plugins by Andrew Belt 
//and graphics from the Component Library by Wes Milholen 
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//***********************************************************************************************

#ifndef IMPROMPU_MODULAR_HPP
#define IMPROMPU_MODULAR_HPP


#include "rack.hpp"
#include "IMWidgets.hpp"
#include "dsp/digital.hpp"

using namespace rack;

#define plugin "ImpromptuModular"

// General constants
static const bool clockIgnoreOnRun = false;
static const bool retrigGatesOnReset = true;
static constexpr float clockIgnoreOnResetDuration = 0.001f;// disable clock on powerup and reset for 1 ms (so that the first step plays)
static const float lightLambda = 0.075f;
static const int displayAlpha = 23;
static const std::string lightPanelID = "Classic";
static const std::string darkPanelID = "Dark-valor";
static const std::string expansionMenuLabel = "Extra CVs (requires +4HP to the right!)";// note: PS32EX detects '4' and replaces it with '7'
static const unsigned int displayRefreshStepSkips = 256;
static const unsigned int userInputsStepSkipMask = 0xF;// sub interval of displayRefreshStepSkips, since inputs should be more responsive than lights
// above value should make it such that inputs are sampled > 1kHz so as to not miss 1ms triggers


// Component offset constants

static const int hOffsetCKSS = 5;
static const int vOffsetCKSS = 2;
static const int vOffsetCKSSThree = -2;
static const int hOffsetCKSSH = 2;
static const int vOffsetCKSSH = 5;
static const int offsetCKD6 = -1;//does both h and v
static const int offsetCKD6b = 0;//does both h and v
static const int vOffsetDisplay = -2;
static const int offsetIMBigKnob = -6;//does both h and v
static const int offsetIMSmallKnob = 0;//does both h and v
static const int offsetMediumLight = 9;
static const float offsetLEDbutton = 3.0f;//does both h and v
static const float offsetLEDbuttonLight = 4.4f;//does both h and v
static const int offsetTL1105 = 4;//does both h and v
static const int offsetLEDbezel = 1;//does both h and v
static const float offsetLEDbezelLight = 2.2f;//does both h and v
static const float offsetLEDbezelBig = -11;//does both h and v
static const int offsetTrimpot = 3;//does both h and v



// Variations on existing knobs, lights, etc


// Screws

struct IMScrew : DynamicSVGScrew {
	IMScrew() {
		addSVGalt(SVG::load(assetPlugin(plugin, "res/dark/comp/ScrewSilver.svg")));
	}
};


// Ports

struct IMPort : DynamicSVGPort {
	IMPort() {
		//addFrame(SVG::load(assetGlobal("res/ComponentLibrary/PJ301M.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/light/comp/PJ301M.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/dark/comp/PJ301M.svg")));
		shadow->blurRadius = 10.0;
		shadow->opacity = 0.8;
	}
};


// Buttons and switches

struct CKSSNoRandom : CKSS {
	CKSSNoRandom() {}
	void randomize() override {}
};

struct CKSSH : SVGSwitch, ToggleSwitch {
	CKSSH() {
		addFrame(SVG::load(assetPlugin(plugin, "res/comp/CKSSH_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/comp/CKSSH_1.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct CKSSHNoRandom : CKSSH {
	CKSSHNoRandom() {}
	void randomize() override {}
};

struct CKSSHThree : SVGSwitch, ToggleSwitch {
	CKSSHThree() {
		addFrame(SVG::load(assetPlugin(plugin, "res/comp/CKSSHThree_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/comp/CKSSHThree_1.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/comp/CKSSHThree_2.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct CKSSThreeInv : SVGSwitch, ToggleSwitch {
	CKSSThreeInv() {
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/CKSSThree_2.svg")));
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/CKSSThree_1.svg")));
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/CKSSThree_0.svg")));
	}
};

struct CKSSThreeInvNoRandom : CKSSThreeInv {
	CKSSThreeInvNoRandom() {}
	void randomize() override {}
};

struct IMBigPushButton : DynamicSVGSwitch, MomentarySwitch {
	IMBigPushButton() {
		addFrameAll(SVG::load(assetPlugin(plugin, "res/light/comp/CKD6b_0.svg")));
		addFrameAll(SVG::load(assetPlugin(plugin, "res/light/comp/CKD6b_1.svg")));
		addFrameAll(SVG::load(assetPlugin(plugin, "res/dark/comp/CKD6b_0.svg")));
		addFrameAll(SVG::load(assetPlugin(plugin, "res/dark/comp/CKD6b_1.svg")));	
	}
};

struct IMBigPushButtonWithRClick : IMBigPushButton {// with right click that sets value to 2.0 instead of 1.0
	void onMouseDown(EventMouseDown &e) override;
	void onMouseUp(EventMouseUp &e) override;
};

struct IMPushButton : DynamicSVGSwitch, MomentarySwitch {
	IMPushButton() {
		addFrameAll(SVG::load(assetPlugin(plugin, "res/light/comp/TL1105_0.svg")));
		addFrameAll(SVG::load(assetPlugin(plugin, "res/light/comp/TL1105_1.svg")));
		addFrameAll(SVG::load(assetPlugin(plugin, "res/dark/comp/TL1105_0.svg")));
		addFrameAll(SVG::load(assetPlugin(plugin, "res/dark/comp/TL1105_1.svg")));	
	}
};

struct LEDBezelBig : SVGSwitch, MomentarySwitch {
	TransformWidget *tw;
	LEDBezelBig();
};


// Knobs

struct IMKnob : DynamicSVGKnob {
	IMKnob() {
		minAngle = -0.83*M_PI;
		maxAngle = 0.83*M_PI;
		shadow->blurRadius = 10.0;
		shadow->opacity = 0.8;
	}
};

struct IMBigKnob : IMKnob {
	IMBigKnob() {
		addFrameAll(SVG::load(assetPlugin(plugin, "res/light/comp/BlackKnobLargeWithMark.svg")));
		addFrameAll(SVG::load(assetPlugin(plugin, "res/dark/comp/BlackKnobLargeWithMark.svg")));
		addEffect(SVG::load(assetPlugin(plugin, "res/dark/comp/BlackKnobLargeWithMarkEffects.svg")));
	}
};
struct IMBigSnapKnob : IMBigKnob {
	IMBigSnapKnob() {
		snap = true;
		smooth = false;
	}
	void randomize() override {}
};

struct IMBigKnobInf : IMKnob {
	IMBigKnobInf() {
		addFrameAll(SVG::load(assetPlugin(plugin, "res/light/comp/BlackKnobLarge.svg")));
		addFrameAll(SVG::load(assetPlugin(plugin, "res/dark/comp/BlackKnobLarge.svg")));
		addEffect(SVG::load(assetPlugin(plugin, "res/dark/comp/BlackKnobLargeEffects.svg")));
		speed = 0.9f;				
		//smooth = false;
	}
};

struct IMSmallKnob : IMKnob {
	IMSmallKnob() {
		addFrameAll(SVG::load(assetPlugin(plugin, "res/light/comp/RoundSmallBlackKnob.svg")));
		addFrameAll(SVG::load(assetPlugin(plugin, "res/dark/comp/RoundSmallBlackKnob.svg")));
		addEffect(SVG::load(assetPlugin(plugin, "res/dark/comp/RoundSmallBlackKnobEffects.svg")));		
		shadow->box.pos = Vec(0.0, box.size.y * 0.15);
	}
};

struct IMSmallSnapKnob : IMSmallKnob {
	IMSmallSnapKnob() {
		snap = true;
		smooth = false;
	}
};

struct IMMediumKnobInf : IMKnob {
	IMMediumKnobInf() {
		addFrameAll(SVG::load(assetPlugin(plugin, "res/light/comp/RoundMediumBlackKnobNoMark.svg")));
		addFrameAll(SVG::load(assetPlugin(plugin, "res/dark/comp/RoundMediumBlackKnobNoMark.svg")));
		addEffect(SVG::load(assetPlugin(plugin, "res/dark/comp/RoundMediumBlackKnobNoMarkEffects.svg")));
		shadow->box.pos = Vec(0.0, box.size.y * 0.15);
		speed = 0.9f;				
		//smooth = false;
	}
};

struct IMFivePosSmallKnob : IMSmallSnapKnob {
	IMFivePosSmallKnob() {
		minAngle = -0.5*M_PI;
		maxAngle = 0.5*M_PI;
	}
};

struct IMSixPosBigKnob : IMBigSnapKnob {
	IMSixPosBigKnob() {
		minAngle = -0.4*M_PI;
		maxAngle = 0.4*M_PI;
	}
	void randomize() override {}
};

struct IMTactile : DynamicIMTactile {
	IMTactile() {
		smooth = false;// must be false or else DynamicIMTactile::changeValue() call from module will crash Rack
	}
};



// Lights

struct OrangeLight : GrayModuleLightWidget {
	OrangeLight() {
		addBaseColor(COLOR_ORANGE);
	}
};
struct GreenRedWhiteLight : GrayModuleLightWidget {
	GreenRedWhiteLight() {
		addBaseColor(COLOR_GREEN);
		addBaseColor(COLOR_RED);
		addBaseColor(COLOR_WHITE);
	}
};

template <typename BASE>
struct MuteLight : BASE {
	MuteLight() {
		this->box.size = mm2px(Vec(6.0f, 6.0f));
	}
};


template <typename BASE>
struct GiantLight : BASE {
	GiantLight() {
		this->box.size = mm2px(Vec(19.0f, 19.0f));
	}
};
template <typename BASE>
struct GiantLight2 : BASE {
	GiantLight2() {
		this->box.size = mm2px(Vec(12.8f, 12.8f));
	}
};

// Other widgets

struct InvisibleKey : MomentarySwitch {
	InvisibleKey() {
		box.size = Vec(34, 72);
	}
};

struct InvisibleKeySmall : MomentarySwitch {
	InvisibleKeySmall() {
		box.size = Vec(23, 38);
	}
	void onMouseDown(EventMouseDown &e) override;
	void onMouseUp(EventMouseUp &e) override;
};

struct LEDButtonWithRClick : LEDButton {// with right click that sets value to 2.0 instead of 1.0
	void onMouseDown(EventMouseDown &e) override;
	void onMouseUp(EventMouseUp &e) override;
};


struct ScrewSilverRandomRot : FramebufferWidget {// location: include/app.hpp and src/app/SVGScrew.cpp [some code also from src/app/SVGKnob.cpp]
	SVGWidget *sw;
	TransformWidget *tw;
	ScrewCircle *sc;
	ScrewSilverRandomRot();
};

struct ScrewHole : TransparentWidget {
	ScrewHole(Vec posGiven);
	void draw(NVGcontext *vg) override;
};	



// Other

struct Trigger : SchmittTrigger {
	// implements a 0.1V - 1.0V SchmittTrigger (include/dsp/digital.hpp) instead of 
	//   calling SchmittTriggerInstance.process(math::rescale(in, 0.1f, 1.f, 0.f, 1.f))
	bool process(float in) {
		switch (state) {
			case LOW:
				if (in >= 1.0f) {
					state = HIGH;
					return true;
				}
				break;
			case HIGH:
				if (in <= 0.1f) {
					state = LOW;
				}
				break;
			default:
				if (in >= 1.0f) {
					state = HIGH;
				}
				else if (in <= 0.1f) {
					state = LOW;
				}
				break;
		}
		return false;
	}	
};	

struct HoldDetect {
	long modeHoldDetect;// 0 when not detecting, downward counter when detecting
	
	void reset() {
		modeHoldDetect = 0l;
	}
	
	void start(long startValue) {
		modeHoldDetect = startValue;
	}

	bool process(float paramValue) {
		bool ret = false;
		if (modeHoldDetect > 0l) {
			if (paramValue < 0.5f)
				modeHoldDetect = 0l;
			else {
				if (modeHoldDetect == 1l) {
					ret = true;
				}
				modeHoldDetect--;
			}
		}
		return ret;
	}
};

inline bool calcWarningFlash(long count, long countInit) {
	if ( (count > (countInit * 2l / 4l) && count < (countInit * 3l / 4l)) || (count < (countInit * 1l / 4l)) )
		return false;
	return true;
}	



NVGcolor prepareDisplay(NVGcontext *vg, Rect *box, int fontSize);
void printNote(float cvVal, char* text, bool sharp);
int moveIndex(int index, int indexNext, int numSteps);


#endif
