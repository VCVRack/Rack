#pragma once
#include "common.hpp"
#ifdef OSCTEST_MODULE
#include <sstream>
#include <iomanip>
#include <algorithm>

////////////////////
// module widgets
////////////////////
using namespace rack;
extern Plugin *plugin;

struct OscTest;
struct OscTestWidget : ModuleWidget
{
	OscTestWidget(OscTest *module);
};

struct OscTest : Module
{
	enum ParamIds
	{
		BTN1,
		POT1,
		NUM_PARAMS
	};
	enum InputIds
	{

		NUM_INPUTS
	};
	enum OutputIds
	{
		OUT_1,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		LED1,
		NUM_LIGHTS
	};
	OscTest() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{
		connected = 0;
		drv = new OSCDriver(this, 8);
		lasttime = clock();
	}
	~OscTest()
	{
		delete drv;
	}
	void step() override;

	OSCDriver *drv;
	float connected;
	clock_t lasttime;
};

#endif
