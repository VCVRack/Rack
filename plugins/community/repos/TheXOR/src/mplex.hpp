#include "common.hpp"

////////////////////
// module widgets
////////////////////
using namespace rack;
#define plugin "TheXOR"

namespace rack_plugin_TheXOR {

#define NUM_MPLEX_INPUTS		(8)

struct Mplex;
struct MplexWidget : ModuleWidget
{
	MplexWidget(Mplex * module);
private:
	float yncscape(float y, float height)
	{
		return RACK_GRID_HEIGHT - mm2px(y + height);
	}
};

struct Mplex : Module
{
	enum ParamIds
	{
		BTUP, BTDN,
		NUM_PARAMS
	};
	enum InputIds
	{
		INUP,
		INDN,
		IN_1,
		NUM_INPUTS = IN_1 + NUM_MPLEX_INPUTS
	};
	enum OutputIds
	{
		OUT_1,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		LED_1,
		NUM_LIGHTS = LED_1 + NUM_MPLEX_INPUTS
	};

	Mplex() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{		
		load();
	}

	void fromJson(json_t *root) override { Module::fromJson(root); on_loaded(); }
	json_t *toJson() override
	{
		json_t *rootJ = json_object();

		return rootJ;
	}
	void step() override;
	void reset() override { load(); }
	void randomize() override 
	{
		set_output((int)roundf(rescale(randomUniform(), 0.0, 1.0, 0, NUM_MPLEX_INPUTS)));
	}

private:
	void load();
	void on_loaded();
	void set_output(int n);

	int cur_sel;
	SchmittTrigger upTrigger;
	SchmittTrigger dnTrigger;
};

} // namespace rack_plugin_TheXOR
