#pragma once
//#define DEBUG
#include "rack.hpp"
#include "dsp/digital.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace rack_plugin_TheXOR {

#define LVL_ON    (10.0)
#define LVL_OFF   (0.0)

using namespace rack;
#define plugin "TheXOR"

#if defined(ARCH_WIN) && defined(USE_LAUNCHPAD)
#define LAUNCHPAD
#endif

#ifdef LAUNCHPAD
#include "../digitalExt/launchpad.hpp"
#include "../digitalExt/launchpadControls.hpp"
#ifdef DEBUG
#define LPTEST_MODULE
#endif
#endif

#if defined(ARCH_WIN) && defined(USE_OSC)
#define OSC_ENABLE
#ifdef DEBUG
#define OSCTEST_MODULE
#endif
#include "../digitalExt/osc/oscDriver.hpp"
#endif

#if defined(LAUNCHPAD) || defined(OSCTEST_MODULE)
#define DIGITAL_EXT
#endif

struct _davies1900base : Davies1900hKnob 
{
	_davies1900base(const char *res) 
	{
		setSVG(SVG::load(assetPlugin(plugin, res)));
	}

	void randomize() override
	{
		if(snap)
			setValue(roundf(rescale(randomUniform(), 0.0, 1.0, minValue, maxValue)));
		else
			Davies1900hKnob::randomize();
	}
};

struct Davies1900hFixWhiteKnob : _davies1900base 
{
	Davies1900hFixWhiteKnob() : _davies1900base("res/Davies1900hWhite.svg") {}
};

struct Davies1900hFixBlackKnob : _davies1900base 
{
	Davies1900hFixBlackKnob() : _davies1900base("res/Davies1900hBlack.svg") {}
};

struct Davies1900hFixRedKnob : _davies1900base 
{
	Davies1900hFixRedKnob() : _davies1900base("res/Davies1900hRed.svg") {}
};

struct Davies1900hFixWhiteKnobSmall : _davies1900base
{
	Davies1900hFixWhiteKnobSmall() : _davies1900base("res/Davies1900hWhiteSmall.svg") {}
};

struct WhiteLight : GrayModuleLightWidget
{
	WhiteLight()
	{
		addBaseColor(COLOR_WHITE);
	}
};

struct _ioPort : SVGPort
{
	_ioPort(const char *res)
	{
		background->svg = SVG::load(assetPlugin(plugin, res));
		background->wrap();
		box.size = background->box.size;
	}
};

struct PJ301YPort : _ioPort 
{
	PJ301YPort() : _ioPort("res/PJ301Y.svg") {}
};

struct PJ301BPort : _ioPort
{
	PJ301BPort() : _ioPort("res/PJ301B.svg") {}
};

struct PJ301GPort : _ioPort
{
	PJ301GPort() : _ioPort("res/PJ301G.svg") {}
};
struct PJ301GRPort : _ioPort
{
	PJ301GRPort() : _ioPort("res/PJ301GR.svg") {}
};

struct PJ301RPort : _ioPort
{
	PJ301RPort() : _ioPort("res/PJ301R.svg") {}
};

struct PJ301WPort : _ioPort
{
	PJ301WPort() : _ioPort("res/PJ301W.svg") {}
};

struct PJ301OPort : _ioPort
{
	PJ301OPort() : _ioPort("res/PJ301O.svg") {}
};

struct PJ301BLUPort : _ioPort
{
	PJ301BLUPort() : _ioPort("res/PJ301BLU.svg") {}
};

struct CL1362YPort : _ioPort
{
	CL1362YPort() : _ioPort("res/CL1362Y.svg") {}
};

struct CL1362GPort : _ioPort
{
	CL1362GPort() : _ioPort("res/CL1362G.svg") {}
};

struct CL1362RPort : _ioPort
{
	CL1362RPort() : _ioPort("res/CL1362R.svg") {}
};

