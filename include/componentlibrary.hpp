#pragma once
#include "rack.hpp"


namespace rack {


////////////////////
// Color scheme
////////////////////

static const NVGcolor SCHEME_BLACK_TRANSPARENT = nvgRGBA(0x00, 0x00, 0x00, 0x00);
static const NVGcolor SCHEME_BLACK = nvgRGB(0x00, 0x00, 0x00);
static const NVGcolor SCHEME_WHITE = nvgRGB(0xff, 0xff, 0xff);
static const NVGcolor SCHEME_RED = nvgRGB(0xed, 0x2c, 0x24);
static const NVGcolor SCHEME_ORANGE = nvgRGB(0xf2, 0xb1, 0x20);
static const NVGcolor SCHEME_YELLOW = nvgRGB(0xf9, 0xdf, 0x1c);
static const NVGcolor SCHEME_GREEN = nvgRGB(0x90, 0xc7, 0x3e);
static const NVGcolor SCHEME_CYAN = nvgRGB(0x22, 0xe6, 0xef);
static const NVGcolor SCHEME_BLUE = nvgRGB(0x29, 0xb2, 0xef);
static const NVGcolor SCHEME_PURPLE = nvgRGB(0xd5, 0x2b, 0xed);
static const NVGcolor SCHEME_LIGHT_PANEL = nvgRGB(0xe6, 0xe6, 0xe6);
static const NVGcolor SCHEME_DARK_PANEL = nvgRGB(0x17, 0x17, 0x17);

////////////////////
// Knobs
////////////////////

struct RoundKnob : SVGKnob {
	RoundKnob() {
		minAngle = -0.83*M_PI;
		maxAngle = 0.83*M_PI;
	}
};

struct RoundBlackKnob : RoundKnob {
	RoundBlackKnob() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/RoundBlackKnob.svg")));
	}
};

struct RoundSmallBlackKnob : RoundKnob {
	RoundSmallBlackKnob() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/RoundSmallBlackKnob.svg")));
	}
};

struct RoundLargeBlackKnob : RoundKnob {
	RoundLargeBlackKnob() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/RoundLargeBlackKnob.svg")));
	}
};

struct RoundHugeBlackKnob : RoundKnob {
	RoundHugeBlackKnob() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/RoundHugeBlackKnob.svg")));
	}
};

struct RoundBlackSnapKnob : RoundBlackKnob {
	RoundBlackSnapKnob() {
		// TODO
		// quantity.snap = true;
		// quantity.smooth = false;
	}
};


struct Davies1900hKnob : SVGKnob {
	Davies1900hKnob() {
		minAngle = -0.83*M_PI;
		maxAngle = 0.83*M_PI;
	}
};

struct Davies1900hWhiteKnob : Davies1900hKnob {
	Davies1900hWhiteKnob() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Davies1900hWhite.svg")));
	}
};

struct Davies1900hBlackKnob : Davies1900hKnob {
	Davies1900hBlackKnob() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Davies1900hBlack.svg")));
	}
};

struct Davies1900hRedKnob : Davies1900hKnob {
	Davies1900hRedKnob() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Davies1900hRed.svg")));
	}
};

struct Davies1900hLargeWhiteKnob : Davies1900hKnob {
	Davies1900hLargeWhiteKnob() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Davies1900hLargeWhite.svg")));
	}
};

struct Davies1900hLargeBlackKnob : Davies1900hKnob {
	Davies1900hLargeBlackKnob() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Davies1900hLargeBlack.svg")));
	}
};

struct Davies1900hLargeRedKnob : Davies1900hKnob {
	Davies1900hLargeRedKnob() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Davies1900hLargeRed.svg")));
	}
};


struct Rogan : SVGKnob {
	Rogan() {
		minAngle = -0.83*M_PI;
		maxAngle = 0.83*M_PI;
	}
};

struct Rogan6PSWhite : Rogan {
	Rogan6PSWhite() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan6PSWhite.svg")));
	}
};

struct Rogan5PSGray : Rogan {
	Rogan5PSGray() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan5PSGray.svg")));
	}
};

struct Rogan3PSBlue : Rogan {
	Rogan3PSBlue() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan3PSBlue.svg")));
	}
};

struct Rogan3PSRed : Rogan {
	Rogan3PSRed() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan3PSRed.svg")));
	}
};

struct Rogan3PSGreen : Rogan {
	Rogan3PSGreen() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan3PSGreen.svg")));
	}
};

struct Rogan3PSWhite : Rogan {
	Rogan3PSWhite() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan3PSWhite.svg")));
	}
};

