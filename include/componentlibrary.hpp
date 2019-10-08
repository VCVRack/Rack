#pragma once
#include <app/SvgKnob.hpp>
#include <app/SvgSlider.hpp>
#include <app/SvgPort.hpp>
#include <app/ModuleLightWidget.hpp>
#include <app/SvgSwitch.hpp>
#include <app/SvgScrew.hpp>
#include <asset.hpp>


namespace rack {

/** Component Library by [Grayscale](https://grayscale.info/).

Copied from `LICENSE.md`:

The **Component Library graphics** in the `res/ComponentLibrary` directory are copyright © 2019 [Grayscale](http://grayscale.info/) and licensed under [CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/).
You may not freely sell plugins using Component Library graphics.
However, a free commercial license is available for plugins sold through the [VCV Store](https://vcvrack.com/plugins.html).
Email contact@vcvrack.com for more information about licensing or the VCV Store.
*/
namespace componentlibrary {


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
struct TGrayModuleLightWidget : TBase {
	TGrayModuleLightWidget() {
		this->bgColor = nvgRGB(0x5a, 0x5a, 0x5a);
		this->borderColor = nvgRGBA(0, 0, 0, 0x60);
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
struct LargeLight : TBase {
	LargeLight() {
		this->box.size = app::mm2px(math::Vec(5.179, 5.179));
	}
};

/** Based on the size of 3mm LEDs */
template <typename TBase>
struct MediumLight : TBase {
	MediumLight() {
		this->box.size = app::mm2px(math::Vec(3.176, 3.176));
	}
};

/** Based on the size of 2mm LEDs */
template <typename TBase>
struct SmallLight : TBase {
	SmallLight() {
		this->box.size = app::mm2px(math::Vec(2.176, 2.176));
	}
};

/** Based on the size of 1mm LEDs */
template <typename TBase>
struct TinyLight : TBase {
	TinyLight() {
		this->box.size = app::mm2px(math::Vec(1.088, 1.088));
	}
};

template <typename TBase>
struct RectangleLight : TBase {
	void drawLight(const widget::Widget::DrawArgs& args) override {
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, this->box.size.x, this->box.size.y);

		// Background
		if (this->bgColor.a > 0.0) {
			nvgFillColor(args.vg, this->bgColor);
			nvgFill(args.vg);
		}

		// Foreground
		if (this->color.a > 0.0) {
			nvgFillColor(args.vg, this->color);
			nvgFill(args.vg);
		}

		// Border
		if (this->borderColor.a > 0.0) {
			nvgStrokeWidth(args.vg, 0.5);
			nvgStrokeColor(args.vg, this->borderColor);
			nvgStroke(args.vg);
		}
	}
};

/** A light for displaying on top of PB61303. Must add a color by subclassing or templating. */
template <typename TBase>
struct LEDBezelLight : TBase {
	LEDBezelLight() {
		this->bgColor = color::BLACK_TRANSPARENT;
		this->box.size = app::mm2px(math::Vec(6.0, 6.0));
	}
};

/** A light to displayed over PB61303. Must add a color by subclassing or templating.
Don't add this as a child of the PB61303 itself. Instead, just place it over it as a sibling in the scene graph, offset by app::mm2px(math::Vec(0.5, 0.5)).
*/
template <typename TBase>
struct PB61303Light : TBase {
	PB61303Light() {
		this->bgColor = color::BLACK_TRANSPARENT;
		this->box.size = app::mm2px(math::Vec(9.0, 9.0));
	}
};


////////////////////
// Knobs
////////////////////

struct RoundKnob : app::SvgKnob {
	RoundKnob() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
	}
};

struct RoundBlackKnob : RoundKnob {
	RoundBlackKnob() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/RoundBlackKnob.svg")));
	}
};

struct RoundSmallBlackKnob : RoundKnob {
	RoundSmallBlackKnob() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/RoundSmallBlackKnob.svg")));
	}
};

struct RoundLargeBlackKnob : RoundKnob {
	RoundLargeBlackKnob() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/RoundLargeBlackKnob.svg")));
	}
};

struct RoundHugeBlackKnob : RoundKnob {
	RoundHugeBlackKnob() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/RoundHugeBlackKnob.svg")));
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
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Davies1900hWhite.svg")));
	}
};

struct Davies1900hBlackKnob : Davies1900hKnob {
	Davies1900hBlackKnob() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Davies1900hBlack.svg")));
	}
};

struct Davies1900hRedKnob : Davies1900hKnob {
	Davies1900hRedKnob() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Davies1900hRed.svg")));
	}
};

struct Davies1900hLargeWhiteKnob : Davies1900hKnob {
	Davies1900hLargeWhiteKnob() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Davies1900hLargeWhite.svg")));
	}
};

struct Davies1900hLargeBlackKnob : Davies1900hKnob {
	Davies1900hLargeBlackKnob() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Davies1900hLargeBlack.svg")));
	}
};

struct Davies1900hLargeRedKnob : Davies1900hKnob {
	Davies1900hLargeRedKnob() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Davies1900hLargeRed.svg")));
	}
};


