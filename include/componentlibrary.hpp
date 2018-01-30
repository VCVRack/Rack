#pragma once
#include "app.hpp"
#include "asset.hpp"


namespace rack {


////////////////////
// Colors
////////////////////

#define COLOR_BLACK_TRANSPARENT nvgRGBA(0x00, 0x00, 0x00, 0x00)
#define COLOR_BLACK nvgRGB(0x00, 0x00, 0x00)
#define COLOR_WHITE nvgRGB(0xff, 0xff, 0xff)
#define COLOR_RED nvgRGB(0xed, 0x2c, 0x24)
#define COLOR_ORANGE nvgRGB(0xf2, 0xb1, 0x20)
#define COLOR_YELLOW nvgRGB(0xf9, 0xdf, 0x1c)
#define COLOR_GREEN nvgRGB(0x90, 0xc7, 0x3e)
#define COLOR_CYAN nvgRGB(0x22, 0xe6, 0xef)
#define COLOR_BLUE nvgRGB(0x29, 0xb2, 0xef)
#define COLOR_PURPLE nvgRGB(0xd5, 0x2b, 0xed)

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
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/RoundBlack.svg")));
		box.size = Vec(38, 38);
	}
};

struct RoundSmallBlackKnob : RoundBlackKnob {
	RoundSmallBlackKnob() {
		box.size = Vec(28, 28);
	}
};

struct RoundLargeBlackKnob : RoundBlackKnob {
	RoundLargeBlackKnob() {
		box.size = Vec(46, 46);
	}
};

struct RoundHugeBlackKnob : RoundBlackKnob {
	RoundHugeBlackKnob() {
		box.size = Vec(56, 56);
	}
};

struct RoundSmallBlackSnapKnob : RoundSmallBlackKnob {
	RoundSmallBlackSnapKnob() {
		snap = true;
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
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Davies1900hWhite.svg")));
	}
};

struct Davies1900hBlackKnob : Davies1900hKnob {
	Davies1900hBlackKnob() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Davies1900hBlack.svg")));
	}
};

struct Davies1900hRedKnob : Davies1900hKnob {
	Davies1900hRedKnob() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Davies1900hRed.svg")));
	}
};

struct Davies1900hLargeWhiteKnob : Davies1900hKnob {
	Davies1900hLargeWhiteKnob() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Davies1900hLargeWhite.svg")));
	}
};

struct Davies1900hLargeBlackKnob : Davies1900hKnob {
	Davies1900hLargeBlackKnob() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Davies1900hLargeBlack.svg")));
	}
};

struct Davies1900hLargeRedKnob : Davies1900hKnob {
	Davies1900hLargeRedKnob() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Davies1900hLargeRed.svg")));
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
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan6PSWhite.svg")));
	}
};

struct Rogan5PSGray : Rogan {
	Rogan5PSGray() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan5PSGray.svg")));
	}
};

struct Rogan3PSBlue : Rogan {
	Rogan3PSBlue() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan3PSBlue.svg")));
	}
};

struct Rogan3PSRed : Rogan {
	Rogan3PSRed() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan3PSRed.svg")));
	}
};

struct Rogan3PSGreen : Rogan {
	Rogan3PSGreen() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan3PSGreen.svg")));
	}
};

struct Rogan3PSWhite : Rogan {
	Rogan3PSWhite() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan3PSWhite.svg")));
	}
};

struct Rogan3PBlue : Rogan {
	Rogan3PBlue() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan3PBlue.svg")));
	}
};

struct Rogan3PRed : Rogan {
	Rogan3PRed() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan3PRed.svg")));
	}
};

struct Rogan3PGreen : Rogan {
	Rogan3PGreen() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan3PGreen.svg")));
	}
};

struct Rogan3PWhite : Rogan {
	Rogan3PWhite() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan3PWhite.svg")));
	}
};

struct Rogan2SGray : Rogan {
	Rogan2SGray() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan2SGray.svg")));
	}
};

struct Rogan2PSBlue : Rogan {
	Rogan2PSBlue() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan2PSBlue.svg")));
	}
};