struct Rogan3PBlue : Rogan {
	Rogan3PBlue() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan3PBlue.svg")));
	}
};

struct Rogan3PRed : Rogan {
	Rogan3PRed() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan3PRed.svg")));
	}
};

struct Rogan3PGreen : Rogan {
	Rogan3PGreen() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan3PGreen.svg")));
	}
};

struct Rogan3PWhite : Rogan {
	Rogan3PWhite() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan3PWhite.svg")));
	}
};

struct Rogan2SGray : Rogan {
	Rogan2SGray() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan2SGray.svg")));
	}
};

struct Rogan2PSBlue : Rogan {
	Rogan2PSBlue() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan2PSBlue.svg")));
	}
};

struct Rogan2PSRed : Rogan {
	Rogan2PSRed() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan2PSRed.svg")));
	}
};

struct Rogan2PSGreen : Rogan {
	Rogan2PSGreen() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan2PSGreen.svg")));
	}
};

struct Rogan2PSWhite : Rogan {
	Rogan2PSWhite() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan2PSWhite.svg")));
	}
};

struct Rogan2PBlue : Rogan {
	Rogan2PBlue() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan2PBlue.svg")));
	}
};

struct Rogan2PRed : Rogan {
	Rogan2PRed() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan2PRed.svg")));
	}
};

struct Rogan2PGreen : Rogan {
	Rogan2PGreen() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan2PGreen.svg")));
	}
};

struct Rogan2PWhite : Rogan {
	Rogan2PWhite() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan2PWhite.svg")));
	}
};

struct Rogan1PSBlue : Rogan {
	Rogan1PSBlue() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan1PSBlue.svg")));
	}
};

struct Rogan1PSRed : Rogan {
	Rogan1PSRed() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan1PSRed.svg")));
	}
};

struct Rogan1PSGreen : Rogan {
	Rogan1PSGreen() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan1PSGreen.svg")));
	}
};

struct Rogan1PSWhite : Rogan {
	Rogan1PSWhite() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan1PSWhite.svg")));
	}
};

struct Rogan1PBlue : Rogan {
	Rogan1PBlue() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan1PBlue.svg")));
	}
};

struct Rogan1PRed : Rogan {
	Rogan1PRed() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan1PRed.svg")));
	}
};

struct Rogan1PGreen : Rogan {
	Rogan1PGreen() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan1PGreen.svg")));
	}
};

struct Rogan1PWhite : Rogan {
	Rogan1PWhite() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Rogan1PWhite.svg")));
	}
};


struct SynthTechAlco : SVGKnob {
	SynthTechAlco() {
		minAngle = -0.82*M_PI;
		maxAngle = 0.82*M_PI;
		setSVG(SVG::load(asset::global("res/ComponentLibrary/SynthTechAlco.svg")));
		SVGWidget *cap = new SVGWidget;
		cap->setSVG(SVG::load(asset::global("res/ComponentLibrary/SynthTechAlco_cap.svg")));
		addChild(cap);
	}
};

struct Trimpot : SVGKnob {
	Trimpot() {
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(asset::global("res/ComponentLibrary/Trimpot.svg")));
	}
};

struct BefacoBigKnob : SVGKnob {
	BefacoBigKnob() {
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(asset::global("res/ComponentLibrary/BefacoBigKnob.svg")));
	}
};

struct BefacoBigSnapKnob : BefacoBigKnob {
	BefacoBigSnapKnob() {
		// TODO
		// quantity.snap = true;
		// quantity.smooth = false;
	}
};

struct BefacoTinyKnob : SVGKnob {
	BefacoTinyKnob() {
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(asset::global("res/ComponentLibrary/BefacoTinyKnob.svg")));
	}
};

struct BefacoSlidePot : SVGSlider {
	BefacoSlidePot() {
		Vec margin = Vec(3.5, 3.5);
		maxHandlePos = Vec(-1, -2).plus(margin);
		minHandlePos = Vec(-1, 87).plus(margin);
		setSVGs(SVG::load(asset::global("res/ComponentLibrary/BefacoSlidePot.svg")), SVG::load(asset::global("res/ComponentLibrary/BefacoSlidePotHandle.svg")));
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));
	}
};

struct LEDSlider : SVGSlider {
	LEDSlider() {
		maxHandlePos = mm2px(Vec(0.738, 0.738).plus(Vec(2, 0)));
		minHandlePos = mm2px(Vec(0.738, 22.078).plus(Vec(2, 0)));
		setSVGs(SVG::load(asset::global("res/ComponentLibrary/LEDSlider.svg")), NULL);
	}
};

