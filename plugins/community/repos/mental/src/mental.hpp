#include "rack.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(mental);

#ifdef USE_VST2
#define plugin "mental"
#endif // USE_VST2

namespace rack_plugin_mental {

////////////////////
// module widgets
////////////////////

/////////////////////////////////////////////
// ports

struct OutPort : SVGPort {
	OutPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/OutPort.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct InPort : SVGPort {
	InPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/InPort.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct CVInPort : SVGPort {
	CVInPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/CVInPort.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct CVOutPort : SVGPort {
	CVOutPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/CVOutPort.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct GateInPort : SVGPort {
	GateInPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/GateInPort.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct GateOutPort : SVGPort {
	GateOutPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/GateOutPort.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

// Knobs

struct LrgKnob : RoundKnob {
	LrgKnob() {
		setSVG(SVG::load(assetPlugin(plugin, "res/components/LrgKnob.svg")));
		box.size = Vec(42,42);

	}
};

struct MedKnob : RoundKnob {
	MedKnob() {
		setSVG(SVG::load(assetPlugin(plugin, "res/components/MedKnob.svg")));
		box.size = Vec(24,24);

	}
};

struct SmlKnob : RoundKnob {
	SmlKnob() {
		setSVG(SVG::load(assetPlugin(plugin, "res/components/SmlKnob.svg")));
		box.size = Vec(20,20);
	}
};

// switches

struct ThreeWaySwitch : SVGSwitch, ToggleSwitch {
	ThreeWaySwitch() {
		addFrame(SVG::load(assetPlugin(plugin,"res/components/Three_2.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/components/Three_1.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/components/Three_0.svg")));
	}
};

// lights

/// lights

struct MentalLight : GrayModuleLightWidget{
	MentalLight() { bgColor = nvgRGB(0x40, 0x40, 0x40); }
};
struct RedLED : MentalLight {
 	RedLED() {
 		addBaseColor(nvgRGB(0xff, 0x00, 0x00)); }
};

struct BlueLED : MentalLight {
 	BlueLED() {
 		addBaseColor(nvgRGB(0x00, 0x47, 0x7e)); }
};

struct OrangeLED : MentalLight {
 	OrangeLED() {
 		addBaseColor(COLOR_ORANGE); }
};

template <typename BASE>
 struct TinyLight : BASE {
 	TinyLight() {
 		this->box.size = Vec(4, 4);
 	}
 };

template <typename BASE>
 struct SmlLight : BASE {
 	SmlLight() {
 		this->box.size = Vec(8, 8);
 	}
 };

template <typename BASE>
 struct MedLight : BASE {
 	MedLight() {
 		this->box.size = Vec(10, 10);
 	}
 };

} // namespace rack_plugin_mental
