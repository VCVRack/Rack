#include "rack.hpp"

#pragma once

using namespace rack;

namespace rack_plugin_AS {
}

RACK_PLUGIN_DECLARE(AS);

#ifdef USE_VST2
#define plugin "AS"
#endif // USE_VST2


//////////////////////////////////////
//MODULE COMPONENTS
/////////////////////////////////////
struct as_HexScrew : SVGScrew {
	as_HexScrew() {
		sw->svg = SVG::load(assetPlugin(plugin, "res/as-hexscrew.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct as_Knob : SVGKnob {
	as_Knob() {
      minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSVG( SVG::load(assetPlugin(plugin, "res/as-knob.svg")) );
	}
};
struct as_KnobBlack : SVGKnob {
	as_KnobBlack() {
      minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSVG( SVG::load(assetPlugin(plugin, "res/as-knobBlack.svg")) );
	}
};

struct as_FxKnobWhite : SVGKnob {
	as_FxKnobWhite() {
      minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSVG( SVG::load(assetPlugin(plugin, "res/as-FxKnobWhite.svg")) );
	}
};
struct as_FxKnobBlack : SVGKnob {
	as_FxKnobBlack() {
      minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSVG( SVG::load(assetPlugin(plugin, "res/as-FxKnobBlack.svg")) );
	}
};

struct BigLEDBezel : SVGSwitch, MomentarySwitch {
   BigLEDBezel() {
      addFrame(SVG::load(assetPlugin(plugin, "res/as_bigLEDBezel.svg")));
   }
};

struct as_CKSS : SVGSwitch, ToggleSwitch {
	as_CKSS() {
		addFrame(SVG::load(assetPlugin(plugin,"res/as_CKSS_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/as_CKSS_1.svg")));
	}
};

struct as_CKSSH : SVGSwitch, ToggleSwitch {
	as_CKSSH() {
		addFrame(SVG::load(assetPlugin(plugin, "res/as_CKSSH_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/as_CKSSH_1.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct as_CKSSThree : SVGSwitch, ToggleSwitch {
	as_CKSSThree() {
		addFrame(SVG::load(assetPlugin(plugin,"res/as_CKSSThree_2.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/as_CKSSThree_1.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/as_CKSSThree_0.svg")));
	}
};

struct as_MuteBtn : SVGSwitch, ToggleSwitch {
	as_MuteBtn() {
		addFrame(SVG::load(assetPlugin(plugin,"res/as_mute-off.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/as_mute-on.svg")));
	}
};

struct as_SoloBtn : SVGSwitch, ToggleSwitch {
	as_SoloBtn() {
		addFrame(SVG::load(assetPlugin(plugin,"res/as_solo-off.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/as_solo-on.svg")));
	}
};

template <typename BASE>
struct GiantLight : BASE {
        GiantLight() {
                this->box.size = mm2px(Vec(18.0, 18.0));
        }
};

template <typename BASE>
 struct MeterLight : BASE {
 	MeterLight() {
 		this->box.size = Vec(8, 8);
		 this->bgColor = nvgRGB(0x14, 0x14, 0x14);
 	}
 };

 template <typename BASE>
 struct DisplayLedLight : BASE {
 	DisplayLedLight() {
 		this->box.size = mm2px(Vec(2.0, 2.0));
		 this->bgColor = nvgRGB(0x14, 0x14, 0x14);
 	}
 };
 struct OrangeLight : GrayModuleLightWidget {
	OrangeLight() {
		addBaseColor(COLOR_ORANGE);
	}
};

template <typename BASE>
struct LedLight : BASE {
	LedLight() {
	  //this->box.size = Vec(20.0, 20.0);
	  this->box.size = mm2px(Vec(6.0, 6.0));
	}
};

/** Reads two adjacent lightIds, so `lightId` and `lightId + 1` must be defined */
struct YellowRedLight : GrayModuleLightWidget {
	YellowRedLight() {
		addBaseColor(COLOR_YELLOW);
		addBaseColor(COLOR_RED);
	}
};

struct as_PJ301MPort : SVGPort {
	as_PJ301MPort() {
		setSVG(SVG::load(assetPlugin(plugin,"res/as-PJ301M.svg")));
		//background->svg = SVG::load(assetPlugin(plugin,"res/as-PJ301M.svg"));
		//background->wrap();
		//box.size = background->box.size;
	}
};

struct as_SlidePot : SVGFader {
	as_SlidePot() {
		Vec margin = Vec(4, 4);
		maxHandlePos = Vec(-1.5, -8).plus(margin);
		minHandlePos = Vec(-1.5, 87).plus(margin);
		background->svg = SVG::load(assetPlugin(plugin,"res/as-SlidePot.svg"));
		background->wrap();
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));
		handle->svg = SVG::load(assetPlugin(plugin,"res/as-SlidePotHandle.svg"));
		handle->wrap();
	}
};

struct as_FaderPot : SVGFader {
	as_FaderPot() {
		Vec margin = Vec(4, 4);
		maxHandlePos = Vec(-1.5, -8).plus(margin);
		minHandlePos = Vec(-1.5, 57).plus(margin);
		background->svg = SVG::load(assetPlugin(plugin,"res/as-FaderPot.svg"));
		background->wrap();
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));
		handle->svg = SVG::load(assetPlugin(plugin,"res/as-SlidePotHandle.svg"));
		handle->wrap();
	}
};
