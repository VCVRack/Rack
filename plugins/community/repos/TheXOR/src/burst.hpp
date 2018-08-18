#include "common.hpp"

////////////////////
// module widgets
////////////////////
using namespace rack;
#define plugin "TheXOR"

namespace rack_plugin_TheXOR {

struct Burst;
struct BurstWidget : SequencerWidget
{
	BurstWidget(Burst * module);
};

#define NUM_BURST_PORTS (6)
struct Burst : Module
{
	enum ParamIds
	{
		OUT_SPAN,
		EVENT_COUNT,
		MODE,
		MODE_INVERT,
		TRIGGER,
		TRIG_THRESH,
		NUM_PARAMS
	};

	enum InputIds
	{
		CLOCK_IN,
		OUT_SPAN_IN,
		EVENT_COUNT_IN,
		TRIGGER_THRESH_IN,
		RESET,
		NUM_INPUTS
	};

	enum OutputIds
	{
		OUT_1,
		NUM_OUTPUTS = OUT_1 + NUM_BURST_PORTS
	};

	enum LightIds
	{
		LEDOUT_1,
		NUM_LIGHTS = LEDOUT_1 + NUM_BURST_PORTS
	};

	Burst() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{		
	}

	void step() override;
	void reset() override { load(); }
	void fromJson(json_t *root) override { Module::fromJson(root); on_loaded(); }
	json_t *toJson() override
	{
		json_t *rootJ = json_object();
		return rootJ;
	};

private:
	void on_loaded();
	void load();
	void all_off();
	int getInt(ParamIds p_id, InputIds i_id, float minValue, float maxValue);
	void prepare_step();
	void next_step();
	void end_step();
	void port(int n, bool on) { lights[LEDOUT_1 + n].value = outputs[OUT_1 + n].value = on ? LVL_ON : LVL_OFF; }
	void invert_port(int n) { port(n, outputs[OUT_1 + n].value < LVL_ON); }

private:
	SchmittTrigger2 clock;
	SchmittTrigger trigger;
	SchmittTrigger resetTrigger;
	bool active;
	bool trigger_pending;
	enum MODE
	{
		FWD = 0,
		PEND = 1,
		RAND = 2
	};
	struct 
	{
		int cycle_counter;
		int max_cycle;
		int out_span;
		int max_span;
		enum MODE mode;
		bool invert_mode;
		bool retrogade;
		bool first_cycle;
	} activating_params;

};

} // namespace rack_plugin_TheXOR
