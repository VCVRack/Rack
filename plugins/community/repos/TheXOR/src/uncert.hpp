#include "common.hpp"

////////////////////
// module widgets
////////////////////
using namespace rack;
#define plugin "TheXOR"

namespace rack_plugin_TheXOR {

struct Uncertain;
struct UncertainWidget : SequencerWidget
{
	UncertainWidget(Uncertain *module);
};

struct Uncertain : Module
{
	static constexpr float SEMITONE = 1.0 / 12.0;// 1/12 V
	static constexpr float MIN_VOLTAGE = 1.0 / 96.0;// 1/96 V
	static constexpr float MAX_VOLTAGE = 10.0; // 10V
	enum ParamIds
	{
		FLUCT_AMT,
		QUANTIZED_AMT,
		STORED_AMT,
		CURVEAMP_AMT,
		NUM_PARAMS
	};

	enum InputIds
	{
		CLOCK_FLUCT,
		IN_FLUCT,
		CLOCK_QUANTIZED,
		IN_QUANTIZED,
		CLOCK_STORED,
		IN_STORED,
		IN_CURVEAMP,
		NUM_INPUTS
	};

	enum OutputIds
	{
		OUT_FLUCT,
		OUT_QUANTIZED_N1,
		OUT_QUANTIZED_2N,
		OUT_STORED_RND,
		OUT_STORED_BELL,
		NUM_OUTPUTS
	};

	enum LightIds
	{
		NUM_LIGHTS 
	};

	Uncertain() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
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
	void out_quantized(int clk);
	void out_stored(int clk);
	void out_fluct(int clk);
	int getInt(ParamIds p_id, InputIds i_id, float minValue, float maxValue) { return (int)getFloat(p_id, i_id, minValue, maxValue); }
	float getFloat(ParamIds p_id, InputIds i_id, float minValue, float maxValue);
	float rndFluctVoltage();
	float rndGaussianVoltage();

private:
	SchmittTrigger2 clock_fluct;
	SchmittTrigger2 clock_quantized;
	SchmittTrigger2 clock_stored;
	struct fluct_params
	{
		float vA;
		float vB;
		float deltaV;
		clock_t tStart;
		clock_t duration;

		void reset()
		{
			duration = tStart = 0;
			vA = deltaV = 0;
		}	
	} fluctParams;
};

} // namespace rack_plugin_TheXOR
