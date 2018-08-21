#include "rack.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(PvC);
#ifdef USE_VST2
#define plugin "PvC"
#endif // USE_VST2

//////////////////////////////////////////////////////////////////////////////

namespace rack_plugin_PvC {

struct OutPortBin : SVGPort {
	OutPortBin() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/OutPortBin.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};
struct OutPortVal : SVGPort {
	OutPortVal() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/OutPortVal.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};
struct InPortBin : SVGPort {
	InPortBin() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/InPortBin.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};
struct InPortAud : SVGPort {
	InPortAud() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/InPortAud.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};
struct InPortCtrl : SVGPort {
	InPortCtrl() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/InPortCtrl.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

/// knobs & buttons

struct PvCKnob : RoundKnob {
	PvCKnob() {
		setSVG(SVG::load(assetPlugin(plugin, "res/components/PvCKnob.svg")));
		box.size = Vec(22,22);
	}
};
struct PvCLEDKnob : PvCKnob {
	PvCLEDKnob() {
		setSVG(SVG::load(assetPlugin(plugin, "res/components/PvCKnobT.svg")));
	}
};
struct PvCSnapKnob : PvCKnob {
	PvCSnapKnob() {
		snap = true;
	}
};

struct LabelButtonL : SVGSwitch, MomentarySwitch {
	LabelButtonL() {
		addFrame(SVG::load(assetPlugin(plugin, "res/components/LabelButtonL_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/components/LabelButtonL_1.svg")));
		box.size = Vec(36,12);
	}
};

struct LabelButtonS : SVGSwitch, MomentarySwitch {
	LabelButtonS() {
		addFrame(SVG::load(assetPlugin(plugin, "res/components/LabelButtonS_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/components/LabelButtonS_1.svg")));
		box.size = Vec(24,12);
	}
};

/// lights

struct PvCLight : GrayModuleLightWidget{
	PvCLight() { bgColor = nvgRGB(0x00, 0x00, 0x00); }
};
struct RedLED : PvCLight {
 	RedLED() {
 		addBaseColor(COLOR_RED); }
};
struct GreenLED : PvCLight {
 	GreenLED() {
 		addBaseColor(COLOR_GREEN); }
};
struct BlueLED : PvCLight {
 	BlueLED() {
 		addBaseColor(COLOR_BLUE); }
};
struct WhiteLED : PvCLight {
 	WhiteLED() {
 		addBaseColor(COLOR_WHITE); }
};
struct OrangeLED : PvCLight {
	OrangeLED() {
		addBaseColor(COLOR_ORANGE);	}
};
struct YellowLED : PvCLight {
	YellowLED() {
		addBaseColor(COLOR_YELLOW);	}
};
struct PurpleLED : PvCLight {
	PurpleLED() {
		addBaseColor(COLOR_PURPLE);	}
};
struct CyanLED : PvCLight {
	CyanLED() {
		addBaseColor(COLOR_CYAN);	}
};
struct GreenRedLED : PvCLight {
 	GreenRedLED() {
 		addBaseColor(COLOR_GREEN);
 		addBaseColor(COLOR_RED); }
};
struct WhiteRedLED : PvCLight {
	WhiteRedLED() {
		addBaseColor(COLOR_WHITE);
		addBaseColor(COLOR_RED);
	}
};

template <typename BASE>
 struct PvCBigLED : BASE {
	PvCBigLED() {
		this->box.size = Vec(22, 22);
	}
 };

template <typename BASE>
 struct FourPixLight : BASE {
 	FourPixLight() {
 		this->box.size = Vec(4, 4);
 	}
 };

template <typename BASE>
 struct EightPixLight : BASE {
 	EightPixLight() {
 		this->box.size = Vec(8, 8);
 	}
 };

/// screws

struct ScrewHead1 : SVGScrew {
	ScrewHead1() {
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/ScrewHead1.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};
struct ScrewHead2 : SVGScrew {
	ScrewHead2() {
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/ScrewHead2.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};
struct ScrewHead3 : SVGScrew {
	ScrewHead3() {
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/ScrewHead3.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};
struct ScrewHead4 : SVGScrew {
	ScrewHead4() {
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/ScrewHead4.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

} // namespace rack_plugin_PvC

using namespace rack_plugin_PvC;
