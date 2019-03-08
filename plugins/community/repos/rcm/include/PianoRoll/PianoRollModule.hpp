#include "rack.hpp"
#include "dsp/digital.hpp"
#include "dsp/ringbuffer.hpp"

#include "PatternData.hpp"
#include "Transport.hpp"
#include "Auditioner.hpp"

#include "../ValueChangeTrigger.hpp"

namespace rack_plugin_rcm {

struct PianoRollModule : rack::Module {
	enum ParamIds {
		RUN_BUTTON_PARAM,
		RESET_BUTTON_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		RUN_INPUT,
		RESET_INPUT,
		PATTERN_INPUT,
		RECORD_INPUT,
		VOCT_INPUT,
		GATE_INPUT,
		RETRIGGER_INPUT,
		VELOCITY_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CLOCK_OUTPUT,
		RUN_OUTPUT,
		RESET_OUTPUT,
		PATTERN_OUTPUT,
		RECORD_OUTPUT,
		VOCT_OUTPUT,
		GATE_OUTPUT,
		RETRIGGER_OUTPUT,
		VELOCITY_OUTPUT,
		END_OF_PATTERN_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	rack::SchmittTrigger clockInputTrigger;
	rack::SchmittTrigger resetInputTrigger;
	rack::SchmittTrigger runInputTrigger;

	rack::PulseGenerator retriggerOutputPulse;
	rack::PulseGenerator eopOutputPulse;
	rack::PulseGenerator gateOutputPulse;

	Auditioner auditioner;

	ValueChangeTrigger<bool> runInputActive;
	rack::RingBuffer<float, 16> clockBuffer;
	int clockDelay = 0;

	rack::SchmittTrigger recordingIn;
	rack::RingBuffer<float, 512> voctInBuffer;
	rack::RingBuffer<float, 512> gateInBuffer;
	rack::RingBuffer<float, 512> retriggerInBuffer;
	rack::RingBuffer<float, 512> velocityInBuffer;

  PatternData patternData;
  Transport transport;

	PianoRollModule();

	void step() override;
	void onReset() override;

	json_t *toJson() override;
	void fromJson(json_t *rootJ) override;
};

} // namespace rack_plugin_rcm
