#include "common.hpp"

////////////////////
// module widgets
////////////////////
using namespace rack;
#define plugin "TheXOR"

namespace rack_plugin_TheXOR {

#define NUM_QUANTIZERS  (6)

struct Quantizer;
struct QuantizerWidget : ModuleWidget
{
	QuantizerWidget(Quantizer * module);
private:
	float yncscape(float y, float height)
	{
		return RACK_GRID_HEIGHT - mm2px(y + height);
	}
};

struct Quantizer : Module
{
	enum ParamIds
	{
		TRANSP_1,
		NUM_PARAMS = TRANSP_1 + NUM_QUANTIZERS
	};
	enum InputIds
	{
		IN_1,
		TRNSPIN_1 = IN_1 + NUM_QUANTIZERS,
		NUM_INPUTS = TRNSPIN_1 + NUM_QUANTIZERS
	};
	enum OutputIds
	{
		OUT_1,
		NUM_OUTPUTS = OUT_1 + NUM_QUANTIZERS
	};
	enum LightIds
	{
		NUM_LIGHTS
	};
	Quantizer() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{		
	}
	void step() override;

private:
	float quantize_out(Input &in, float transpose);
	float getQuantize(int n);
};

} // namespace rack_plugin_TheXOR
