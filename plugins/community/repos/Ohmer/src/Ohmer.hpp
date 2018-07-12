#include "rack.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(Ohmer);

#ifdef USE_VST2
#define plugin "Ohmer"
#endif // USE_VST2

namespace rack_plugin_Ohmer {

//// COLOR TABLE USED FOR DOT-MATRIX DISPLAY (REGARDLING SELECTED MODEL).

static const NVGcolor tblDMDtextColor[6] = {
	nvgRGB(0x08, 0x08, 0x08), // LCD-like for Classic.
	nvgRGB(0x08, 0x08, 0x08), // LCD-like for Stage Repro.
	nvgRGB(0x08, 0x08, 0x08), // LCD-like for Absolute Night.
	nvgRGB(0xe0, 0xe0, 0xff), // Blue plasma-like for Dark "Signature".
	nvgRGB(0xff, 0x8a, 0x00), // Orange plasma-like for Deepblue "Signature".
	nvgRGB(0xb0, 0xff, 0xff) // Light cyan plasma-like for Carbon "Signature".
};

//// CUSTOM COMPONENTS (SCREWS, JACKS, KNOBS, ENCODERS, BUTTONS, LEDS).

// Custom silver Torx screw.
struct Torx_Silver : SVGScrew {
	Torx_Silver() {
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/Torx_Silver.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

// Custom gols Torx screw.
struct Torx_Gold : SVGScrew {
	Torx_Gold() {
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/Torx_Gold.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

// Silver momentary button (used by standard-line KlokSpid modules).
// This button is used for:
// - BPM start/stop toggle (KlokSpid module acting as standalone BPM clock generator).
// - entering Setup (by holding this button).
// - advance to next Setup parameter (and exit Setup).
struct KS_ButtonSilver : SVGSwitch, MomentarySwitch {
	KS_ButtonSilver() {
		addFrame(SVG::load(assetPlugin(plugin,"res/components/KS_Button_Up_Silver.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/components/KS_Button_Down_Silver.svg")));
	}
};

// Gold momentary button (used by Signature-line KlokSpid modules).
struct KS_ButtonGold : SVGSwitch, MomentarySwitch {
	KS_ButtonGold() {
		addFrame(SVG::load(assetPlugin(plugin,"res/components/KS_Button_Up_Gold.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/components/KS_Button_Down_Gold.svg")));
	}
};

// RKD jumper shunts (working as ON/OFF toggle switch).
struct RKD_Jumper : SVGSwitch, ToggleSwitch {
	RKD_Jumper() {
		addFrame(SVG::load(assetPlugin(plugin,"res/components/PCB_BJ_Off.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/components/PCB_BJ_On.svg")));
	}
};

// RKDBRK toggle switch (working as ON/OFF).
struct RKDBRK_Switch : SVGSwitch, ToggleSwitch {
	RKDBRK_Switch() {
		addFrame(SVG::load(assetPlugin(plugin,"res/components/NKKH_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/components/NKKH_1.svg")));
	}
};

// Custom port, with red in-ring (input port), gold.
struct PJ301M_In : SVGPort {
	PJ301M_In() {
		background->svg = SVG::load(assetPlugin(plugin,"res/components/PJ301M_In.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

// Custom port, with green in-ring (output port), gold.
struct PJ301M_Out : SVGPort {
	PJ301M_Out() {
		background->svg = SVG::load(assetPlugin(plugin,"res/components/PJ301M_Out.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

// Custom nickel metal port, with red in-ring (input port), used only by RKD & RKD-BRK modules (CLK jack). Derived from default CL1362.svg
struct CL1362_In : SVGPort {
	CL1362_In() {
		background->svg = SVG::load(assetPlugin(plugin,"res/components/CL1362_In.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

// Custom nickel metal port, with red in-ring (input port), used only by RKD & RKD-BRK modules (90° rotated for... ROTATE and RESET input ports). Derived from default CL1362.svg
struct CL1362_In_RR : SVGPort {
	CL1362_In_RR() {
		background->svg = SVG::load(assetPlugin(plugin,"res/components/CL1362_In_RR.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

// Custom nickel metal port, with green in-ring (output port), used only by RKD & RKD-BRK modules. Derived from default CL1362.svg
struct CL1362_Out : SVGPort {
	CL1362_Out() {
		background->svg = SVG::load(assetPlugin(plugin,"res/components/CL1362_Out.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

// Freeware "Moog-style" continuous encoder, used by KlokSpid and Metriks modules.
struct KS_Encoder : SVGKnob {
	KS_Encoder() {
  	minAngle = -1.0 * M_PI;
		maxAngle = M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/components/KS_Encoder.svg")));
		//smooth = false;
	}
};

// Custom orange color used by two small LEDs (CV-RATIO, start/stop), KlokSpid module.
// Also, this color is used for medium LED located below CV/TRIG port (KlokSpid module).
struct KlokSpidOrangeLight : GrayModuleLightWidget {
	KlokSpidOrangeLight() {
		addBaseColor(nvgRGB(0xe8, 0xad, 0x10));
	}
};

// White LED color for RKD & RKD-BRK modules (used for CLK and output 8).
struct RKDWhiteLight : GrayModuleLightWidget {
	RKDWhiteLight() {
		addBaseColor(nvgRGB(0xff, 0xff, 0xff));
	}
};

// White LED color for RKD & RKD-BRK modules(used for output 2).
struct RKDOrangeLight : GrayModuleLightWidget {
	RKDOrangeLight() {
		addBaseColor(nvgRGB(0xf2, 0xb1, 0x20));
	}
};

// White LED color for RKD & RKD-BRK modules (used for output 7).
struct RKDPurpleLight : GrayModuleLightWidget {
	RKDPurpleLight() {
		addBaseColor(nvgRGB(0xd5, 0x2b, 0xed));
	}
};

// Tri-colored red/orange/blue LED for RKD & RKD-BRK modules (used by "RESET").
struct RedOrangeBlueLight : GrayModuleLightWidget {
	RedOrangeBlueLight() {
		addBaseColor(COLOR_RED);
		addBaseColor(nvgRGB(0xe8, 0xad, 0x10)); // Orange (same used by KlokSpid module).
		addBaseColor(COLOR_BLUE);
	}
};

} // namespace rack_plugin_Ohmer

using namespace rack_plugin_Ohmer;