struct CL1362WPort : _ioPort
{
	CL1362WPort() : _ioPort("res/CL1362W.svg") {}
};

struct BefacoPushBig : SVGSwitch, MomentarySwitch {
	BefacoPushBig() {
		addFrame(SVG::load(assetPlugin(plugin, "res/BefacoPush_0big.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/BefacoPush_1big.svg")));
	}
};

struct CKSSFix : SVGSwitch, ToggleSwitch {
	CKSSFix() {
		addFrame(SVG::load(assetPlugin(plugin, "res/CKSS_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CKSS_1.svg")));
	}
	void randomize() override
	{
		setValue(roundf(rescale(randomUniform(), 0.0, 1.0, minValue, maxValue)));
	}
};

struct CKSSThreeFix : SVGSwitch, ToggleSwitch {
	CKSSThreeFix() {
		addFrame(SVG::load(assetPlugin(plugin, "res/CKSSThree_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CKSSThree_1.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CKSSThree_2.svg")));
	}
	void randomize() override
	{
		setValue(roundf(rescale(randomUniform(), 0.0, 1.0, minValue, maxValue)));
	}
};

struct TL1105Sw : SVGSwitch, ToggleSwitch {
	TL1105Sw() {
		addFrame(SVG::load(assetPlugin(plugin, "res/TL1105_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/TL1105_1.svg")));
	}
};

struct SchmittTrigger2
{
	// UNKNOWN is used to represent a stable state when the previous state is not yet set
	enum { UNKNOWN, LOW, HIGH } state = UNKNOWN;
	float low = 0.0;
	float high = 1.0;
	void setThresholds(float low, float high)
	{
		this->low = low;
		this->high = high;
	}

	int process(float in)
	{
		switch(state)
		{
			case LOW:
			if(in >= high)
			{
				state = HIGH;
				return 1;
			}
			break;
			case HIGH:
			if(in <= low)
			{
				state = LOW;
				return -1;
			}
			break;
			default:
			if(in >= high)
			{
				state = HIGH;
			} else if(in <= low)
			{
				state = LOW;
			}
			break;
		}
		return 0;
	}

	void reset()
	{
		state = UNKNOWN;
	}
};

struct NKK2 : SVGSwitch, ToggleSwitch
{
	NKK2() {
		addFrame(SVG::load(assetPlugin(plugin, "res/NKK_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/NKK_1.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/NKK_2.svg")));
	}

	void randomize() override
	{
		setValue(roundf(rescale(randomUniform(), 0.0, 1.0, minValue, maxValue)));
	}
};

struct BefacoSnappedSwitch : SVGSwitch, ToggleSwitch
{
	void randomize() override
	{
		if(randomUniform() >= 0.5)
			setValue(1.0);
		else
			setValue(0.0);
	}

	BefacoSnappedSwitch()
	{
		addFrame(SVG::load(assetPlugin(plugin, "res/BefacoSwitch_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/BefacoSwitch_2.svg")));
	}
};



struct VerticalSwitch : SVGFader 
{
	VerticalSwitch()
	{
		snap = true;
		maxHandlePos = Vec(-mm2px(2.3-2.3/2.0), 0);
		minHandlePos = Vec(-mm2px(2.3-2.3/2.0),mm2px(13-2.8));
		background->svg = SVG::load(assetPlugin(plugin, "res/counterSwitchShort.svg"));
		background->wrap();
		background->box.pos = Vec(0, 0);
		box.size = background->box.size;
		handle->svg = SVG::load(assetPlugin(plugin, "res/counterSwitchPotHandle.svg"));
		handle->wrap();
	}

	void randomize() override { setValue(roundf(randomUniform() * maxValue)); }

};

template<class T> struct SeqMenuItem : MenuItem
{
public:
	SeqMenuItem(const char *title, T *pW, int act)
	{
		text = title;
		widget = pW;
		action = act;
	};

	void onAction(EventAction &e) override { widget->onMenu(action); };

private:
	T *widget;
	int action;
};