struct Rogan2PSRed : Rogan {
	Rogan2PSRed() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan2PSRed.svg")));
	}
};

struct Rogan2PSGreen : Rogan {
	Rogan2PSGreen() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan2PSGreen.svg")));
	}
};

struct Rogan2PSWhite : Rogan {
	Rogan2PSWhite() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan2PSWhite.svg")));
	}
};

struct Rogan2PBlue : Rogan {
	Rogan2PBlue() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan2PBlue.svg")));
	}
};

struct Rogan2PRed : Rogan {
	Rogan2PRed() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan2PRed.svg")));
	}
};

struct Rogan2PGreen : Rogan {
	Rogan2PGreen() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan2PGreen.svg")));
	}
};

struct Rogan2PWhite : Rogan {
	Rogan2PWhite() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan2PWhite.svg")));
	}
};

struct Rogan1PSBlue : Rogan {
	Rogan1PSBlue() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan1PSBlue.svg")));
	}
};

struct Rogan1PSRed : Rogan {
	Rogan1PSRed() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan1PSRed.svg")));
	}
};

struct Rogan1PSGreen : Rogan {
	Rogan1PSGreen() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan1PSGreen.svg")));
	}
};

struct Rogan1PSWhite : Rogan {
	Rogan1PSWhite() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan1PSWhite.svg")));
	}
};

struct Rogan1PBlue : Rogan {
	Rogan1PBlue() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan1PBlue.svg")));
	}
};

struct Rogan1PRed : Rogan {
	Rogan1PRed() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan1PRed.svg")));
	}
};

struct Rogan1PGreen : Rogan {
	Rogan1PGreen() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan1PGreen.svg")));
	}
};

struct Rogan1PWhite : Rogan {
	Rogan1PWhite() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan1PWhite.svg")));
	}
};


struct SynthTechAlco : SVGKnob {
	SynthTechAlco() {
		minAngle = -0.82*M_PI;
		maxAngle = 0.82*M_PI;
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/SynthTechAlco.svg")));
		SVGWidget *cap = new SVGWidget();
		cap->setSVG(SVG::load(assetGlobal("res/ComponentLibrary/SynthTechAlco_cap.svg")));
		addChild(cap);
	}
};

struct Trimpot : SVGKnob {
	Trimpot() {
		box.size = Vec(17, 17);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Trimpot.svg")));
	}
};

struct BefacoBigKnob : SVGKnob {
	BefacoBigKnob() {
		box.size = Vec(75, 75);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/BefacoBigKnob.svg")));
	}
};

struct BefacoBigSnapKnob : BefacoBigKnob {
	BefacoBigSnapKnob() {
		snap = true;
	}
};

struct BefacoTinyKnob : SVGKnob {
	BefacoTinyKnob() {
		box.size = Vec(26, 26);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/BefacoTinyKnob.svg")));
	}
};

struct BefacoSlidePot : SVGFader {
	BefacoSlidePot() {
		Vec margin = Vec(3.5, 3.5);
		maxHandlePos = Vec(-1, -2).plus(margin);
		minHandlePos = Vec(-1, 87).plus(margin);
		background->svg = SVG::load(assetGlobal("res/ComponentLibrary/BefacoSlidePot.svg"));
		background->wrap();
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));
		handle->svg = SVG::load(assetGlobal("res/ComponentLibrary/BefacoSlidePotHandle.svg"));
		handle->wrap();
	}
};

////////////////////
// IO widgets
////////////////////

struct USB_B_AudioWidget : AudioWidget, SVGWidget {
	USB_B_AudioWidget() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/USB-B.svg")));
	}
};

struct MIDI_DIN_MidiWidget : MidiWidget, SVGWidget {
	MIDI_DIN_MidiWidget() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/MIDI_DIN.svg")));
	}
};

////////////////////
// Jacks
////////////////////

