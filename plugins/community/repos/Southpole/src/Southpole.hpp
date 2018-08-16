
#include "rack.hpp"

using namespace rack;

#define plugin "Southpole"

// GUI COMPONENTS

struct sp_Port : SVGPort {
	sp_Port() {
		setSVG(SVG::load(assetPlugin(plugin, "res/sp-Port20.svg")));
	}
};

struct sp_Switch : SVGSwitch, ToggleSwitch {
	sp_Switch() {
		addFrame(SVG::load(assetPlugin(plugin,"res/sp-switchv_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/sp-switchv_1.svg")));
	}
};

struct sp_Encoder : SVGKnob {
	sp_Encoder() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/sp-encoder.svg")));
		//sw->svg = SVG::load(assetPlugin(plugin, "res/sp-encoder.svg"));
		//sw->wrap();
		//box.size = sw->box.size;
	}
};

struct sp_BlackKnob : SVGKnob {
	sp_BlackKnob() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/sp-knobBlack.svg")));
		//sw->svg = SVG::load(assetPlugin(plugin, "res/sp-knobBlack.svg"));
		//sw->wrap();
		//box.size = Vec(32,32);
	}
};

struct sp_SmallBlackKnob : SVGKnob {
	sp_SmallBlackKnob() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/sp-knobBlack.svg")));
		//sw->svg = SVG::load(assetPlugin(plugin, "res/sp-knobBlack.svg"));
		//sw->wrap();
		//box.size = Vec(20,20);
	}
};

struct sp_Trimpot : SVGKnob {
	sp_Trimpot() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/sp-trimpotBlack.svg")));
		//sw->svg = SVG::load(assetPlugin(plugin, "res/sp-knobBlack.svg"));
		//sw->wrap();
		//box.size = Vec(18,18);
	}
};
