#include "common.hpp"

////////////////////
// module widgets
////////////////////
using namespace rack;
#define plugin "TheXOR"

namespace rack_plugin_TheXOR {

#define NUM_SWITCHES  (5)

struct Switch;
struct SwitchWidget : ModuleWidget
{
	SwitchWidget(Switch * module);
private:
	float yncscape(float y, float height)
	{
		return RACK_GRID_HEIGHT - mm2px(y + height);
	}
};

struct Switch : Module
{
	enum ParamIds
	{
		SW_1,
		NUM_PARAMS = SW_1 + NUM_SWITCHES
	};
	enum InputIds
	{
		IN_1,
		MOD_1= IN_1 + NUM_SWITCHES,
		NUM_INPUTS = MOD_1 + NUM_SWITCHES
	};
	enum OutputIds
	{
		OUT_1,
		NUM_OUTPUTS = OUT_1 + NUM_SWITCHES
	};
	enum LightIds
	{
		LED_1,
		NUM_LIGHTS = LED_1 + NUM_SWITCHES
	};
	Switch() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{		
	}
	void step() override;

private:

	bool getSwitch(int n)
	{
		return (inputs[MOD_1 + n].normalize(0.0) + params[SW_1 + n].value) > 0.5;
	}
};

} // namespace rack_plugin_TheXOR
