//***********************************************************************************************
//Geodesics: A modular collection for VCV Rack by Pierre Collard and Marc Boulé
//
//Based on code from the Fundamental plugins by Andrew Belt 
//  and graphics from the Component Library by Wes Milholen 
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//
//***********************************************************************************************


#ifndef GEODESICS_HPP
#define GEODESICS_HPP

#include "rack.hpp"
#include "GeoWidgets.hpp"
#include "dsp/digital.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(Geodesics);
#ifdef USE_VST2
#define plugin "Geodesics"
#endif // USE_VST2


namespace rack_plugin_Geodesics {

// General constants
static const float lightLambda = 0.075f;
static const std::string lightPanelID = "White light";
static const std::string darkPanelID = "Dark copper";



// Variations on existing knobs, lights, etc


// Ports

struct GeoPort : DynamicSVGPort {
	GeoPort() {
		shadow->blurRadius = 10.0;
		shadow->opacity = 0.8;
		addFrame(SVG::load(assetPlugin(plugin, "res/light/comp/Jack.svg")));
		//addFrame(SVG::load(assetPlugin(plugin, "res/dark/comp/Jack.svg")));// no dark ports in Geodesics for now
	}
};

struct BlankPort : SVGPort {
	BlankPort() {
		shadow->opacity = 0.0;
		setSVG(SVG::load(assetPlugin(plugin, "res/comp/Otrsp-01.svg")));
	}
};


// Buttons and switches

struct GeoPushButton : DynamicSVGSwitch, MomentarySwitch {
	GeoPushButton() {// only one skin for now
		addFrameAll(SVG::load(assetPlugin(plugin, "res/light/comp/PushButton1_0.svg")));
		addFrameAll(SVG::load(assetPlugin(plugin, "res/light/comp/PushButton1_1.svg")));
		//addFrameAll(SVG::load(assetPlugin(plugin, "res/dark/comp/CKD6b_0.svg"))); // no dark buttons in Geodesics for now
		//addFrameAll(SVG::load(assetPlugin(plugin, "res/dark/comp/CKD6b_1.svg"))); // no dark buttons in Geodesics for now
	}
};



// Knobs

struct GeoKnob : DynamicSVGKnob {
	GeoKnob() {
		minAngle = -0.73*M_PI;
		maxAngle = 0.73*M_PI;
		shadow->blurRadius = 10.0;
		shadow->opacity = 0.8;
		//shadow->box.pos = Vec(0.0, box.size.y * 0.15); may need this if know is small (taken from IMSmallKnob)
		addFrameAll(SVG::load(assetPlugin(plugin, "res/light/comp/Knob.svg")));
		//addFrameAll(SVG::load(assetPlugin(plugin, "res/dark/comp/Knob.svg")));// no dark knobs in Geodesics for now
	}
};

struct GeoKnobRight : GeoKnob {
	GeoKnobRight() {
		orientationAngle = M_PI / 2.0f;
	}
};
struct GeoKnobLeft : GeoKnob {
	GeoKnobLeft() {
		orientationAngle = M_PI / -2.0f;
	}
};
struct GeoKnobBottom : GeoKnob {
	GeoKnobBottom() {
		orientationAngle = M_PI;
	}
};


struct BlankCKnob : SVGKnob {
	BlankCKnob() {
		minAngle = -0.73*M_PI;
		maxAngle = 0.73*M_PI;
		shadow->opacity = 0.0;
		setSVG(SVG::load(assetPlugin(plugin, "res/comp/C-01.svg")));
	}
};



// Lights

struct GeoGrayModuleLight : ModuleLightWidget {
	GeoGrayModuleLight() {
		bgColor = nvgRGB(0x8e, 0x8e, 0x8e);
		borderColor = nvgRGBA(0, 0, 0, 0x60);
	}
};

struct GeoWhiteLight : GeoGrayModuleLight {
	GeoWhiteLight() {
		addBaseColor(COLOR_WHITE);
	}
};
struct GeoBlueLight : GeoGrayModuleLight {
	GeoBlueLight() {
		addBaseColor(COLOR_BLUE);
	}
};
struct GeoRedLight : GeoGrayModuleLight {
	GeoRedLight() {
		addBaseColor(COLOR_RED);
	}
};
struct GeoYellowLight : GeoGrayModuleLight {
	GeoYellowLight() {
		addBaseColor(COLOR_YELLOW);
	}
};
struct GeoWhiteRedLight : GeoGrayModuleLight {
	GeoWhiteRedLight() {
		addBaseColor(COLOR_WHITE);
		addBaseColor(COLOR_RED);
	}
};struct GeoBlueYellowWhiteLight : GeoGrayModuleLight {
	GeoBlueYellowWhiteLight() {
		addBaseColor(COLOR_BLUE);
		addBaseColor(COLOR_YELLOW);
		addBaseColor(COLOR_WHITE);
	}
};


// Other

} // namespace rack_plugin_Geodesics

using namespace rack_plugin_Geodesics;

#endif
