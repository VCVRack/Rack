#include "common.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

#ifdef LPTEST_MODULE
////////////////////
// module widgets
////////////////////
using namespace rack;
#define plugin "TheXOR"

namespace rack_plugin_TheXOR {

struct LaunchpadTest;
struct LaunchpadTestWidget : ModuleWidget
{
	LaunchpadTestWidget(LaunchpadTest * module);
};

struct PatternBtn : SVGSwitch, ToggleSwitch {
	PatternBtn() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Patternbtn_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Patternbtn_1.svg")));
	}
};

struct LaunchpadTest : Module
{
	enum ParamIds
	{
		BTN,
		SW,
		KNOB,
		NUM_PARAMS
	};
	enum InputIds
	{
		IN_V,
		NUM_INPUTS
	};
	enum OutputIds
	{
		KNOB_OUT,
		BTN_OUT,
		SW_OUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		LP_CONNECTED,
		NUM_LIGHTS
	};
	LaunchpadTest() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{
		v_in = 0;
		drv = new LaunchpadBindingDriver(this, Scene8, 1);
	}
	~LaunchpadTest()
	{
		delete drv;
	}
	void step() override;

	LaunchpadBindingDriver *drv;
	float v_in;
};

#endif

} // namespace rack_plugin_TheXOR
