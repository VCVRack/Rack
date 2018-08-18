#include "common.hpp"

////////////////////
// module widgets
////////////////////
using namespace rack;
#define plugin "TheXOR"

namespace rack_plugin_TheXOR {

#define NUM_BOOL_OP		(5)		//not, and, or, (the) xor, implication
struct Boole;
struct BooleWidget : ModuleWidget
{
	BooleWidget(Boole * module);
private:
	float yncscape(float y, float height)
	{
		return RACK_GRID_HEIGHT - mm2px(y + height);
	}
};

struct Boole : Module
{
	enum ParamIds
	{
		INVERT_1,
		THRESH_1 = INVERT_1 + NUM_BOOL_OP - 1,
		NUM_PARAMS = THRESH_1 + 2* NUM_BOOL_OP-1
	};
	enum InputIds
	{
		IN_1,
		NUM_INPUTS = IN_1 + 2 * NUM_BOOL_OP-1
	};
	enum OutputIds
	{
		OUT_1,
		NUM_OUTPUTS = OUT_1 + NUM_BOOL_OP
	};
	enum LightIds
	{
		LED_1,
		NUM_LIGHTS = LED_1 + 3* NUM_BOOL_OP-1
	};

	Boole() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{		
	}
	void step() override;

private:
	bool process(int num_op, int index);
};

} // namespace rack_plugin_TheXOR