struct Rogan : app::SvgKnob {
	Rogan() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
	}
};

struct Rogan6PSWhite : Rogan {
	Rogan6PSWhite() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan6PSWhite.svg")));
	}
};

struct Rogan5PSGray : Rogan {
	Rogan5PSGray() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan5PSGray.svg")));
	}
};

struct Rogan3PSBlue : Rogan {
	Rogan3PSBlue() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan3PSBlue.svg")));
	}
};

struct Rogan3PSRed : Rogan {
	Rogan3PSRed() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan3PSRed.svg")));
	}
};

struct Rogan3PSGreen : Rogan {
	Rogan3PSGreen() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan3PSGreen.svg")));
	}
};

struct Rogan3PSWhite : Rogan {
	Rogan3PSWhite() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan3PSWhite.svg")));
	}
};

struct Rogan3PBlue : Rogan {
	Rogan3PBlue() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan3PBlue.svg")));
	}
};

struct Rogan3PRed : Rogan {
	Rogan3PRed() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan3PRed.svg")));
	}
};

struct Rogan3PGreen : Rogan {
	Rogan3PGreen() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan3PGreen.svg")));
	}
};

struct Rogan3PWhite : Rogan {
	Rogan3PWhite() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan3PWhite.svg")));
	}
};

struct Rogan2SGray : Rogan {
	Rogan2SGray() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan2SGray.svg")));
	}
};

struct Rogan2PSBlue : Rogan {
	Rogan2PSBlue() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan2PSBlue.svg")));
	}
};

struct Rogan2PSRed : Rogan {
	Rogan2PSRed() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan2PSRed.svg")));
	}
};

struct Rogan2PSGreen : Rogan {
	Rogan2PSGreen() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan2PSGreen.svg")));
	}
};

struct Rogan2PSWhite : Rogan {
	Rogan2PSWhite() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan2PSWhite.svg")));
	}
};

struct Rogan2PBlue : Rogan {
	Rogan2PBlue() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan2PBlue.svg")));
	}
};

struct Rogan2PRed : Rogan {
	Rogan2PRed() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan2PRed.svg")));
	}
};

struct Rogan2PGreen : Rogan {
	Rogan2PGreen() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan2PGreen.svg")));
	}
};

struct Rogan2PWhite : Rogan {
	Rogan2PWhite() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan2PWhite.svg")));
	}
};

struct Rogan1PSBlue : Rogan {
	Rogan1PSBlue() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan1PSBlue.svg")));
	}
};

struct Rogan1PSRed : Rogan {
	Rogan1PSRed() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan1PSRed.svg")));
	}
};

struct Rogan1PSGreen : Rogan {
	Rogan1PSGreen() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan1PSGreen.svg")));
	}
};

struct Rogan1PSWhite : Rogan {
	Rogan1PSWhite() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan1PSWhite.svg")));
	}
};

struct Rogan1PBlue : Rogan {
	Rogan1PBlue() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan1PBlue.svg")));
	}
};

struct Rogan1PRed : Rogan {
	Rogan1PRed() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan1PRed.svg")));
	}
};

struct Rogan1PGreen : Rogan {
	Rogan1PGreen() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan1PGreen.svg")));
	}
};

struct Rogan1PWhite : Rogan {
	Rogan1PWhite() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Rogan1PWhite.svg")));
	}
};


struct SynthTechAlco : app::SvgKnob {
	SynthTechAlco() {
		minAngle = -0.82 * M_PI;
		maxAngle = 0.82 * M_PI;
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/SynthTechAlco.svg")));
		// Add cap
		widget::FramebufferWidget* capFb = new widget::FramebufferWidget;
		widget::SvgWidget* cap = new widget::SvgWidget;
		cap->setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/SynthTechAlco_cap.svg")));
		capFb->addChild(cap);
		addChild(capFb);
	}
};

struct Trimpot : app::SvgKnob {
	Trimpot() {
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/Trimpot.svg")));
	}
};

struct BefacoBigKnob : app::SvgKnob {
	BefacoBigKnob() {
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/BefacoBigKnob.svg")));
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
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/BefacoTinyKnob.svg")));
	}
};

struct BefacoSlidePot : app::SvgSlider {
	BefacoSlidePot() {
		math::Vec margin = math::Vec(3.5, 3.5);
		maxHandlePos = math::Vec(-1, -2).plus(margin);
		minHandlePos = math::Vec(-1, 87).plus(margin);
		setBackgroundSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/BefacoSlidePot.svg")));
		setHandleSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/BefacoSlidePotHandle.svg")));
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));
	}
};

struct LEDSlider : app::SvgSlider {
	LEDSlider() {
		maxHandlePos = app::mm2px(math::Vec(0.738, 0.738).plus(math::Vec(2, 0)));
		minHandlePos = app::mm2px(math::Vec(0.738, 22.078).plus(math::Vec(2, 0)));
		setBackgroundSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/LEDSlider.svg")));
	}
};

