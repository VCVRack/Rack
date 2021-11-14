#pragma once
#include <widget/FramebufferWidget.hpp>
#include <widget/SvgWidget.hpp>
#include <app/SvgKnob.hpp>
#include <app/SvgSlider.hpp>
#include <app/SvgPort.hpp>
#include <app/ModuleLightWidget.hpp>
#include <app/SvgSwitch.hpp>
#include <app/SvgScrew.hpp>
#include <app/AudioDisplay.hpp>
#include <app/MidiDisplay.hpp>
#include <asset.hpp>


namespace rack {

/** Library of Rack components: knobs, ports, lights, switches, buttons, etc.

See LICENSE.md for legal details about using Rack component graphics in your Rack plugin.
*/
namespace componentlibrary {


using namespace window;


////////////////////
// Color scheme
////////////////////

static const NVGcolor SCHEME_BLACK_TRANSPARENT = nvgRGBA(0x00, 0x00, 0x00, 0x00);
static const NVGcolor SCHEME_BLACK = nvgRGB(0x00, 0x00, 0x00);
static const NVGcolor SCHEME_WHITE = nvgRGB(0xff, 0xff, 0xff);
static const NVGcolor SCHEME_RED = nvgRGB(0xed, 0x2c, 0x24);
static const NVGcolor SCHEME_ORANGE = nvgRGB(0xf2, 0xb1, 0x20);
static const NVGcolor SCHEME_YELLOW = nvgRGB(0xff, 0xd7, 0x14);
static const NVGcolor SCHEME_GREEN = nvgRGB(0x90, 0xc7, 0x3e);
static const NVGcolor SCHEME_CYAN = nvgRGB(0x22, 0xe6, 0xef);
static const NVGcolor SCHEME_BLUE = nvgRGB(0x29, 0xb2, 0xef);
static const NVGcolor SCHEME_PURPLE = nvgRGB(0xd5, 0x2b, 0xed);
static const NVGcolor SCHEME_LIGHT_GRAY = nvgRGB(0xe6, 0xe6, 0xe6);
static const NVGcolor SCHEME_DARK_GRAY = nvgRGB(0x17, 0x17, 0x17);


////////////////////
// Lights
////////////////////

/*
Many of these classes use CRTP (https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern).

To use a red light with its default base class for example, use `RedLight` or `TRedLight<>`. (They are synonymous.)

Use the `TBase` template argument if you want a different base class.
E.g. `RectangleLight<RedLight>`

Although this paradigm might seem confusing at first, it ends up being extremely simple in your plugin code and perfect for "decorating" your classes with appearance traits and behavioral properties.
For example, need a slider with a green LED? Just use

	createLightParamCentered<VCVLightSlider<GreenLight>>(...)
*/

template <typename TBase = app::ModuleLightWidget>
struct TSvgLight : TBase {
	widget::FramebufferWidget* fb;
	widget::SvgWidget* sw;

	TSvgLight() {
		fb = new widget::FramebufferWidget;
		this->addChild(fb);

		sw = new widget::SvgWidget;
		fb->addChild(sw);
	}

