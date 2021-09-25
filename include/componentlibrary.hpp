#pragma once
#include <widget/FramebufferWidget.hpp>
#include <widget/SvgWidget.hpp>
#include <app/SvgKnob.hpp>
#include <app/SvgSlider.hpp>
#include <app/SvgPort.hpp>
#include <app/ModuleLightWidget.hpp>
#include <app/SvgSwitch.hpp>
#include <app/SvgScrew.hpp>
#include <app/AudioWidget.hpp>
#include <app/MidiWidget.hpp>
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
static const NVGcolor SCHEME_YELLOW = nvgRGB(0xf9, 0xdf, 0x1c);
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

	createLightParamCentered<LEDLightSlider<GreenLight>>(...)
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
typedef TSvgLight<> SvgLight;

template <typename TBase = app::ModuleLightWidget>
struct TGrayModuleLightWidget : TBase {
	TGrayModuleLightWidget() {
		this->bgColor = nvgRGBA(0x33, 0x33, 0x33, 0xff);
		this->borderColor = nvgRGBA(0, 0, 0, 53);
	}
};
typedef TGrayModuleLightWidget<> GrayModuleLightWidget;

template <typename TBase = GrayModuleLightWidget>
struct TRedLight : TBase {
	TRedLight() {
		this->addBaseColor(SCHEME_RED);
	}
};
typedef TRedLight<> RedLight;

template <typename TBase = GrayModuleLightWidget>
struct TGreenLight : TBase {
	TGreenLight() {
		this->addBaseColor(SCHEME_GREEN);
	}
};
typedef TGreenLight<> GreenLight;

template <typename TBase = GrayModuleLightWidget>
struct TYellowLight : TBase {
	TYellowLight() {
		this->addBaseColor(SCHEME_YELLOW);
	}
};
typedef TYellowLight<> YellowLight;

template <typename TBase = GrayModuleLightWidget>
struct TBlueLight : TBase {
	TBlueLight() {
		this->addBaseColor(SCHEME_BLUE);
	}
};
typedef TBlueLight<> BlueLight;

template <typename TBase = GrayModuleLightWidget>
struct TWhiteLight : TBase {
	TWhiteLight() {
		this->addBaseColor(SCHEME_WHITE);
	}
};
typedef TWhiteLight<> WhiteLight;

/** Reads two adjacent lightIds, so `lightId` and `lightId + 1` must be defined */
template <typename TBase = GrayModuleLightWidget>
struct TGreenRedLight : TBase {
	TGreenRedLight() {
		this->addBaseColor(SCHEME_GREEN);
		this->addBaseColor(SCHEME_RED);
	}
};
typedef TGreenRedLight<> GreenRedLight;

template <typename TBase = GrayModuleLightWidget>
struct TRedGreenBlueLight : TBase {
	TRedGreenBlueLight() {
		this->addBaseColor(SCHEME_RED);
		this->addBaseColor(SCHEME_GREEN);
		this->addBaseColor(SCHEME_BLUE);
	}
};
typedef TRedGreenBlueLight<> RedGreenBlueLight;

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

/** A light for displaying on top of PB61303. Must add a color by subclassing or templating. */
template <typename TBase>
struct LEDBezelLight : TBase {
	LEDBezelLight() {
		this->borderColor = color::BLACK_TRANSPARENT;
		this->bgColor = color::BLACK_TRANSPARENT;
		this->box.size = math::Vec(17.545, 17.545);
	}
};