struct PJ301MPort : SVGPort {
	PJ301MPort() {
		background->svg = SVG::load(assetGlobal("res/ComponentLibrary/PJ301M.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct PJ3410Port : SVGPort {
	PJ3410Port() {
		background->svg = SVG::load(assetGlobal("res/ComponentLibrary/PJ3410.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct CL1362Port : SVGPort {
	CL1362Port() {
		background->svg = SVG::load(assetGlobal("res/ComponentLibrary/CL1362.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

////////////////////
// Lights
////////////////////

struct RedLight : ModuleLightWidget {
	RedLight() {
		addBaseColor(COLOR_RED);
	}
};

struct GreenLight : ModuleLightWidget {
	GreenLight() {
		addBaseColor(COLOR_GREEN);
	}
};

struct YellowLight : ModuleLightWidget {
	YellowLight() {
		addBaseColor(COLOR_YELLOW);
	}
};

struct BlueLight : ModuleLightWidget {
	BlueLight() {
		addBaseColor(COLOR_BLUE);
	}
};

/** Reads two adjacent lightIds, so `lightId` and `lightId + 1` must be defined */
struct GreenRedLight : ModuleLightWidget {
	GreenRedLight() {
		addBaseColor(COLOR_GREEN);
		addBaseColor(COLOR_RED);
	}
};

struct RedGreenBlueLight : ModuleLightWidget {
	RedGreenBlueLight() {
		addBaseColor(COLOR_RED);
		addBaseColor(COLOR_GREEN);
		addBaseColor(COLOR_BLUE);
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


////////////////////
// Switches and Buttons
////////////////////

struct NKK : SVGSwitch, ToggleSwitch {
	NKK() {
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/NKK_0.svg")));
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/NKK_1.svg")));
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/NKK_2.svg")));
	}
};

struct CKSS : SVGSwitch, ToggleSwitch {
	CKSS() {
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/CKSS_0.svg")));
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/CKSS_1.svg")));
	}
};

struct CKSSThree : SVGSwitch, ToggleSwitch {
	CKSSThree() {
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/CKSSThree_0.svg")));
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/CKSSThree_1.svg")));
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/CKSSThree_2.svg")));
	}
};

struct CKD6 : SVGSwitch, MomentarySwitch {
	CKD6() {
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/CKD6_0.svg")));
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/CKD6_1.svg")));
	}
};

struct TL1105 : SVGSwitch, MomentarySwitch {
	TL1105() {
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/TL1105_0.svg")));
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/TL1105_1.svg")));
	}
};

struct LEDButton : SVGSwitch, MomentarySwitch {
	LEDButton() {
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/LEDButton.svg")));
	}
};

struct BefacoSwitch : SVGSwitch, ToggleSwitch {
	BefacoSwitch() {
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/BefacoSwitch_0.svg")));
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/BefacoSwitch_1.svg")));
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/BefacoSwitch_2.svg")));
	}
};

struct BefacoPush : SVGSwitch, MomentarySwitch {
	BefacoPush() {
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/BefacoPush_0.svg")));
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/BefacoPush_1.svg")));
	}
};

struct PB61303 : SVGSwitch, MomentarySwitch {
	PB61303() {
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/PB61303.svg")));
	}
};

struct LEDBezel : SVGSwitch, MomentarySwitch {
	LEDBezel() {
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/LEDBezel.svg")));
	}
};


////////////////////
// Misc
////////////////////

struct ScrewSilver : SVGScrew {
	ScrewSilver() {
		sw->setSVG(SVG::load(assetGlobal("res/ComponentLibrary/ScrewSilver.svg")));
		box.size = sw->box.size;
	}
};

struct ScrewBlack : SVGScrew {
	ScrewBlack() {
		sw->setSVG(SVG::load(assetGlobal("res/ComponentLibrary/ScrewBlack.svg")));
		box.size = sw->box.size;
	}
};

struct LightPanel : Panel {
	LightPanel() {
		backgroundColor = nvgRGB(0xe6, 0xe6, 0xe6);
	}
};

struct DarkPanel : Panel {
	DarkPanel() {
		backgroundColor = nvgRGB(0x17, 0x17, 0x17);
	}
};


} // namespace rack