	void setSvg(std::shared_ptr<Svg> svg) {
		sw->setSvg(svg);
		fb->box.size = sw->box.size;
		this->box.size = sw->box.size;
	}
};
using SvgLight = TSvgLight<>;

template <typename TBase = app::ModuleLightWidget>
struct TGrayModuleLightWidget : TBase {
	TGrayModuleLightWidget() {
		this->bgColor = nvgRGBA(0x33, 0x33, 0x33, 0xff);
		this->borderColor = nvgRGBA(0, 0, 0, 53);
	}
};
using GrayModuleLightWidget = TGrayModuleLightWidget<>;

template <typename TBase = GrayModuleLightWidget>
struct TWhiteLight : TBase {
	TWhiteLight() {
		this->addBaseColor(SCHEME_WHITE);
	}
};
using WhiteLight = TWhiteLight<>;

template <typename TBase = GrayModuleLightWidget>
struct TRedLight : TBase {
	TRedLight() {
		this->addBaseColor(SCHEME_RED);
	}
};
using RedLight = TRedLight<>;

template <typename TBase = GrayModuleLightWidget>
struct TGreenLight : TBase {
	TGreenLight() {
		this->addBaseColor(SCHEME_GREEN);
	}
};
using GreenLight = TGreenLight<>;

template <typename TBase = GrayModuleLightWidget>
struct TBlueLight : TBase {
	TBlueLight() {
		this->addBaseColor(SCHEME_BLUE);
	}
};
using BlueLight = TBlueLight<>;

template <typename TBase = GrayModuleLightWidget>
struct TYellowLight : TBase {
	TYellowLight() {
		this->addBaseColor(SCHEME_YELLOW);
	}
};
using YellowLight = TYellowLight<>;

/** Reads two adjacent lightIds, so `lightId` and `lightId + 1` must be defined */
template <typename TBase = GrayModuleLightWidget>
struct TGreenRedLight : TBase {
	TGreenRedLight() {
		this->addBaseColor(SCHEME_GREEN);
		this->addBaseColor(SCHEME_RED);
	}
};
using GreenRedLight = TGreenRedLight<>;

template <typename TBase = GrayModuleLightWidget>
struct TRedGreenBlueLight : TBase {
	TRedGreenBlueLight() {
		this->addBaseColor(SCHEME_RED);
		this->addBaseColor(SCHEME_GREEN);
		this->addBaseColor(SCHEME_BLUE);
	}
};
using RedGreenBlueLight = TRedGreenBlueLight<>;

/** Based on the size of 5mm LEDs */
template <typename TBase>
struct LargeLight : TSvgLight<TBase> {
	LargeLight() {
		this->setSvg(Svg::load(asset::system("res/ComponentLibrary/LargeLight.svg")));
	}
};

/** Based on the size of 3mm LEDs */
template <typename TBase>
struct MediumLight : TSvgLight<TBase> {
	MediumLight() {
		this->setSvg(Svg::load(asset::system("res/ComponentLibrary/MediumLight.svg")));
	}
};

/** Based on the size of 2mm LEDs */
template <typename TBase>
struct SmallLight : TSvgLight<TBase> {
	SmallLight() {
		this->setSvg(Svg::load(asset::system("res/ComponentLibrary/SmallLight.svg")));
	}
};

/** Based on the size of 1mm LEDs */
template <typename TBase>
struct TinyLight : TSvgLight<TBase> {
	TinyLight() {
		this->setSvg(Svg::load(asset::system("res/ComponentLibrary/TinyLight.svg")));
	}
};

/** Based on the size of 5mm LEDs */
template <typename TBase = GrayModuleLightWidget>
struct LargeSimpleLight : TBase {
	LargeSimpleLight() {
		this->box.size = mm2px(math::Vec(5, 5));
	}
};

/** Based on the size of 3mm LEDs */
template <typename TBase = GrayModuleLightWidget>
struct MediumSimpleLight : TBase {
	MediumSimpleLight() {
		this->box.size = mm2px(math::Vec(3, 3));
	}
};

/** Based on the size of 2mm LEDs */
template <typename TBase = GrayModuleLightWidget>
struct SmallSimpleLight : TBase {
	SmallSimpleLight() {
		this->box.size = mm2px(math::Vec(2, 2));
	}
};

/** Based on the size of 1mm LEDs */
template <typename TBase = GrayModuleLightWidget>
struct TinySimpleLight : TBase {
	TinySimpleLight() {
		this->box.size = mm2px(math::Vec(1, 1));
	}
};

template <typename TBase>
struct RectangleLight : TBase {
	void drawBackground(const widget::Widget::DrawArgs& args) override {
		// Derived from LightWidget::drawBackground()

		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, this->box.size.x, this->box.size.y);

		// Background
		if (this->bgColor.a > 0.0) {
			nvgFillColor(args.vg, this->bgColor);
			nvgFill(args.vg);
		}

		// Border
		if (this->borderColor.a > 0.0) {
			nvgStrokeWidth(args.vg, 0.5);
			nvgStrokeColor(args.vg, this->borderColor);
			nvgStroke(args.vg);
		}
	}