/** API is unstable for LEDSlider. Will add a LightWidget later. */
struct LEDSliderGreen : LEDSlider {
	LEDSliderGreen() {
		setHandleSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/LEDSliderGreenHandle.svg")));
	}
};

struct LEDSliderRed : LEDSlider {
	LEDSliderRed() {
		setHandleSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/LEDSliderRedHandle.svg")));
	}
};

struct LEDSliderYellow : LEDSlider {
	LEDSliderYellow() {
		setHandleSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/LEDSliderYellowHandle.svg")));
	}
};

struct LEDSliderBlue : LEDSlider {
	LEDSliderBlue() {
		setHandleSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/LEDSliderBlueHandle.svg")));
	}
};

struct LEDSliderWhite : LEDSlider {
	LEDSliderWhite() {
		setHandleSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/LEDSliderWhiteHandle.svg")));
	}
};

struct LEDSliderHorizontal : app::SvgSlider {
	LEDSliderHorizontal() {
		horizontal = true;
		maxHandlePos = app::mm2px(math::Vec(22.078, 0.738).plus(math::Vec(0, 2)));
		minHandlePos = app::mm2px(math::Vec(0.738, 0.738).plus(math::Vec(0, 2)));
		setBackgroundSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/LEDSliderHorizontal.svg")));
	}
};

template <typename TBase, typename TLightBase = RedLight>
struct LightSlider : TBase {
	app::ModuleLightWidget* light;

	LightSlider() {
		light = new RectangleLight<TLightBase>;
		this->addChild(light);
	}

	void setFirstLightId(int firstLightId) {
		if (this->paramQuantity)
			light->module = this->paramQuantity->module;
		light->firstLightId = firstLightId;
	}

	void step() override {
		TBase::step();
		// Move center of light to center of handle
		light->box.pos = this->handle->box.pos
		                 .plus(this->handle->box.size.div(2))
		                 .minus(light->box.size.div(2));
	}
};

template <typename TLightBase = RedLight>
struct LEDLightSlider : LightSlider<LEDSlider, TLightBase> {
	LEDLightSlider() {
		this->setHandleSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/LEDSliderHandle.svg")));
		this->light->box.size = app::mm2px(math::Vec(1.524, 3.276));
	}
};

template <typename TLightBase = RedLight>
struct LEDLightSliderHorizontal : LightSlider<LEDSliderHorizontal, TLightBase> {
	LEDLightSliderHorizontal() {
		this->setHandleSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/LEDSliderHorizontalHandle.svg")));
		this->light->box.size = app::mm2px(math::Vec(3.276, 1.524));
	}
};


////////////////////
// Ports
////////////////////

struct PJ301MPort : app::SvgPort {
	PJ301MPort() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/PJ301M.svg")));
	}
};

struct PJ3410Port : app::SvgPort {
	PJ3410Port() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/PJ3410.svg")));
	}
};

struct CL1362Port : app::SvgPort {
	CL1362Port() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/CL1362.svg")));
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
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/NKK_0.svg")));
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/NKK_1.svg")));
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/NKK_2.svg")));
	}
};

struct CKSS : app::SvgSwitch {
	CKSS() {
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/CKSS_0.svg")));
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/CKSS_1.svg")));
	}
};

struct CKSSThree : app::SvgSwitch {
	CKSSThree() {
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/CKSSThree_0.svg")));
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/CKSSThree_1.svg")));
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/CKSSThree_2.svg")));
	}
};

struct CKD6 : app::SvgSwitch {
	CKD6() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/CKD6_0.svg")));
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/CKD6_1.svg")));
	}
};

struct TL1105 : app::SvgSwitch {
	TL1105() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/TL1105_0.svg")));
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/TL1105_1.svg")));
	}
};

struct LEDButton : app::SvgSwitch {
	LEDButton() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/LEDButton.svg")));
	}
};

struct BefacoSwitch : app::SvgSwitch {
	BefacoSwitch() {
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/BefacoSwitch_0.svg")));
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/BefacoSwitch_1.svg")));
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/BefacoSwitch_2.svg")));
	}
};

struct BefacoPush : app::SvgSwitch {
	BefacoPush() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/BefacoPush_0.svg")));
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/BefacoPush_1.svg")));
	}
};

struct LEDBezel : app::SvgSwitch {
	LEDBezel() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/LEDBezel.svg")));
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

	void setFirstLightId(int firstLightId) {
		if (paramQuantity)
			light->module = paramQuantity->module;
		light->firstLightId = firstLightId;
	}
};

struct PB61303 : app::SvgSwitch {
	PB61303() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/PB61303.svg")));
	}
};

////////////////////
// Misc
////////////////////

struct ScrewSilver : app::SvgScrew {
	ScrewSilver() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/ScrewSilver.svg")));
	}
};

struct ScrewBlack : app::SvgScrew {
	ScrewBlack() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/ScrewBlack.svg")));
	}
};

struct SegmentDisplay : widget::Widget {
	int lightsLen = 0;
	bool vertical = false;
	float margin = app::mm2px(0.5);

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


} // namespace componentlibrary
} // namespace rack