/** API is unstable for LEDSlider. Will add a LightWidget later. */
struct LEDSliderGreen : LEDSlider {
	LEDSliderGreen() {
		handle->setSVG(SVG::load(asset::global("res/ComponentLibrary/LEDSliderGreenHandle.svg")));
	}
};

struct LEDSliderRed : LEDSlider {
	LEDSliderRed() {
		handle->setSVG(SVG::load(asset::global("res/ComponentLibrary/LEDSliderRedHandle.svg")));
	}
};

struct LEDSliderYellow : LEDSlider {
	LEDSliderYellow() {
		handle->setSVG(SVG::load(asset::global("res/ComponentLibrary/LEDSliderYellowHandle.svg")));
	}
};

struct LEDSliderBlue : LEDSlider {
	LEDSliderBlue() {
		handle->setSVG(SVG::load(asset::global("res/ComponentLibrary/LEDSliderBlueHandle.svg")));
	}
};

struct LEDSliderWhite : LEDSlider {
	LEDSliderWhite() {
		handle->setSVG(SVG::load(asset::global("res/ComponentLibrary/LEDSliderWhiteHandle.svg")));
	}
};

////////////////////
// Ports
////////////////////

struct PJ301MPort : SVGPort {
	PJ301MPort() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/PJ301M.svg")));
	}
};

struct PJ3410Port : SVGPort {
	PJ3410Port() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/PJ3410.svg")));
	}
};

struct CL1362Port : SVGPort {
	CL1362Port() {
		setSVG(SVG::load(asset::global("res/ComponentLibrary/CL1362.svg")));
	}
};

////////////////////
// Lights
////////////////////

struct GrayModuleLightWidget : ModuleLightWidget {
	GrayModuleLightWidget() {
		bgColor = nvgRGB(0x5a, 0x5a, 0x5a);
		borderColor = nvgRGBA(0, 0, 0, 0x60);
	}
};

struct RedLight : GrayModuleLightWidget {
	RedLight() {
		addBaseColor(SCHEME_RED);
	}
};

struct GreenLight : GrayModuleLightWidget {
	GreenLight() {
		addBaseColor(SCHEME_GREEN);
	}
};

struct YellowLight : GrayModuleLightWidget {
	YellowLight() {
		addBaseColor(SCHEME_YELLOW);
	}
};

struct BlueLight : GrayModuleLightWidget {
	BlueLight() {
		addBaseColor(SCHEME_BLUE);
	}
};

/** Reads two adjacent lightIds, so `lightId` and `lightId + 1` must be defined */
struct GreenRedLight : GrayModuleLightWidget {
	GreenRedLight() {
		addBaseColor(SCHEME_GREEN);
		addBaseColor(SCHEME_RED);
	}
};

struct RedGreenBlueLight : GrayModuleLightWidget {
	RedGreenBlueLight() {
		addBaseColor(SCHEME_RED);
		addBaseColor(SCHEME_GREEN);
		addBaseColor(SCHEME_BLUE);
	}
};

struct RGBLight : ModuleLightWidget {
	RGBLight() {
		addBaseColor(nvgRGBf(1, 0, 0));
		addBaseColor(nvgRGBf(0, 1, 0));
		addBaseColor(nvgRGBf(0, 0, 1));
	}
};


/** Based on the size of 5mm LEDs */
template <typename BASE>
struct LargeLight : BASE {
	LargeLight() {
		this->box.size = mm2px(Vec(5.179, 5.179));
	}
};

/** Based on the size of 3mm LEDs */
template <typename BASE>
struct MediumLight : BASE {
	MediumLight() {
		this->box.size = mm2px(Vec(3.176, 3.176));
	}
};

/** Based on the size of 2mm LEDs */
template <typename BASE>
struct SmallLight : BASE {
	SmallLight() {
		this->box.size = mm2px(Vec(2.176, 2.176));
	}
};

/** Based on the size of 1mm LEDs */
template <typename BASE>
struct TinyLight : BASE {
	TinyLight() {
		this->box.size = mm2px(Vec(1.088, 1.088));
	}
};

/** A light to displayed over PB61303. Must add a color by subclassing or templating. */
template <typename BASE>
struct LEDBezelLight : BASE {
	LEDBezelLight() {
		this->bgColor = color::BLACK_TRANSPARENT;
		this->box.size = mm2px(Vec(6.0, 6.0));
	}
};