class SequencerWidget : public ModuleWidget
{
protected:
	SequencerWidget(Module *module) : ModuleWidget(module) 	{}
	float yncscape(float y, float height)
	{
		return RACK_GRID_HEIGHT - mm2px(y + height);
	}
	int getParamIndex(int index)
	{
		auto it = std::find_if(params.begin(), params.end(), [&index](const ParamWidget *m) -> bool { return m->paramId == index; });
		if(it != params.end())
			return std::distance(params.begin(), it);

		return -1;
	}

	void std_randomize(int first_index, int last_index)
	{
		for(int k = first_index; k < last_index; k++)
		{
			int index = getParamIndex(k);
			if(index >= 0)
			{
				params[index]->randomize();
			}
		}
	}

	Menu *createContextMenu() override
	{
		Menu *menu = ModuleWidget::createContextMenu();
		MenuLabel *spacerLabel = new MenuLabel();
		menu->addChild(spacerLabel);
		return addContextMenu(menu);
	}

	virtual Menu *addContextMenu(Menu *menu) { return menu; }
};

#if defined(LAUNCHPAD) || defined(OSC_ENABLE)
struct DigitalLed : SVGWidget
{
	float *value;
	std::vector<std::shared_ptr<SVG>> frames;

	DigitalLed(int x, int y, float *pVal)
	{
		frames.push_back(SVG::load(assetPlugin(plugin, "res/digitalLed_off.svg")));
		frames.push_back(SVG::load(assetPlugin(plugin, "res/digitalLed_on.svg")));
		setSVG(frames[0]);
		wrap();
		box.pos = Vec(x, y);
		value = pVal;
	}

	void draw(NVGcontext *vg) override
	{
		int index = (*value > 0) ? 1 : 0;
		setSVG(frames[index]);
		SVGWidget::draw(vg);
	}
};
#endif

struct SigDisplayWidget : TransparentWidget
{
private:
	int digits;
	int precision;
	std::shared_ptr<Font> font;

public:
	float *value;
	SigDisplayWidget(int digit, int precis = 0)
	{
		digits = digit;
		precision = precis;
		font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
	};

	void draw(NVGcontext *vg) override
	{
		// Background
		NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
		NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		nvgStrokeWidth(vg, 1.0);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);
		// text
		nvgFontSize(vg, 18);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 2.5);

		std::stringstream to_display;
		if(precision == 0)
			to_display << std::setw(digits) << std::round(*value);
		else
			to_display << std::fixed << std::setw(digits) << std::setprecision(precision) << *value;

		Vec textPos = Vec(3, 17);

		NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
		nvgFillColor(vg, nvgTransRGBA(textColor, 16));
		nvgText(vg, textPos.x, textPos.y, "~~", NULL);

		textColor = nvgRGB(0xda, 0xe9, 0x29);
		nvgFillColor(vg, nvgTransRGBA(textColor, 16));
		nvgText(vg, textPos.x, textPos.y, "\\\\", NULL);

		textColor = nvgRGB(0xf0, 0x00, 0x00);
		nvgFillColor(vg, textColor);
		nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
	}
};

struct TIMER
{
	float Reset()
	{
		prevTime = clock();
		return Begin();
	}

	void RestartStopWatch() { stopwatch = 0; }
	float Begin()
	{
		RestartStopWatch();
		return totalPulseTime = 0;
	}
	float Elapsed() { return totalPulseTime; }
	float StopWatch() { return stopwatch; }

	float Step()
	{
		clock_t curTime = clock();
		clock_t deltaTime = curTime - prevTime;
		prevTime = curTime;
		float t = float(deltaTime) / CLOCKS_PER_SEC;
		totalPulseTime += t;
		stopwatch += t;
		return t;
	}

private:
	clock_t prevTime;
	float totalPulseTime;
	float stopwatch;
};

} // namespace rack_plugin_TheXOR