	void drawLight(const widget::Widget::DrawArgs& args) override {
		// Derived from LightWidget::drawLight()

		// Foreground
		if (this->color.a > 0.0) {
			nvgBeginPath(args.vg);
			nvgRect(args.vg, 0, 0, this->box.size.x, this->box.size.y);

			nvgFillColor(args.vg, this->color);
			nvgFill(args.vg);
		}
	}
};

/** A light for displaying on top of VCVBezel. Must add a color by subclassing or templating. */
template <typename TBase>
struct VCVBezelLight : TBase {
	VCVBezelLight() {
		this->borderColor = color::BLACK_TRANSPARENT;
		this->bgColor = color::BLACK_TRANSPARENT;
		this->box.size = math::Vec(17.545, 17.545);
	}
};
template <typename TBase>
using LEDBezelLight = VCVBezelLight<TBase>;

/** A light to displayed over PB61303. Must add a color by subclassing or templating.
*/
template <typename TBase>
struct PB61303Light : TBase {
	PB61303Light() {
		this->bgColor = color::BLACK_TRANSPARENT;
		this->box.size = mm2px(math::Vec(9.0, 9.0));
	}
};


////////////////////
// Knobs
////////////////////

struct RoundKnob : app::SvgKnob {
	widget::SvgWidget* bg;

	RoundKnob() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;

		bg = new widget::SvgWidget;
		fb->addChildBelow(bg, tw);
	}
};

struct RoundBlackKnob : RoundKnob {
	RoundBlackKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundBlackKnob.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundBlackKnob_bg.svg")));
	}
};

struct RoundSmallBlackKnob : RoundKnob {
	RoundSmallBlackKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundSmallBlackKnob.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundSmallBlackKnob_bg.svg")));
	}
};

struct RoundLargeBlackKnob : RoundKnob {
	RoundLargeBlackKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundLargeBlackKnob.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundLargeBlackKnob_bg.svg")));
	}
};

struct RoundBigBlackKnob : RoundKnob {
	RoundBigBlackKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundBigBlackKnob.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundBigBlackKnob_bg.svg")));
	}
};

struct RoundHugeBlackKnob : RoundKnob {
	RoundHugeBlackKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundHugeBlackKnob.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundHugeBlackKnob_bg.svg")));
	}
};

struct RoundBlackSnapKnob : RoundBlackKnob {
	RoundBlackSnapKnob() {
		snap = true;
	}
};


struct Davies1900hKnob : app::SvgKnob {
	widget::SvgWidget* bg;

	Davies1900hKnob() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;

		bg = new widget::SvgWidget;
		fb->addChildBelow(bg, tw);
	}
};

struct Davies1900hWhiteKnob : Davies1900hKnob {
	Davies1900hWhiteKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hWhite.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hWhite_bg.svg")));
	}
};

struct Davies1900hBlackKnob : Davies1900hKnob {
	Davies1900hBlackKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hBlack.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hBlack_bg.svg")));
	}
};

struct Davies1900hRedKnob : Davies1900hKnob {
	Davies1900hRedKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hRed.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hRed_bg.svg")));
	}
};

struct Davies1900hLargeWhiteKnob : Davies1900hKnob {
	Davies1900hLargeWhiteKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hLargeWhite.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hLargeWhite_bg.svg")));
	}
};

struct Davies1900hLargeBlackKnob : Davies1900hKnob {
	Davies1900hLargeBlackKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hLargeBlack.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hLargeBlack_bg.svg")));
	}
};

struct Davies1900hLargeRedKnob : Davies1900hKnob {
	Davies1900hLargeRedKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hLargeRed.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hLargeRed_bg.svg")));
	}
};


struct Rogan : app::SvgKnob {
	widget::SvgWidget* bg;
	widget::SvgWidget* fg;

	Rogan() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;

		bg = new widget::SvgWidget;
		fb->addChildBelow(bg, tw);

		fg = new widget::SvgWidget;
		fb->addChildAbove(fg, tw);
	}
};

struct Rogan6PSWhite : Rogan {
	Rogan6PSWhite() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan6PSWhite.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan6PS_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan6PSWhite_fg.svg")));
	}
};

