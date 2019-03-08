#pragma once
#include "componentlibrary.hpp"
#include <vector>
#include <jansson.h>
#include "widgets.hpp"
#include <iostream>

using namespace std;

namespace rack {

struct BidooBlueKnob : RoundKnob {
	BidooBlueKnob() {
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/BlueKnobBidoo.svg")));
	}
};

struct BidooGreenKnob : RoundKnob {
	BidooGreenKnob() {
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/GreenKnobBidoo.svg")));
	}
};

struct BidooRedKnob : RoundKnob {
	BidooRedKnob() {
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/RedKnobBidoo.svg")));
	}
};

struct BidooHugeBlueKnob : RoundKnob {
	BidooHugeBlueKnob() {
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/HugeBlueKnobBidoo.svg")));
	}
};

struct BidooLargeBlueKnob : RoundKnob {
	BidooLargeBlueKnob() {
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/LargeBlueKnobBidoo.svg")));
	}
};

struct BidooSmallBlueKnob : RoundKnob {
	BidooSmallBlueKnob() {
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/SmallBlueKnobBidoo.svg")));
	}
};

struct BidooBlueSnapKnob : RoundBlackSnapKnob  {
	BidooBlueSnapKnob() {
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/BlueKnobBidoo.svg")));
	}
};

struct BidooBlueSnapTrimpot : Trimpot  {
	BidooBlueSnapTrimpot() {
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/BlueTrimpotBidoo.svg")));
		snap = true;
		smooth = false;
	}
};

struct BidooBlueTrimpot : Trimpot {
	BidooBlueTrimpot() {
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/BlueTrimpotBidoo.svg")));
	}
};

struct BlueCKD6 : SVGSwitch, MomentarySwitch {
	BlueCKD6() {
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/BlueCKD6_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/BlueCKD6_1.svg")));
	}
};

struct BlueBtn : SVGSwitch, MomentarySwitch {
	string caption;
	shared_ptr<Font> font;

	BlueBtn() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/BlueBtn_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/BlueBtn_1.svg")));
	}

	void draw(NVGcontext *vg) override {
		SVGSwitch::draw(vg);
		nvgFontSize(vg, 12.0f);
		nvgFontFaceId(vg, font->handle);
		nvgTextAlign(vg, NVG_ALIGN_CENTER);
		nvgText(vg, 8.0f, 12.0f, (caption).c_str(), NULL);
		nvgStroke(vg);
	}
};

struct RedBtn : SVGSwitch, MomentarySwitch {
	string caption;
	shared_ptr<Font> font;

	RedBtn() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/RedBtn_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/RedBtn_1.svg")));
	}

	void draw(NVGcontext *vg) override {
		SVGSwitch::draw(vg);
		nvgFontSize(vg, 12.0f);
		nvgFontFaceId(vg, font->handle);
		nvgTextAlign(vg, NVG_ALIGN_CENTER);
		nvgText(vg, 8.0f, 12.0f, (caption).c_str(), NULL);
		nvgStroke(vg);
	}
};

struct MuteBtn : SVGSwitch, MomentarySwitch {
	MuteBtn() {
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/MuteBtn_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/MuteBtn_1.svg")));
	}
};

struct SoloBtn : SVGSwitch, MomentarySwitch {
	SoloBtn() {
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/SoloBtn_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/SoloBtn_1.svg")));
	}
};

struct LeftBtn : SVGSwitch, MomentarySwitch {
	LeftBtn() {
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/LeftBtn_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/LeftBtn_1.svg")));
	}
};

struct RightBtn : SVGSwitch, MomentarySwitch {
	RightBtn() {
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/RightBtn_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/RightBtn_1.svg")));
	}
};

struct UpBtn : SVGSwitch, MomentarySwitch {
	UpBtn() {
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/UpBtn_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/UpBtn_1.svg")));
	}
};

struct DownBtn : SVGSwitch, MomentarySwitch {
	DownBtn() {
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/DownBtn_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/DownBtn_1.svg")));
	}
};

struct BidooziNCColoredKnob : RoundKnob {
	BidooziNCColoredKnob() {
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/ziNCBlueKnobBidoo.svg")));
	}
	float *coeff;
	void draw(NVGcontext *vg) override {
			for (NSVGshape *shape = this->sw->svg->handle->shapes; shape != NULL; shape = shape->next) {
				std::string str(shape->id);
				if (str == "bidooziNCBlueKnob") {
					int corrCoef = rescale(clamp(*coeff,0.0f,1.0f),0.0f,1.0f,0.0f,255.0f);
					shape->fill.color = (((unsigned int)clamp(42+corrCoef,0,255)) | ((unsigned int)clamp(87-corrCoef,0,255) << 8) | ((unsigned int)clamp(117-corrCoef,0,255) << 16));
					shape->fill.color |= (unsigned int)(255) << 24;
				}
			}
		RoundKnob::draw(vg);
	}
};