/** A light to displayed over PB61303. Must add a color by subclassing or templating.
Don't add this as a child of the PB61303 itself. Instead, just place it over it as a sibling in the scene graph, offset by mm2px(math::Vec(0.5, 0.5)).
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
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundBlackKnob-bg.svg")));
	}
};

struct RoundSmallBlackKnob : RoundKnob {
	RoundSmallBlackKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundSmallBlackKnob.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundSmallBlackKnob-bg.svg")));
	}
};

struct RoundLargeBlackKnob : RoundKnob {
	RoundLargeBlackKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundLargeBlackKnob.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundLargeBlackKnob-bg.svg")));
	}
};

struct RoundBigBlackKnob : RoundKnob {
	RoundBigBlackKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundBigBlackKnob.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundBigBlackKnob-bg.svg")));
	}
};

struct RoundHugeBlackKnob : RoundKnob {
	RoundHugeBlackKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundHugeBlackKnob.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/RoundHugeBlackKnob-bg.svg")));
	}
};

struct RoundBlackSnapKnob : RoundBlackKnob {
	RoundBlackSnapKnob() {
		snap = true;
	}
};


struct Davies1900hKnob : app::SvgKnob {
	Davies1900hKnob() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
	}
};

struct Davies1900hWhiteKnob : Davies1900hKnob {
	Davies1900hWhiteKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hWhite.svg")));
	}
};

struct Davies1900hBlackKnob : Davies1900hKnob {
	Davies1900hBlackKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hBlack.svg")));
	}
};

struct Davies1900hRedKnob : Davies1900hKnob {
	Davies1900hRedKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hRed.svg")));
	}
};

struct Davies1900hLargeWhiteKnob : Davies1900hKnob {
	Davies1900hLargeWhiteKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hLargeWhite.svg")));
	}
};

struct Davies1900hLargeBlackKnob : Davies1900hKnob {
	Davies1900hLargeBlackKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hLargeBlack.svg")));
	}
};

struct Davies1900hLargeRedKnob : Davies1900hKnob {
	Davies1900hLargeRedKnob() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Davies1900hLargeRed.svg")));
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
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan6PS-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan6PSWhite-fg.svg")));
	}
};

struct Rogan5PSGray : Rogan {
	Rogan5PSGray() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan5PSGray.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan5PS-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan5PSGray-fg.svg")));
	}
};

struct Rogan3PSBlue : Rogan {
	Rogan3PSBlue() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PSBlue.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PS-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PSBlue-fg.svg")));
	}
};

struct Rogan3PSRed : Rogan {
	Rogan3PSRed() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PSRed.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PS-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PSRed-fg.svg")));
	}
};

struct Rogan3PSGreen : Rogan {
	Rogan3PSGreen() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PSGreen.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PS-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PSGreen-fg.svg")));
	}
};

struct Rogan3PSWhite : Rogan {
	Rogan3PSWhite() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PSWhite.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PS-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PSWhite-fg.svg")));
	}
};

struct Rogan3PBlue : Rogan {
	Rogan3PBlue() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PBlue.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3P-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PBlue-fg.svg")));
	}
};

struct Rogan3PRed : Rogan {
	Rogan3PRed() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PRed.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3P-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PRed-fg.svg")));
	}
};

struct Rogan3PGreen : Rogan {
	Rogan3PGreen() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PGreen.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3P-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PGreen-fg.svg")));
	}
};

struct Rogan3PWhite : Rogan {
	Rogan3PWhite() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PWhite.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3P-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan3PWhite-fg.svg")));
	}
};

struct Rogan2SGray : Rogan {
	Rogan2SGray() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2SGray.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2S-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2SGray-fg.svg")));
	}
};

struct Rogan2PSBlue : Rogan {
	Rogan2PSBlue() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PSBlue.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PS-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PSBlue-fg.svg")));
	}
};

struct Rogan2PSRed : Rogan {
	Rogan2PSRed() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PSRed.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PS-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PSRed-fg.svg")));
	}
};

struct Rogan2PSGreen : Rogan {
	Rogan2PSGreen() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PSGreen.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PS-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PSGreen-fg.svg")));
	}
};

struct Rogan2PSWhite : Rogan {
	Rogan2PSWhite() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PSWhite.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PS-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PSWhite-fg.svg")));
	}
};

struct Rogan2PBlue : Rogan {
	Rogan2PBlue() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PBlue.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2P-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PBlue-fg.svg")));
	}
};

struct Rogan2PRed : Rogan {
	Rogan2PRed() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PRed.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2P-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PRed-fg.svg")));
	}
};

struct Rogan2PGreen : Rogan {
	Rogan2PGreen() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PGreen.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2P-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PGreen-fg.svg")));
	}
};

struct Rogan2PWhite : Rogan {
	Rogan2PWhite() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PWhite.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2P-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan2PWhite-fg.svg")));
	}
};

struct Rogan1PSBlue : Rogan {
	Rogan1PSBlue() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSBlue.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PS-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSBlue-fg.svg")));
	}
};

struct Rogan1PSRed : Rogan {
	Rogan1PSRed() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSRed.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PS-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSRed-fg.svg")));
	}
};

struct Rogan1PSGreen : Rogan {
	Rogan1PSGreen() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSGreen.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PS-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSGreen-fg.svg")));
	}
};

struct Rogan1PSWhite : Rogan {
	Rogan1PSWhite() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSWhite.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PS-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSWhite-fg.svg")));
	}
};

struct Rogan1PBlue : Rogan {
	Rogan1PBlue() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PBlue.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1P-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PBlue-fg.svg")));
	}
};

struct Rogan1PRed : Rogan {
	Rogan1PRed() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PRed.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1P-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PRed-fg.svg")));
	}
};

struct Rogan1PGreen : Rogan {
	Rogan1PGreen() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PGreen.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1P-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PGreen-fg.svg")));
	}
};

struct Rogan1PWhite : Rogan {
	Rogan1PWhite() {
		setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PWhite.svg")));
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1P-bg.svg")));
		fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PWhite-fg.svg")));
	}
};


struct SynthTechAlco : app::SvgKnob {
	SynthTechAlco() {
		minAngle = -0.82 * M_PI;
		maxAngle = 0.82 * M_PI;
		setSvg(Svg::load(asset::system("res/ComponentLibrary/SynthTechAlco.svg")));
		// Add cap
		widget::FramebufferWidget* capFb = new widget::FramebufferWidget;
		widget::SvgWidget* cap = new widget::SvgWidget;
		cap->setSvg(Svg::load(asset::system("res/ComponentLibrary/SynthTechAlco_cap.svg")));
		capFb->addChild(cap);
		addChild(capFb);
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
		bg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Trimpot-bg.svg")));
	}
};

struct BefacoBigKnob : app::SvgKnob {
	BefacoBigKnob() {
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSvg(Svg::load(asset::system("res/ComponentLibrary/BefacoBigKnob.svg")));
	}
};

struct BefacoBigSnapKnob : BefacoBigKnob {
	BefacoBigSnapKnob() {
		snap = true;
	}
};

struct BefacoTinyKnob : app::SvgKnob {
	BefacoTinyKnob() {
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSvg(Svg::load(asset::system("res/ComponentLibrary/BefacoTinyKnob.svg")));
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

struct LEDSlider : app::SvgSlider {
	LEDSlider() {
		setBackgroundSvg(Svg::load(asset::system("res/ComponentLibrary/LEDSlider.svg")));
		setHandleSvg(Svg::load(asset::system("res/ComponentLibrary/LEDSliderHandle.svg")));
		setHandlePosCentered(
			math::Vec(19.84260/2, 76.53517 - 11.74218/2),
			math::Vec(19.84260/2, 0.0 + 11.74218/2)
		);
	}
};

// TODO Modernize
struct LEDSliderHorizontal : app::SvgSlider {
	LEDSliderHorizontal() {
		horizontal = true;
		maxHandlePos = mm2px(math::Vec(22.078, 0.738).plus(math::Vec(0, 2)));
		minHandlePos = mm2px(math::Vec(0.738, 0.738).plus(math::Vec(0, 2)));
		setBackgroundSvg(Svg::load(asset::system("res/ComponentLibrary/LEDSliderHorizontal.svg")));
	}
};

template <typename TBase, typename TLightBase = RedLight>
struct LightSlider : TBase {
	app::ModuleLightWidget* light;

	LightSlider() {
		light = new TLightBase;
		this->addChild(light);
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
struct LEDSliderLight : RectangleLight<TSvgLight<TBase>> {
	LEDSliderLight() {
		this->setSvg(Svg::load(asset::system("res/ComponentLibrary/LEDSliderLight.svg")));
	}
};

template <typename TLightBase = RedLight>
struct LEDLightSlider : LightSlider<LEDSlider, LEDSliderLight<TLightBase>> {
	LEDLightSlider() {}
};

/** Deprecated. Use LEDSliderLight with your preferred LightWidget. */
struct LEDSliderGreen : LEDLightSlider<GreenLight> {};
struct LEDSliderRed : LEDLightSlider<RedLight> {};
struct LEDSliderYellow : LEDLightSlider<YellowLight> {};
struct LEDSliderBlue : LEDLightSlider<BlueLight> {};
struct LEDSliderWhite : LEDLightSlider<WhiteLight> {};

