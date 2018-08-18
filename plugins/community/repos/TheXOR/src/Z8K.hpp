#pragma once
#include "common.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "z8kSequencer.hpp"

namespace rack_plugin_TheXOR {

////////////////////
// module widgets
////////////////////

struct Z8K;
struct Z8KWidget : SequencerWidget
{
public:
	Z8KWidget(Z8K *module);
};

struct Z8K : Module
{
	enum ParamIds
	{
		VOLTAGE_1,
		NUM_PARAMS = VOLTAGE_1 + 16
	};

	enum InputIds
	{
		RESET_1,
		RESET_A = RESET_1 + 4,
		RESET_VERT = RESET_A + 4,
		RESET_HORIZ,

		DIR_1,
		DIR_A = DIR_1 + 4,
		DIR_VERT = DIR_A + 4,
		DIR_HORIZ,

		CLOCK_1,
		CLOCK_A = CLOCK_1 + 4,
		CLOCK_VERT = CLOCK_A + 4,
		CLOCK_HORIZ,

		NUM_INPUTS
	};

	enum OutputIds
	{
		CV_1,
		CV_A = CV_1 + 4,
		CV_VERT = CV_A + 4,
		CV_HORIZ,
		ACTIVE_STEP,
		NUM_OUTPUTS = ACTIVE_STEP + 16
	};

	enum LightIds
	{
		LED_ROW,
		LED_COL = LED_ROW + 16,
		LED_VERT = LED_COL + 16,
		LED_HORIZ = LED_VERT + 16,
		NUM_LIGHTS = LED_HORIZ + 16
	};

	enum SequencerIds
	{
		SEQ_1,
		SEQ_A = SEQ_1 + 4,
		SEQ_VERT = SEQ_A + 4,
		SEQ_HORIZ,
		NUM_SEQUENCERS
	};

	Z8K() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{
		/*
		#ifdef LAUNCHPAD
		drv = new LaunchpadBindingDriver(this, Scene4, 1);
		drv->SetAutoPageKey(LaunchpadKey::SESSION, 0);
		#endif*/
		#ifdef OSCTEST_MODULE
		oscDrv = new OSCDriver(this, 4);
		#endif
		on_loaded();
	}

	#ifdef DIGITAL_EXT
	~Z8K()
	{
		/*#if defined(LAUNCHPAD)
		delete drv;
		#endif*/
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
	/*#ifdef LAUNCHPAD
	LaunchpadBindingDriver *drv;
	#endif*/
	#if defined(OSCTEST_MODULE)
	OSCDriver *oscDrv;
	#endif

private:
	void on_loaded();
	void load();
	z8kSequencer seq[10];
};

} // namespace rack_plugin_TheXOR
