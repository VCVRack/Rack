#pragma once
#include "common.hpp"

namespace rack_plugin_TheXOR {

struct UPSWITCH : SVGSwitch, MomentarySwitch
{
	UPSWITCH()
	{
		addFrame(SVG::load(assetPlugin(plugin, "res/upswitch_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/upswitch_1.svg")));
	}
};

struct DNSWITCH : SVGSwitch, MomentarySwitch
{
	DNSWITCH()
	{
		addFrame(SVG::load(assetPlugin(plugin, "res/dnswitch_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/dnswitch_1.svg")));
	}
};

struct Rogan1PSRedSmall : Rogan
{
	Rogan1PSRedSmall()
	{
		setSVG(SVG::load(assetPlugin(plugin, "res/Rogan2PSRedSmall.svg")));
	}
};

#define OUT_SOCKETS (21)
struct PwmClock;
struct PwmClockWidget : SequencerWidget
{
	PwmClockWidget(PwmClock *module);
	void SetBpm(float bpmint);
};


struct SA_TIMER	//sample accurate version
{
	float Reset()
	{
		prevTime = curTime = engineGetSampleTime();
		return Begin();
	}

	void RestartStopWatch() { stopwatch = 0; }
	float Begin()
	{
		RestartStopWatch();
		return totalPulseTime = 0;
	}
	float Elapsed() { return totalPulseTime; }
	float StopWatch() { return stopwatch; }

	float Step()
	{
		curTime += engineGetSampleTime();
		float deltaTime = curTime - prevTime;
		prevTime = curTime;
		totalPulseTime += deltaTime;
		stopwatch += deltaTime;
		return deltaTime;
	}

private:
	float curTime;
	float prevTime;
	float totalPulseTime;
	float stopwatch;
};

struct PwmClock : Module
{
	enum ParamIds
	{
		BPM_INC, BPM_DEC,
		PWM, BPM, BPMDEC,
		SWING,
		OFFON,
		NUM_PARAMS
	};
	enum InputIds
	{
		RESET,
		EXT_BPM,
		PWM_IN,
		SWING_IN,
		OFFON_IN,
		NUM_INPUTS
	};

	enum OutputIds
	{
		OUT_1,
		NUM_OUTPUTS = OUT_1 + OUT_SOCKETS
	};

	enum LightIds
	{
		ACTIVE,
		NUM_LIGHTS
	};

	PwmClock() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{

		on_loaded();
	}
	void step() override;

	json_t *toJson() override
	{
		json_t *rootJ = json_object();
		json_t *bpmJson = json_integer((int)bpm_integer);
		json_object_set_new(rootJ, "bpm_integer", bpmJson);
		return rootJ;
	}

	void fromJson(json_t *rootJ) override
	{
		json_t *bpmJson = json_object_get(rootJ, "bpm_integer");
		if(bpmJson)
			bpm_integer = json_integer_value(bpmJson);
		on_loaded();
	}

	void reset() override
	{
		bpm_integer = 120;

		load();
	}
	void randomize() override {}
	void setWidget(PwmClockWidget *pwdg) { pWidget = pwdg; }
	float bpm;
	float swing;

private:
	SchmittTrigger btnup;
	SchmittTrigger btndwn;
	PwmClockWidget *pWidget;
	uint32_t tick = UINT32_MAX;
	int bpm_integer = 120;
	SchmittTrigger2 resetTrigger;

	void process_keys();
	void updateBpm();
	
	float getDuration(int n)
	{
		return odd_beat[n] ? swingAmt[n] : duration[n];
	}
	float duration[OUT_SOCKETS];
	float swingAmt[OUT_SOCKETS];
	bool odd_beat[OUT_SOCKETS];
	void on_loaded();
	void load();
	void _reset();
	float getPwm();
	float getSwing();
	SA_TIMER sa_timer[OUT_SOCKETS];
};

} // namespace rack_plugin_TheXOR