// TODO Modernize
template <typename TLightBase = RedLight>
struct LEDLightSliderHorizontal : LightSlider<LEDSliderHorizontal, TLightBase> {
	LEDLightSliderHorizontal() {
		this->setHandleSvg(Svg::load(asset::system("res/ComponentLibrary/LEDSliderHorizontalHandle.svg")));
		this->light->box.size = mm2px(math::Vec(3.276, 1.524));
	}
};


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
struct LatchingSwitch : TSwitch {
	LatchingSwitch() {
		this->momentary = false;
	}
};

template <typename TSwitch>
struct MomentarySwitch : TSwitch {
	MomentarySwitch() {
		this->momentary = true;
	}
};

struct NKK : app::SvgSwitch {
	NKK() {
		addFrame(Svg::load(asset::system("res/ComponentLibrary/NKK_0.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/NKK_1.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/NKK_2.svg")));
	}
};

struct CKSS : app::SvgSwitch {
	CKSS() {
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSS_0.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSS_1.svg")));
	}
};

struct CKSSThree : app::SvgSwitch {
	CKSSThree() {
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSSThree_0.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSSThree_1.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSSThree_2.svg")));
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

struct LEDButton : app::SvgSwitch {
	LEDButton() {
		momentary = true;
		addFrame(Svg::load(asset::system("res/ComponentLibrary/LEDButton.svg")));
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

struct LEDBezel : app::SvgSwitch {
	LEDBezel() {
		momentary = true;
		addFrame(Svg::load(asset::system("res/ComponentLibrary/LEDBezel.svg")));
	}
};

template <typename TLightBase = WhiteLight>
struct LEDLightBezel : LEDBezel {
	app::ModuleLightWidget* light;

	LEDLightBezel() {
		light = new LEDBezelLight<TLightBase>;
		// Move center of light to center of box
		light->box.pos = box.size.div(2).minus(light->box.size.div(2));
		addChild(light);
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
		addFrame(Svg::load(asset::system("res/ComponentLibrary/USB-B.svg")));
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