struct Rogan5PSGray : Rogan {
	Rogan5PSGray() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan5PSGray.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan5PS_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan5PSGray_fg.svg")));
	}
};

struct Rogan3PSBlue : Rogan {
	Rogan3PSBlue() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PSBlue.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PS_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PSBlue_fg.svg")));
	}
};

struct Rogan3PSRed : Rogan {
	Rogan3PSRed() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PSRed.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PS_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PSRed_fg.svg")));
	}
};

struct Rogan3PSGreen : Rogan {
	Rogan3PSGreen() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PSGreen.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PS_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PSGreen_fg.svg")));
	}
};

struct Rogan3PSWhite : Rogan {
	Rogan3PSWhite() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PSWhite.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PS_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PSWhite_fg.svg")));
	}
};

struct Rogan3PBlue : Rogan {
	Rogan3PBlue() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PBlue.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3P_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PBlue_fg.svg")));
	}
};

struct Rogan3PRed : Rogan {
	Rogan3PRed() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PRed.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3P_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PRed_fg.svg")));
	}
};

struct Rogan3PGreen : Rogan {
	Rogan3PGreen() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PGreen.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3P_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PGreen_fg.svg")));
	}
};

struct Rogan3PWhite : Rogan {
	Rogan3PWhite() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PWhite.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3P_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PWhite_fg.svg")));
	}
};

struct Rogan2SGray : Rogan {
	Rogan2SGray() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2SGray.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2S_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2SGray_fg.svg")));
	}
};

struct Rogan2PSBlue : Rogan {
	Rogan2PSBlue() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PSBlue.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PS_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PSBlue_fg.svg")));
	}
};

struct Rogan2PSRed : Rogan {
	Rogan2PSRed() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PSRed.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PS_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PSRed_fg.svg")));
	}
};

struct Rogan2PSGreen : Rogan {
	Rogan2PSGreen() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PSGreen.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PS_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PSGreen_fg.svg")));
	}
};

struct Rogan2PSWhite : Rogan {
	Rogan2PSWhite() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PSWhite.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PS_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PSWhite_fg.svg")));
	}
};

struct Rogan2PBlue : Rogan {
	Rogan2PBlue() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PBlue.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2P_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PBlue_fg.svg")));
	}
};

struct Rogan2PRed : Rogan {
	Rogan2PRed() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PRed.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2P_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PRed_fg.svg")));
	}
};

struct Rogan2PGreen : Rogan {
	Rogan2PGreen() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PGreen.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2P_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PGreen_fg.svg")));
	}
};

struct Rogan2PWhite : Rogan {
	Rogan2PWhite() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PWhite.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2P_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PWhite_fg.svg")));
	}
};

struct Rogan1PSBlue : Rogan {
	Rogan1PSBlue() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSBlue.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PS_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSBlue_fg.svg")));
	}
};

struct Rogan1PSRed : Rogan {
	Rogan1PSRed() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSRed.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PS_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSRed_fg.svg")));
	}
};

struct Rogan1PSGreen : Rogan {
	Rogan1PSGreen() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSGreen.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PS_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSGreen_fg.svg")));
	}
};

struct Rogan1PSWhite : Rogan {
	Rogan1PSWhite() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSWhite.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PS_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSWhite_fg.svg")));
	}
};

struct Rogan1PBlue : Rogan {
	Rogan1PBlue() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PBlue.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1P_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PBlue_fg.svg")));
	}
};

struct Rogan1PRed : Rogan {
	Rogan1PRed() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PRed.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1P_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PRed_fg.svg")));
	}
};

struct Rogan1PGreen : Rogan {
	Rogan1PGreen() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PGreen.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1P_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PGreen_fg.svg")));
	}
};

struct Rogan1PWhite : Rogan {
	Rogan1PWhite() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PWhite.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1P_bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PWhite_fg.svg")));
	}
};


struct SynthTechAlco : app::SvgKnob {
	widget::SvgWidget* bg;

	SynthTechAlco() {
		minAngle = -0.82 * M_PI;
		maxAngle = 0.82 * M_PI;

		bg = new widget::SvgWidget;
		fb->addChildBelow(bg, tw);

		setSvg(Svg::load(asset::system("res/ComponentLibrary/SynthTechAlco.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/SynthTechAlco_bg.svg")));
	}
};