/** A light to displayed over PB61303. Must add a color by subclassing or templating.
Don't add this as a child of the PB61303 itself. Instead, just place it over it as a sibling in the scene graph, offset by mm2px(Vec(0.5, 0.5)).
*/
template <typename BASE>
struct PB61303Light : BASE {
	PB61303Light() {
		this->bgColor = color::BLACK_TRANSPARENT;
		this->box.size = mm2px(Vec(9.0, 9.0));
	}
};


////////////////////
// Switches and Buttons
////////////////////

struct NKK : SVGSwitch, ToggleSwitch {
	NKK() {
		addFrame(SVG::load(asset::global("res/ComponentLibrary/NKK_0.svg")));
		addFrame(SVG::load(asset::global("res/ComponentLibrary/NKK_1.svg")));
		addFrame(SVG::load(asset::global("res/ComponentLibrary/NKK_2.svg")));
	}
};

struct CKSS : SVGSwitch, ToggleSwitch {
	CKSS() {
		addFrame(SVG::load(asset::global("res/ComponentLibrary/CKSS_0.svg")));
		addFrame(SVG::load(asset::global("res/ComponentLibrary/CKSS_1.svg")));
	}
};

struct CKSSThree : SVGSwitch, ToggleSwitch {
	CKSSThree() {
		addFrame(SVG::load(asset::global("res/ComponentLibrary/CKSSThree_0.svg")));
		addFrame(SVG::load(asset::global("res/ComponentLibrary/CKSSThree_1.svg")));
		addFrame(SVG::load(asset::global("res/ComponentLibrary/CKSSThree_2.svg")));
	}
};

struct CKD6 : SVGSwitch, MomentarySwitch {
	CKD6() {
		addFrame(SVG::load(asset::global("res/ComponentLibrary/CKD6_0.svg")));
		addFrame(SVG::load(asset::global("res/ComponentLibrary/CKD6_1.svg")));
	}
};

struct TL1105 : SVGSwitch, MomentarySwitch {
	TL1105() {
		addFrame(SVG::load(asset::global("res/ComponentLibrary/TL1105_0.svg")));
		addFrame(SVG::load(asset::global("res/ComponentLibrary/TL1105_1.svg")));
	}
};

struct LEDButton : SVGSwitch, MomentarySwitch {
	LEDButton() {
		addFrame(SVG::load(asset::global("res/ComponentLibrary/LEDButton.svg")));
	}
};

struct BefacoSwitch : SVGSwitch, ToggleSwitch {
	BefacoSwitch() {
		addFrame(SVG::load(asset::global("res/ComponentLibrary/BefacoSwitch_0.svg")));
		addFrame(SVG::load(asset::global("res/ComponentLibrary/BefacoSwitch_1.svg")));
		addFrame(SVG::load(asset::global("res/ComponentLibrary/BefacoSwitch_2.svg")));
	}
};

struct BefacoPush : SVGSwitch, MomentarySwitch {
	BefacoPush() {
		addFrame(SVG::load(asset::global("res/ComponentLibrary/BefacoPush_0.svg")));
		addFrame(SVG::load(asset::global("res/ComponentLibrary/BefacoPush_1.svg")));
	}
};

struct LEDBezel : SVGSwitch, MomentarySwitch {
	LEDBezel() {
		addFrame(SVG::load(asset::global("res/ComponentLibrary/LEDBezel.svg")));
	}
};

struct PB61303 : SVGSwitch, MomentarySwitch {
	PB61303() {
		addFrame(SVG::load(asset::global("res/ComponentLibrary/PB61303.svg")));
	}
};

struct PB61303Button : SVGButton {
	PB61303Button() {
		setSVGs(SVG::load(asset::global("res/ComponentLibrary/PB61303.svg")), NULL);
	}
};

struct LEDBezelButton : SVGButton {
	LEDBezelButton() {
		setSVGs(SVG::load(asset::global("res/ComponentLibrary/LEDBezel.svg")), NULL);
	}
};

////////////////////
// Misc
////////////////////

struct ScrewSilver : SVGScrew {
	ScrewSilver() {
		sw->setSVG(SVG::load(asset::global("res/ComponentLibrary/ScrewSilver.svg")));
		box.size = sw->box.size;
	}
};

struct ScrewBlack : SVGScrew {
	ScrewBlack() {
		sw->setSVG(SVG::load(asset::global("res/ComponentLibrary/ScrewBlack.svg")));
		box.size = sw->box.size;
	}
};


} // namespace rack