struct BidooColoredKnob : RoundKnob {
	BidooColoredKnob() {
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/BlackKnobBidoo.svg")));
	}

	void draw(NVGcontext *vg) override {
			for (NSVGshape *shape = this->sw->svg->handle->shapes; shape != NULL; shape = shape->next) {
				std::string str(shape->id);
				if (str == "bidooKnob") {
					shape->fill.color = (((unsigned int)42+(unsigned int)value*21) | (((unsigned int)87-(unsigned int)value*8) << 8) | (((unsigned int)117-(unsigned int)value) << 16));
					shape->fill.color |= (unsigned int)(255) << 24;
				}
			}
		RoundKnob::draw(vg);
	}
};

struct BidooMorphKnob : RoundKnob {
	BidooMorphKnob() {
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/SpiralKnobBidoo.svg")));
	}
};

struct BidooColoredTrimpot : RoundKnob {
	BidooColoredTrimpot() {
		minAngle = -0.75f*M_PI;
		maxAngle = 0.75f*M_PI;
		setSVG(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/TrimpotBidoo.svg")));
	}

	void draw(NVGcontext *vg) override {
		for (NSVGshape *shape = this->sw->svg->handle->shapes; shape != NULL; shape = shape->next) {
			std::string str(shape->id);
			if (str == "bidooTrimPot") {
				if (value == 0.0f) {
					shape->fill.color = (((unsigned int)128) | ((unsigned int)128 << 8) | ((unsigned int)128 << 16));
					shape->fill.color |= (unsigned int)(120) << 24;
				} else {
					shape->fill.color = (((unsigned int)255) | (((unsigned int)(205 - (value *15)))<< 8) | ((unsigned int)10 << 16));
					shape->fill.color |= ((unsigned int)255) << 24;
				}
			}
		}
		RoundKnob::draw(vg);
	}
};

struct BidooSlidePotLong : SVGFader {
	BidooSlidePotLong() {
		snap = true;
		maxHandlePos = Vec(0.0f, 0.0f);
		minHandlePos = Vec(0.0f, 84.0f);
		background->svg = SVG::load(assetPlugin(plugin,"res/ComponentLibrary/bidooSlidePotLong.svg"));
		background->wrap();
		background->box.pos = Vec(0.0f, 0.0f);
		box.size = background->box.size;
		handle->svg = SVG::load(assetPlugin(plugin,"res/ComponentLibrary/bidooSlidePotHandle.svg"));
		handle->wrap();
	}

	void randomize() override {
  	setValue(roundf(rescale(randomUniform(), 0.0f, 1.0f, minValue, maxValue)));
  }
};

struct BidooSlidePotShort : SVGFader {
	BidooSlidePotShort() {
		snap = true;
		maxHandlePos = Vec(0.0f, 0.0f);
		minHandlePos = Vec(0.0f, 60.0f);
		background->svg = SVG::load(assetPlugin(plugin,"res/ComponentLibrary/bidooSlidePotShort.svg"));
		background->wrap();
		background->box.pos = Vec(0.0f, 0.0f);
		box.size = background->box.size;
		handle->svg = SVG::load(assetPlugin(plugin,"res/ComponentLibrary/bidooSlidePotHandle.svg"));
		handle->wrap();
	}

	void randomize() override {
  	setValue(roundf(rescale(randomUniform(), 0.0f, 1.0f, minValue, maxValue)));
  }
};

struct BidooLongSlider : SVGFader {
	BidooLongSlider() {
		maxHandlePos = Vec(0.0f, 0.0f);
		minHandlePos = Vec(0.0f, 84.0f);
		background->svg = SVG::load(assetPlugin(plugin,"res/ComponentLibrary/bidooLongSlider.svg"));
		background->wrap();
		background->box.pos = Vec(0.0f, 0.0f);
		box.size = background->box.size;
		handle->svg = SVG::load(assetPlugin(plugin,"res/ComponentLibrary/bidooLongSliderHandle.svg"));
		handle->wrap();
	}
};

struct CKSS8 : SVGSwitch, ToggleSwitch {
	CKSS8() {
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/CKSS8_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/CKSS8_1.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/CKSS8_2.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/CKSS8_3.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/CKSS8_4.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/CKSS8_5.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/CKSS8_6.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/CKSS8_7.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct CKSS4 : SVGSwitch, ToggleSwitch {
	CKSS4() {
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/CKSS4_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/CKSS4_1.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/CKSS4_2.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/CKSS4_3.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct TinyPJ301MPort : SVGPort {
	TinyPJ301MPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/ComponentLibrary/TinyPJ301M.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct MiniLEDButton : SVGSwitch, MomentarySwitch {
	MiniLEDButton() {
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/miniLEDButton.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};


} // namespace rack