struct Trimpot : app::SvgKnob {
	widget::SvgWidget* bg;

	Trimpot() {
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;

		bg = new widget::SvgWidget;
		fb->addChildBelow(bg, tw);

		setSvg(Svg::load(asset::system("res/ComponentLibrary/Trimpot.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Trimpot_bg.svg")));
	}
};

struct BefacoBigKnob : app::SvgKnob {
	widget::SvgWidget* bg;

	BefacoBigKnob() {
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSvg(Svg::load(asset::system("res/ComponentLibrary/BefacoBigKnob.svg")));

		bg = new widget::SvgWidget;
		fb->addChildBelow(bg, tw);
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/BefacoBigKnob_bg.svg")));
	}
};

struct BefacoTinyKnob : app::SvgKnob {
	widget::SvgWidget* bg;

	BefacoTinyKnob() {
		minAngle = -0.8 * M_PI;
		maxAngle = 0.8 * M_PI;

		bg = new widget::SvgWidget;
		fb->addChildBelow(bg, tw);

		setSvg(Svg::load(asset::system("res/ComponentLibrary/BefacoTinyPointBlack.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/BefacoTinyKnobWhite_bg.svg")));
	}
};

struct BefacoSlidePot : app::SvgSlider {
	BefacoSlidePot() {
		math::Vec margin = math::Vec(3.5, 3.5);
		maxHandlePos = math::Vec(-1, -2).plus(margin);
		minHandlePos = math::Vec(-1, 87).plus(margin);
		setBackgroundSvg(Svg::load(asset::system("res/ComponentLibrary/BefacoSlidePot.svg")));
		setHandleSvg(Svg::load(asset::system("res/ComponentLibrary/BefacoSlidePotHandle.svg")));
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));
	}
};

struct VCVSlider : app::SvgSlider {
	VCVSlider() {
		setBackgroundSvg(Svg::load(asset::system("res/ComponentLibrary/VCVSlider.svg")));
		setHandleSvg(Svg::load(asset::system("res/ComponentLibrary/VCVSliderHandle.svg")));
		setHandlePosCentered(
			math::Vec(19.84260/2, 76.53517 - 11.74218/2),
			math::Vec(19.84260/2, 0.0 + 11.74218/2)
		);
	}
};
using LEDSlider = VCVSlider;

struct VCVSliderHorizontal : app::SvgSlider {
	VCVSliderHorizontal() {
		horizontal = true;
		// TODO Fix positions
		maxHandlePos = mm2px(math::Vec(22.078, 0.738).plus(math::Vec(0, 2)));
		minHandlePos = mm2px(math::Vec(0.738, 0.738).plus(math::Vec(0, 2)));
		// TODO Fix SVG
		setBackgroundSvg(Svg::load(asset::system("res/ComponentLibrary/VCVSliderHorizontal.svg")));
	}
};
using LEDSliderHorizontal = VCVSliderHorizontal;

/** An SvgSlider with an attached light.
Construct with createLightParamCentered() helper function.
*/
template <typename TBase, typename TLightBase = RedLight>
struct LightSlider : TBase {
	app::ModuleLightWidget* light;

	LightSlider() {
		light = new TLightBase;
		this->addChild(light);
	}

	app::ModuleLightWidget* getLight() {
		return light;
	}

	void step() override {
		TBase::step();
		// Move center of light to center of handle
		light->box.pos = this->handle->box.pos
			.plus(this->handle->box.size.div(2))
			.minus(light->box.size.div(2));
	}
};

template <typename TBase>
struct VCVSliderLight : RectangleLight<TSvgLight<TBase>> {
	VCVSliderLight() {
		this->setSvg(Svg::load(asset::system("res/ComponentLibrary/VCVSliderLight.svg")));
	}
};
template <typename TBase>
using LEDSliderLight = VCVSliderLight<TBase>;

