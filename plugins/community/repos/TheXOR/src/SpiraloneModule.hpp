#pragma once

namespace rack_plugin_TheXOR {

struct Spiralone : Module
{
	enum ParamIds
	{
		M_RESET,
		VOLTAGE_1,
		MODE_1 = VOLTAGE_1 + TOTAL_STEPS,
		LENGHT_1 = MODE_1 + NUM_SEQUENCERS,
		STRIDE_1 = LENGHT_1 + NUM_SEQUENCERS,
		XPOSE_1 = STRIDE_1 + NUM_SEQUENCERS,
		NUM_PARAMS = XPOSE_1 + NUM_SEQUENCERS
	};

	enum InputIds
	{
		RESET_1,
		INLENGHT_1 = RESET_1 + NUM_SEQUENCERS,
		INSTRIDE_1 = INLENGHT_1 + NUM_SEQUENCERS,
		INXPOSE_1 = INSTRIDE_1 + NUM_SEQUENCERS,
		CLOCK_1 = INXPOSE_1 + NUM_SEQUENCERS,
		NUM_INPUTS = CLOCK_1 + NUM_SEQUENCERS
	};

	enum OutputIds
	{
		CV_1,
		GATE_1 = CV_1 + NUM_SEQUENCERS,
		NUM_OUTPUTS = GATE_1 + NUM_SEQUENCERS
	};

	enum LightIds
	{
		LED_SEQUENCE_1,
		NUM_LIGHTS = (LED_SEQUENCE_1 + TOTAL_STEPS) * NUM_SEQUENCERS
	};

	Spiralone() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{
		#ifdef LAUNCHPAD
		drv = new LaunchpadBindingDriver(this, Scene5, 1);
		#endif
		#ifdef OSCTEST_MODULE
		oscDrv = new OSCDriver(this, 5);
		#endif

		on_loaded();
	}

	#ifdef DIGITAL_EXT
	~Spiralone()
	{
		#if defined(LAUNCHPAD)
		delete drv;
		#endif
		#if defined(OSCTEST_MODULE)
		delete oscDrv;
		#endif
	}
	#endif

	void step() override;
	void reset() override { load(); }

	void fromJson(json_t *root) override { Module::fromJson(root); on_loaded(); }
	json_t *toJson() override
	{
		json_t *rootJ = json_object();
		return rootJ;
	}

	#ifdef DIGITAL_EXT
	float connected;
	#endif
	#ifdef LAUNCHPAD
	LaunchpadBindingDriver *drv;
	#endif
	#if defined(OSCTEST_MODULE)
	OSCDriver *oscDrv;
	#endif

private:
	void on_loaded();
	void load();

	spiraloneSequencer sequencer[NUM_SEQUENCERS];
	SchmittTrigger masterReset;
};

} // namespace rack_plugin_TheXOR
