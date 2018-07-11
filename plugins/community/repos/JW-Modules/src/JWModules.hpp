#pragma once
#include "rack.hpp"
#include "QuantizeUtils.cpp"

using namespace rack;

RACK_PLUGIN_DECLARE(JW_Modules);

#ifdef USE_VST2
#define plugin "JW-Modules"
#endif // USE_VST2

namespace rack_plugin_JW_Modules {

////////////////////////////////////////////// LABELS //////////////////////////////////////////////

struct CenteredLabel : Widget {
	std::string text;
	int fontSize;
	CenteredLabel(int _fontSize = 12) {
		fontSize = _fontSize;
		box.size.y = BND_WIDGET_HEIGHT;
	}
	void draw(NVGcontext *vg) override {
		nvgTextAlign(vg, NVG_ALIGN_CENTER);
		nvgFillColor(vg, nvgRGB(25, 150, 252));
		nvgFontSize(vg, fontSize);
		nvgText(vg, box.pos.x, box.pos.y, text.c_str(), NULL);
	}
};

////////////////////////////////////////////// KNOBS //////////////////////////////////////////////

struct SmallWhiteKnob : RoundKnob {
	SmallWhiteKnob() {
		setSVG(SVG::load(assetPlugin(plugin, "res/SmallWhiteKnob.svg")));
	}
	CenteredLabel* linkedLabel = nullptr;
	
	void connectLabel(CenteredLabel* label) {
		linkedLabel = label;
		if (linkedLabel) {
			linkedLabel->text = formatCurrentValue();
		}
	}

	void onChange(EventChange &e) override {
		RoundKnob::onChange(e);
		if (linkedLabel) {

			linkedLabel->text = formatCurrentValue();
		}
	}

	virtual std::string formatCurrentValue() {
		return std::to_string(static_cast<unsigned int>(value));
	}
};

struct NoteKnob : SmallWhiteKnob {
	QuantizeUtils *quantizeUtils;
	NoteKnob(){
		snap = true;
	}
	std::string formatCurrentValue() override {
		return quantizeUtils->noteName(int(value));
	}
};

struct ScaleKnob : SmallWhiteKnob {
	QuantizeUtils *quantizeUtils;
	ScaleKnob(){
		snap = true;
	}
	std::string formatCurrentValue() override {
		return quantizeUtils->scaleName(int(value));
	}
};

struct JwSmallSnapKnob : SmallWhiteKnob {
	JwSmallSnapKnob() {
		snap = true;
	}
};

struct JwTinyKnob : RoundKnob {
	JwTinyKnob() {
		setSVG(SVG::load(assetPlugin(plugin, "res/TinyWhiteKnob.svg")));
	}
};

struct BPMPartKnob : JwSmallSnapKnob {	
	BPMPartKnob(){} 
};

////////////////////////////////////////////// SWITCHES //////////////////////////////////////////////

struct JwHorizontalSwitch : SVGSwitch, ToggleSwitch {
	JwHorizontalSwitch() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Switch_Horizontal_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Switch_Horizontal_1.svg")));
	}
};

struct JwVerticalSwitch : SVGSwitch, ToggleSwitch {
	JwVerticalSwitch() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Switch_Vertical_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Switch_Vertical_1.svg")));
	}
};

struct BowlSwitch : SVGSwitch, ToggleSwitch {
	BowlSwitch() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Bowl-no-food.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Bowl-food.svg")));
	}
};

////////////////////////////////////////////// PORTS //////////////////////////////////////////////

struct TinyPJ301MPort : SVGPort {
	TinyPJ301MPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/TinyPJ301M.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct Orange_TinyPJ301MPort : SVGPort {
	Orange_TinyPJ301MPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/TinyPJ301M_orange.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct Yellow_TinyPJ301MPort : SVGPort {
	Yellow_TinyPJ301MPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/TinyPJ301M_yellow.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct Purple_TinyPJ301MPort : SVGPort {
	Purple_TinyPJ301MPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/TinyPJ301M_purple.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct Blue_TinyPJ301MPort : SVGPort {
	Blue_TinyPJ301MPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/TinyPJ301M_blue.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct White_TinyPJ301MPort : SVGPort {
	White_TinyPJ301MPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/TinyPJ301M_white.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

////////////////////////////////////////////// LIGHTS //////////////////////////////////////////////

struct MyBlueValueLight : ModuleLightWidget {
	MyBlueValueLight() {
		firstLightId = 1;
		addBaseColor(nvgRGB(25, 150, 252));
	}
};

struct MyGreenValueLight : ModuleLightWidget {
	MyGreenValueLight() {
		firstLightId = 1;
		addBaseColor(nvgRGB(0, 200, 0));
	}
};

struct MyRedValueLight : ModuleLightWidget {
	MyRedValueLight() {
		firstLightId = 1;
		addBaseColor(nvgRGB(200, 0, 0));
	}
};

////////////////////////////////////////////// BUTTONS //////////////////////////////////////////////

struct RightMoveButton : SVGSwitch, MomentarySwitch {
	RightMoveButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/RightButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/RightButtonDown.svg")));
	}
};

struct LeftMoveButton : SVGSwitch, MomentarySwitch {
	LeftMoveButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/LeftButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/LeftButtonDown.svg")));
	}
};

struct DownMoveButton : SVGSwitch, MomentarySwitch {
	DownMoveButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/DownButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/DownButtonDown.svg")));
	}
};

struct UpMoveButton : SVGSwitch, MomentarySwitch {
	UpMoveButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/UpButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/UpButtonDown.svg")));
	}
};

struct RndMoveButton : SVGSwitch, MomentarySwitch {
	RndMoveButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/RndButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/RndButtonDown.svg")));
	}
};

struct RepMoveButton : SVGSwitch, MomentarySwitch {
	RepMoveButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/RepButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/RepButtonDown.svg")));
	}
};

struct TinyButton : SVGSwitch, MomentarySwitch {
	TinyButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/TinyButtonUp.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/TinyButtonDown.svg")));
	}
};

struct SmallButton : SVGSwitch, MomentarySwitch {
	SmallButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/SmallButtonUp.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/SmallButtonDown.svg")));
	}
};

////////////////////////////////////////////// SCREWS //////////////////////////////////////////////

struct Snowflake : SVGScrew {
	Snowflake() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/SnowFlake.svg")));
		box.size = sw->box.size;
	}
};

struct WavHeadLogo : SVGScrew {
	WavHeadLogo() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/WavHeadSmall.svg")));
		box.size = sw->box.size;
	}
};

struct Screw_J : SVGScrew {
	Screw_J() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/Screw_J.svg")));
		box.size = sw->box.size;
	}
};

struct Screw_W : SVGScrew {
	Screw_W() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/Screw_W.svg")));
		box.size = sw->box.size;
	}
};

struct CatScrew : SVGScrew {
	CatScrew() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/Cat.svg")));
		box.size = sw->box.size;
	}
};

struct HairballScrew : SVGScrew {
	HairballScrew() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/Hairball.svg")));
		box.size = sw->box.size;
	}
};

////////////////////////////////////////////// WIDGETS //////////////////////////////////////////////

inline int clampijw(int x, int minimum, int maximum) {
	return min(max(x, minimum), maximum);
}
inline float clampfjw(float x, float minimum, float maximum) {
	return fminf(fmaxf(x, minimum), maximum);
}
inline float rescalefjw(float x, float xMin, float xMax, float yMin, float yMax) {
	return yMin + (x - xMin) / (xMax - xMin) * (yMax - yMin);
}


} // namespace rack_plugin_JW_Modules