template <typename TLightBase = RedLight>
struct VCVLightSlider : LightSlider<VCVSlider, VCVSliderLight<TLightBase>> {
	VCVLightSlider() {}
};
template <typename TLightBase = RedLight>
using LEDLightSlider = VCVLightSlider<TLightBase>;

/** Deprecated. Use VCVSliderLight with your preferred LightWidget. */
struct LEDSliderGreen : VCVLightSlider<GreenLight> {};
struct LEDSliderRed : VCVLightSlider<RedLight> {};
struct LEDSliderYellow : VCVLightSlider<YellowLight> {};
struct LEDSliderBlue : VCVLightSlider<BlueLight> {};
struct LEDSliderWhite : VCVLightSlider<WhiteLight> {};

template <typename TLightBase = RedLight>
struct VCVLightSliderHorizontal : LightSlider<VCVSliderHorizontal, TLightBase> {
	VCVLightSliderHorizontal() {
		// TODO Fix positions
		this->light->box.size = mm2px(math::Vec(3.276, 1.524));
		// TODO Fix SVG
		this->setHandleSvg(Svg::load(asset::system("res/ComponentLibrary/VCVSliderHorizontalHandle.svg")));
	}
};
template <typename TLightBase = RedLight>
using LEDLightSliderHorizontal = VCVLightSliderHorizontal<TLightBase>;


////////////////////
// Ports
////////////////////

struct PJ301MPort : app::SvgPort {
	PJ301MPort() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/PJ301M.svg")));
	}
};

struct PJ3410Port : app::SvgPort {
	PJ3410Port() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/PJ3410.svg")));
	}
};

struct CL1362Port : app::SvgPort {
	CL1362Port() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/CL1362.svg")));
	}
};


////////////////////
// Switches
////////////////////

template <typename TSwitch>
struct MomentarySwitch : TSwitch {
	MomentarySwitch() {
		this->momentary = true;
	}
};

struct NKK : app::SvgSwitch {
	NKK() {
		shadow->opacity = 0.0;
		addFrame(Svg::load(asset::system("res/ComponentLibrary/NKK_0.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/NKK_1.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/NKK_2.svg")));
	}
};

struct CKSS : app::SvgSwitch {
	CKSS() {
		shadow->opacity = 0.0;
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSS_0.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSS_1.svg")));
	}
};

struct CKSSThree : app::SvgSwitch {
	CKSSThree() {
		shadow->opacity = 0.0;
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSSThree_0.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSSThree_1.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSSThree_2.svg")));
	}
};

struct CKSSThreeHorizontal : app::SvgSwitch {
	CKSSThreeHorizontal() {
		shadow->opacity = 0.0;
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSSThreeHorizontal_0.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSSThreeHorizontal_1.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSSThreeHorizontal_2.svg")));
	}
};

struct CKD6 : app::SvgSwitch {
	CKD6() {
		momentary = true;
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKD6_0.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKD6_1.svg")));
	}
};

struct TL1105 : app::SvgSwitch {
	TL1105() {
		momentary = true;
		addFrame(Svg::load(asset::system("res/ComponentLibrary/TL1105_0.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/TL1105_1.svg")));
	}
};

struct VCVButton : app::SvgSwitch {
	VCVButton() {
		momentary = true;
		addFrame(Svg::load(asset::system("res/ComponentLibrary/VCVButton_0.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/VCVButton_1.svg")));
	}
};
using LEDButton = VCVButton;

struct VCVLatch : VCVButton {
	VCVLatch() {
		momentary = false;
		latch = true;
	}
};

/** Looks best with MediumSimpleLight<WhiteLight> or a color of your choice.
*/
template <typename TLight>
struct VCVLightButton : VCVButton {
	app::ModuleLightWidget* light;

	VCVLightButton() {
		light = new TLight;
		// Move center of light to center of box
		light->box.pos = box.size.div(2).minus(light->box.size.div(2));
		addChild(light);
	}

	app::ModuleLightWidget* getLight() {
		return light;
	}
};
template <typename TLight>
using LEDLightButton = VCVLightButton<TLight>;

template <typename TLight>
struct VCVLightLatch : VCVLightButton<TLight> {
	VCVLightLatch() {
		this->momentary = false;
		this->latch = true;
	}
};

