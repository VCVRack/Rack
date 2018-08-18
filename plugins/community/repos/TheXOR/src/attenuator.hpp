#include "common.hpp"

////////////////////
// module widgets
////////////////////
using namespace rack;
#define plugin "TheXOR"

#define NUM_ATTENUATORS  (6)

namespace rack_plugin_TheXOR {

struct Attenuator;
struct AttenuatorWidget : ModuleWidget
{
	AttenuatorWidget(Attenuator * module);
private:
	float yncscape(float y, float height)
	{
		return RACK_GRID_HEIGHT - mm2px(y + height);
	}
};

struct Attenuator : Module
{
	enum ParamIds
	{
		ATT_1,
		NUM_PARAMS = ATT_1 + NUM_ATTENUATORS
	};
	enum InputIds
	{
		IN_1,
		NUM_INPUTS = IN_1 + NUM_ATTENUATORS
	};
	enum OutputIds
	{
		OUT_1,
		NUM_OUTPUTS = OUT_1 + NUM_ATTENUATORS
	};
	enum LightIds
	{
		NUM_LIGHTS
	};
	Attenuator() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{		
	}
	void step() override;
};

} // namespace rack_plugin_TheXOR
