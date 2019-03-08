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
static const bool clockIgnoreOnRun = false;
//static const bool retrigGatesOnReset = true; no need yet, since no geodesic sequencers emit gates
static constexpr float clockIgnoreOnResetDuration = 0.001f;// disable clock on powerup and reset for 1 ms (so that the first step plays)
static const float lightLambda = 0.075f;
static const std::string lightPanelID = "White light edition";
static const std::string darkPanelID = "Dark matter edition";
static const unsigned int displayRefreshStepSkips = 256;
static const unsigned int userInputsStepSkipMask = 0xF;// sub interval of displayRefreshStepSkips, since inputs should be more responsive than lights
// above value should make it such that inputs are sampled > 1kHz so as to not miss 1ms triggers



// Variations on existing knobs, lights, etc


// Ports

struct GeoPort : DynamicSVGPort {
	GeoPort() {
		shadow->blurRadius = 10.0;
		shadow->opacity = 0.8;
		addFrame(SVG::load(assetPlugin(plugin, "res/WhiteLight/Jack-WL.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/DarkMatter/Jack-DM.svg")));
	}
};

struct BlankPort : SVGPort {
	BlankPort() {
		shadow->opacity = 0.0;
		setSVG(SVG::load(assetPlugin(plugin, "res/general/Otrsp-01.svg")));
	}
};


// Buttons and switches

struct GeoPushButton : DynamicSVGSwitch, MomentarySwitch {
	GeoPushButton() {// only one skin for now
		addFrameAll(SVG::load(assetPlugin(plugin, "res/general/PushButton1_0.svg")));
		addFrameAll(SVG::load(assetPlugin(plugin, "res/general/PushButton1_1.svg")));
	}
};



// Knobs

struct GeoKnob : DynamicSVGKnob {
	GeoKnob() {
		minAngle = -0.73*M_PI;
		maxAngle = 0.73*M_PI;
		shadow->blurRadius = 10.0;
		shadow->opacity = 0.8;
		//shadow->box.pos = Vec(0.0, box.size.y * 0.15); may need this if knob is small (taken from IMSmallKnob)
		addFrameAll(SVG::load(assetPlugin(plugin, "res/WhiteLight/Knob-WL.svg")));
		addFrameAll(SVG::load(assetPlugin(plugin, "res/DarkMatter/Knob-DM.svg")));
	}
};

struct GeoKnobTopRight : GeoKnob {
	GeoKnobTopRight() {orientationAngle = M_PI / 4.0f;}
};
struct GeoKnobRight : GeoKnob {
	GeoKnobRight() {orientationAngle = M_PI / 2.0f;}
};
struct GeoKnobBotRight : GeoKnob {
	GeoKnobBotRight() {orientationAngle = 3.0f * M_PI / 4.0f;}
};
struct GeoKnobBottom : GeoKnob {
	GeoKnobBottom() {orientationAngle = M_PI;}
};
struct GeoKnobBotLeft : GeoKnob {
	GeoKnobBotLeft() {orientationAngle = 5.0f * M_PI / 4.0f;}
};
struct GeoKnobLeft : GeoKnob {
	GeoKnobLeft() {orientationAngle = M_PI / -2.0f;}
};
struct GeoKnobTopLeft : GeoKnob {
	GeoKnobTopLeft() {orientationAngle = M_PI / -4.0f;}
};


struct BlankCKnob : DynamicSVGKnob {
	BlankCKnob() {
		minAngle = -0.73*M_PI;
		maxAngle = 0.73*M_PI;
		shadow->opacity = 0.0;
		addFrameAll(SVG::load(assetPlugin(plugin, "res/WhiteLight/C-WL.svg")));
		addFrameAll(SVG::load(assetPlugin(plugin, "res/DarkMatter/C-DM.svg")));
	}
};



// Lights

struct GeoGrayModuleLight : ModuleLightWidget {
	GeoGrayModuleLight() {
		bgColor = nvgRGB(0x8e, 0x8e, 0x8e);
		borderColor = nvgRGB(0x1d, 0x1d, 0x1b);//nvgRGBA(0, 0, 0, 0x60);
	}
	
	void drawLight(NVGcontext *vg) override { // from LightWidget.cpp (only nvgStrokeWidth of border was changed)
		float radius = box.size.x / 2.0;

		nvgBeginPath(vg);
		nvgCircle(vg, radius, radius, radius);

		// Background
		nvgFillColor(vg, bgColor);
		nvgFill(vg);

		// Foreground
		nvgFillColor(vg, color);
		nvgFill(vg);

		// Border
		nvgStrokeWidth(vg, 1.0);// 0.5);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);
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
};
struct GeoBlueYellowWhiteLight : GeoGrayModuleLight {
	GeoBlueYellowWhiteLight() {
		addBaseColor(COLOR_BLUE);
		addBaseColor(COLOR_YELLOW);
		addBaseColor(COLOR_WHITE);
	}
};
struct GeoBlueYellowLight : GeoGrayModuleLight {
	GeoBlueYellowLight() {
		addBaseColor(COLOR_BLUE);
		addBaseColor(COLOR_YELLOW);
	}
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

int getWeighted1to8random();

} // namespace rack_plugin_Geodesics

#endif