struct BefacoSwitch : app::SvgSwitch {
	BefacoSwitch() {
		addFrame(Svg::load(asset::system("res/ComponentLibrary/BefacoSwitch_0.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/BefacoSwitch_1.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/BefacoSwitch_2.svg")));
	}
};

struct BefacoPush : app::SvgSwitch {
	BefacoPush() {
		momentary = true;
		addFrame(Svg::load(asset::system("res/ComponentLibrary/BefacoPush_0.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/BefacoPush_1.svg")));
	}
};

struct VCVBezel : app::SvgSwitch {
	VCVBezel() {
		momentary = true;
		addFrame(Svg::load(asset::system("res/ComponentLibrary/VCVBezel.svg")));
	}
};
using LEDBezel = VCVBezel;

struct VCVBezelLatch : VCVBezel {
	VCVBezelLatch() {
		momentary = false;
		latch = true;
	}
};

template <typename TLightBase = WhiteLight>
struct VCVLightBezel : VCVBezel {
	app::ModuleLightWidget* light;

	VCVLightBezel() {
		light = new VCVBezelLight<TLightBase>;
		// Move center of light to center of box
		light->box.pos = box.size.div(2).minus(light->box.size.div(2));
		addChild(light);
	}

	app::ModuleLightWidget* getLight() {
		return light;
	}
};
template <typename TLightBase = WhiteLight>
using LEDLightBezel = VCVLightBezel<TLightBase>;

template <typename TLightBase = WhiteLight>
struct VCVLightBezelLatch : VCVLightBezel<TLightBase> {
	VCVLightBezelLatch() {
		this->momentary = false;
		this->latch = true;
	}
};

struct PB61303 : app::SvgSwitch {
	PB61303() {
		momentary = true;
		addFrame(Svg::load(asset::system("res/ComponentLibrary/PB61303.svg")));
	}
};

////////////////////
// Misc
////////////////////

struct ScrewSilver : app::SvgScrew {
	ScrewSilver() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/ScrewSilver.svg")));
	}
};

struct ScrewBlack : app::SvgScrew {
	ScrewBlack() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/ScrewBlack.svg")));
	}
};

struct SegmentDisplay : widget::Widget {
	int lightsLen = 0;
	bool vertical = false;
	float margin = mm2px(0.5);

	void draw(const DrawArgs& args) override {
		// Background
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(args.vg, color::BLACK);
		nvgFill(args.vg);
		Widget::draw(args);
	}

	template <typename TLightBase = WhiteLight>
	void setLights(engine::Module* module, int firstLightId, int lightsLen) {
		clearChildren();
		this->lightsLen = lightsLen;
		float r = (vertical ? box.size.y : box.size.x) - margin;
		for (int i = 0; i < lightsLen; i++) {
			float p = float(i) / lightsLen;
			app::ModuleLightWidget* light = new RectangleLight<TLightBase>;
			if (vertical) {
				light->box.pos.y = p * r + margin;
				light->box.size.y = r / lightsLen - margin;
				light->box.size.x = box.size.x;
			}
			else {
				light->box.pos.x = p * r + margin;
				light->box.size.x = r / lightsLen - margin;
				light->box.size.y = box.size.y;
			}
			light->module = module;
			light->firstLightId = firstLightId;
			firstLightId += light->baseColors.size();
			addChild(light);
		}
	}
};


struct AudioButton_ADAT : app::AudioButton {
	AudioButton_ADAT() {
		addFrame(Svg::load(asset::system("res/ComponentLibrary/ADAT.svg")));
		shadow->opacity = 0.0;
	}
};


struct AudioButton_USB_B : app::AudioButton {
	AudioButton_USB_B() {
		addFrame(Svg::load(asset::system("res/ComponentLibrary/USB_B.svg")));
		shadow->opacity = 0.0;
	}
};


struct MidiButton_MIDI_DIN : app::MidiButton {
	MidiButton_MIDI_DIN() {
		addFrame(Svg::load(asset::system("res/ComponentLibrary/MIDI_DIN.svg")));
		shadow->opacity = 0.0;
	}
};


} // namespace componentlibrary
} // namespace rack
