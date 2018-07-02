#include <string>
#include <chrono>

#include "dsp/digital.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"

#include "peaks/io_buffer.h"
#include "peaks/processors.h"

#include "AudibleInstruments.hpp"


enum SwitchIndex {
	SWITCH_TWIN_MODE,
	SWITCH_FUNCTION,
	SWITCH_GATE_TRIG_1,
	SWITCH_GATE_TRIG_2
};

enum EditMode {
	EDIT_MODE_TWIN,
	EDIT_MODE_SPLIT,
	EDIT_MODE_FIRST,
	EDIT_MODE_SECOND,
	EDIT_MODE_LAST
};

enum Function {
	FUNCTION_ENVELOPE,
	FUNCTION_LFO,
	FUNCTION_TAP_LFO,
	FUNCTION_DRUM_GENERATOR,
	FUNCTION_MINI_SEQUENCER,
	FUNCTION_PULSE_SHAPER,
	FUNCTION_PULSE_RANDOMIZER,
	FUNCTION_FM_DRUM_GENERATOR,
	FUNCTION_LAST,
	FUNCTION_FIRST_ALTERNATE_FUNCTION = FUNCTION_MINI_SEQUENCER
};

struct Settings {
	uint8_t edit_mode;
	uint8_t function[2];
	uint8_t pot_value[8];
	bool snap_mode;
};


static const int32_t kLongPressDuration = 600;
static const uint8_t kNumAdcChannels = 4;
static const uint16_t kAdcThresholdUnlocked = 1 << (16 - 10);  // 10 bits
static const uint16_t kAdcThresholdLocked = 1 << (16 - 8);  // 8 bits


// Global scope, so variables can be accessed by process() function.
int16_t gOutputBuffer[peaks::kBlockSize];
int16_t gBrightness[2] = {0, 0};


static void set_led_brightness(int channel, int16_t value) {
	gBrightness[channel] = value;
}

// File scope because of IOBuffer function signature.
// It cannot refer to a member function of class Peaks().
static void process(peaks::IOBuffer::Block* block, size_t size) {
	for (size_t i = 0; i < peaks::kNumChannels; ++i) {
		// TODO
		// processors[i].Process(block->input[i], gOutputBuffer, size);
		set_led_brightness(i, gOutputBuffer[0]);
		for (size_t j = 0; j < size; ++j) {
			// From calibration_data.h, shifting signed to unsigned values.
			int32_t shifted_value = 32767 + static_cast<int32_t>(gOutputBuffer[j]);
			CONSTRAIN(shifted_value, 0, 65535);
			block->output[i][j] = static_cast<uint16_t>(shifted_value);
		}
	}
}


struct Peaks : Module {
	enum ParamIds {
		KNOB_1_PARAM,
		KNOB_2_PARAM,
		KNOB_3_PARAM,
		KNOB_4_PARAM,
		BUTTON_1_PARAM,
		BUTTON_2_PARAM,
		TRIG_1_PARAM,
		TRIG_2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		GATE_1_INPUT,
		GATE_2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_1_OUTPUT,
		OUT_2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		TRIG_1_LIGHT,
		TRIG_2_LIGHT,
		TWIN_MODE_LIGHT,
		FUNC_1_LIGHT,
		FUNC_2_LIGHT,
		FUNC_3_LIGHT,
		FUNC_4_LIGHT,
		NUM_LIGHTS
	};

	static const peaks::ProcessorFunction function_table_[FUNCTION_LAST][2];

	EditMode edit_mode_ = EDIT_MODE_TWIN;
	Function function_[2] = {FUNCTION_ENVELOPE, FUNCTION_ENVELOPE};
	Settings settings_;

	uint8_t pot_value_[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	bool snap_mode_ = false;
	bool snapped_[4] = {false, false, false, false};

	int32_t adc_lp_[kNumAdcChannels] = {0, 0, 0, 0};
	int32_t adc_value_[kNumAdcChannels] = {0, 0, 0, 0};
	int32_t adc_threshold_[kNumAdcChannels] = {0, 0, 0, 0};
	long long press_time_[2] = {0, 0};

	SchmittTrigger switches_[2];

	peaks::IOBuffer ioBuffer;

	peaks::GateFlags gate_flags[2] = {0, 0};

	SampleRateConverter<2> outputSrc;
	DoubleRingBuffer<Frame<2>, 256> outputBuffer;

	bool initNumberStation = false;

	peaks::Processors processors[2];

	Peaks() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		settings_.edit_mode = EDIT_MODE_TWIN;
		settings_.function[0] = FUNCTION_ENVELOPE;
		settings_.function[1] = FUNCTION_ENVELOPE;
		settings_.snap_mode = false;
		std::fill(&settings_.pot_value[0], &settings_.pot_value[8], 0);

		memset(&ioBuffer, 0, sizeof(ioBuffer));
		memset(&processors[0], 0, sizeof(processors[0]));
		memset(&processors[1], 0, sizeof(processors[1]));
		ioBuffer.Init();
		processors[0].Init(0);
		processors[1].Init(1);
	}

	void onReset() override {
		init();
	}

	void init() {
		std::fill(&pot_value_[0], &pot_value_[8], 0);
		std::fill(&press_time_[0], &press_time_[1], 0);
		std::fill(&gBrightness[0], &gBrightness[1], 0);
		std::fill(&adc_lp_[0], &adc_lp_[kNumAdcChannels], 0);
		std::fill(&adc_value_[0], &adc_value_[kNumAdcChannels], 0);
		std::fill(&adc_threshold_[0], &adc_threshold_[kNumAdcChannels], 0);
		std::fill(&snapped_[0], &snapped_[kNumAdcChannels], false);

		edit_mode_ = static_cast<EditMode>(settings_.edit_mode);
		function_[0] = static_cast<Function>(settings_.function[0]);
		function_[1] = static_cast<Function>(settings_.function[1]);
		std::copy(&settings_.pot_value[0], &settings_.pot_value[8], &pot_value_[0]);

		if (edit_mode_ == EDIT_MODE_FIRST || edit_mode_ == EDIT_MODE_SECOND) {
			lockPots();
			for (uint8_t i = 0; i < 4; ++i) {
				processors[0].set_parameter(
				    i,
				    static_cast<uint16_t>(pot_value_[i]) << 8);
				processors[1].set_parameter(
				    i,
				    static_cast<uint16_t>(pot_value_[i + 4]) << 8);
			}
		}

		snap_mode_ = settings_.snap_mode;

		changeControlMode();
		setFunction(0, function_[0]);
		setFunction(1, function_[1]);
	}

	json_t *toJson() override {

		saveState();

		json_t *rootJ = json_object();

		json_object_set_new(rootJ, "edit_mode", json_integer((int)settings_.edit_mode));
		json_object_set_new(rootJ, "fcn_channel_1", json_integer((int)settings_.function[0]));
		json_object_set_new(rootJ, "fcn_channel_2", json_integer((int)settings_.function[1]));

		json_t *potValuesJ = json_array();
		for (int p : pot_value_) {
			json_t *pJ = json_integer(p);
			json_array_append_new(potValuesJ, pJ);
		}
		json_object_set_new(rootJ, "pot_values", potValuesJ);

		json_object_set_new(rootJ, "snap_mode", json_boolean(settings_.snap_mode));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *editModeJ = json_object_get(rootJ, "edit_mode");
		if (editModeJ) {
			settings_.edit_mode = static_cast<EditMode>(json_integer_value(editModeJ));
		}

		json_t *fcnChannel1J = json_object_get(rootJ, "fcn_channel_1");
		if (fcnChannel1J) {
			settings_.function[0] = static_cast<Function>(json_integer_value(fcnChannel1J));
		}

		json_t *fcnChannel2J = json_object_get(rootJ, "fcn_channel_2");
		if (fcnChannel2J) {
			settings_.function[1] = static_cast<Function>(json_integer_value(fcnChannel2J));
		}

		json_t *snapModeJ = json_object_get(rootJ, "snap_mode");
		if (snapModeJ) {
			settings_.snap_mode = json_boolean_value(snapModeJ);
		}

		json_t *potValuesJ = json_object_get(rootJ, "pot_values");
		size_t potValueId;
		json_t *pJ;
		json_array_foreach(potValuesJ, potValueId, pJ) {
			if (potValueId < sizeof(pot_value_) / sizeof(pot_value_)[0]) {
				settings_.pot_value[potValueId] = json_integer_value(pJ);
			}
		}

		// Update module internal state from settings.
		init();
	}

	void step() override {
		poll();
		pollPots();

		// Initialize "secret" number station mode.
		if (initNumberStation) {
			processors[0].set_function(peaks::PROCESSOR_FUNCTION_NUMBER_STATION);
			processors[1].set_function(peaks::PROCESSOR_FUNCTION_NUMBER_STATION);
			initNumberStation = false;
		}

		if (outputBuffer.empty()) {
			ioBuffer.Process(process);

			uint32_t external_gate_inputs = 0;
			external_gate_inputs |= (inputs[GATE_1_INPUT].value ? 1 : 0);
			external_gate_inputs |= (inputs[GATE_2_INPUT].value ? 2 : 0);

			uint32_t buttons = 0;
			buttons |= (params[TRIG_1_PARAM].value ? 1 : 0);
			buttons |= (params[TRIG_2_PARAM].value ? 2 : 0);

			uint32_t gate_inputs = external_gate_inputs | buttons;

			// Prepare sample rate conversion.
			// Peaks is sampling at 48kHZ.
			outputSrc.setRates(48000, engineGetSampleRate());
			int inLen = peaks::kBlockSize;
			int outLen = outputBuffer.capacity();
			Frame<2> f[peaks::kBlockSize];

			// Process an entire block of data from the IOBuffer.
			for (size_t k = 0; k < peaks::kBlockSize; ++k) {

				peaks::IOBuffer::Slice slice = ioBuffer.NextSlice(1);

				for (size_t i = 0; i < peaks::kNumChannels; ++i) {
					gate_flags[i] = peaks::ExtractGateFlags(
					                    gate_flags[i],
					                    gate_inputs & (1 << i));

					f[k].samples[i] = slice.block->output[i][slice.frame_index];
				}

				// A hack to make channel 1 aware of what's going on in channel 2. Used to
				// reset the sequencer.
				slice.block->input[0][slice.frame_index] = gate_flags[0] | (gate_flags[1] << 4) | (buttons & 8 ? peaks::GATE_FLAG_FROM_BUTTON : 0);

				slice.block->input[1][slice.frame_index] = gate_flags[1] | (buttons & 2 ? peaks::GATE_FLAG_FROM_BUTTON : 0);
			}

			outputSrc.process(f, &inLen, outputBuffer.endData(), &outLen);
			outputBuffer.endIncr(outLen);
		}

		// Update outputs.
		if (!outputBuffer.empty()) {
			Frame<2> f = outputBuffer.shift();

			// Peaks manual says output spec is 0..8V for envelopes and 10Vpp for audio/CV.
			// TODO Check the output values against an actual device.
			outputs[OUT_1_OUTPUT].value = rescale(static_cast<float>(f.samples[0]), 0.0f, 65535.f, -8.0f, 8.0f);
			outputs[OUT_2_OUTPUT].value = rescale(static_cast<float>(f.samples[1]), 0.0f, 65535.f, -8.0f, 8.0f);
		}
	}

	inline Function function() const {
		return edit_mode_ == EDIT_MODE_SECOND ? function_[1] : function_[0];
	}

	void changeControlMode();
	void setFunction(uint8_t index, Function f);
	void onPotChanged(uint16_t id, uint16_t value);
	void onSwitchReleased(uint16_t id, uint16_t data);
	void saveState();
	void lockPots();
	void poll();
	void pollPots();
	void refreshLeds();

	long long getSystemTimeMs();
};

const peaks::ProcessorFunction Peaks::function_table_[FUNCTION_LAST][2] = {
	{ peaks::PROCESSOR_FUNCTION_ENVELOPE, peaks::PROCESSOR_FUNCTION_ENVELOPE },
	{ peaks::PROCESSOR_FUNCTION_LFO, peaks::PROCESSOR_FUNCTION_LFO },
	{ peaks::PROCESSOR_FUNCTION_TAP_LFO, peaks::PROCESSOR_FUNCTION_TAP_LFO },
	{ peaks::PROCESSOR_FUNCTION_BASS_DRUM, peaks::PROCESSOR_FUNCTION_SNARE_DRUM },

	{ peaks::PROCESSOR_FUNCTION_MINI_SEQUENCER, peaks::PROCESSOR_FUNCTION_MINI_SEQUENCER },
	{ peaks::PROCESSOR_FUNCTION_PULSE_SHAPER, peaks::PROCESSOR_FUNCTION_PULSE_SHAPER },
	{ peaks::PROCESSOR_FUNCTION_PULSE_RANDOMIZER, peaks::PROCESSOR_FUNCTION_PULSE_RANDOMIZER },
	{ peaks::PROCESSOR_FUNCTION_FM_DRUM, peaks::PROCESSOR_FUNCTION_FM_DRUM },
};


void Peaks::changeControlMode() {
	uint16_t parameters[4];
	for (int i = 0; i < 4; ++i) {
		parameters[i] = adc_value_[i];
	}

	if (edit_mode_ == EDIT_MODE_SPLIT) {
		processors[0].CopyParameters(&parameters[0], 2);
		processors[1].CopyParameters(&parameters[2], 2);
		processors[0].set_control_mode(peaks::CONTROL_MODE_HALF);
		processors[1].set_control_mode(peaks::CONTROL_MODE_HALF);
	}
	else if (edit_mode_ == EDIT_MODE_TWIN) {
		processors[0].CopyParameters(&parameters[0], 4);
		processors[1].CopyParameters(&parameters[0], 4);
		processors[0].set_control_mode(peaks::CONTROL_MODE_FULL);
		processors[1].set_control_mode(peaks::CONTROL_MODE_FULL);
	}
	else {
		processors[0].set_control_mode(peaks::CONTROL_MODE_FULL);
		processors[1].set_control_mode(peaks::CONTROL_MODE_FULL);
	}
}

void Peaks::setFunction(uint8_t index, Function f) {
	if (edit_mode_ == EDIT_MODE_SPLIT || edit_mode_ == EDIT_MODE_TWIN) {
		function_[0] = function_[1] = f;
		processors[0].set_function(function_table_[f][0]);
		processors[1].set_function(function_table_[f][1]);
	}
	else {
		function_[index] = f;
		processors[index].set_function(function_table_[f][index]);
	}
}

void Peaks::onSwitchReleased(uint16_t id, uint16_t data) {
	switch (id) {
	case SWITCH_TWIN_MODE:
		if (data > kLongPressDuration) {
			edit_mode_ = static_cast<EditMode>(
			                 (edit_mode_ + EDIT_MODE_FIRST) % EDIT_MODE_LAST);
			function_[0] = function_[1];
			processors[0].set_function(function_table_[function_[0]][0]);
			processors[1].set_function(function_table_[function_[0]][1]);
			lockPots();
		}
		else {
			if (edit_mode_ <= EDIT_MODE_SPLIT) {
				edit_mode_ = static_cast<EditMode>(EDIT_MODE_SPLIT - edit_mode_);
			}
			else {
				edit_mode_ = static_cast<EditMode>(EDIT_MODE_SECOND - (edit_mode_ & 1));
				lockPots();
			}
		}
		changeControlMode();
		saveState();
		break;

	case SWITCH_FUNCTION: {
		Function f = function();
		if (data > kLongPressDuration) {
			f = static_cast<Function>((f + FUNCTION_FIRST_ALTERNATE_FUNCTION) % FUNCTION_LAST);
		}
		else {
			if (f <= FUNCTION_DRUM_GENERATOR) {
				f = static_cast<Function>((f + 1) & 3);
			}
			else {
				f = static_cast<Function>(((f + 1) & 3) + FUNCTION_FIRST_ALTERNATE_FUNCTION);
			}
		}
		setFunction(edit_mode_ - EDIT_MODE_FIRST, f);
		saveState();
	}
	break;

	case SWITCH_GATE_TRIG_1:
		// no-op
		break;

	case SWITCH_GATE_TRIG_2:
		// no-op
		break;
	}
}

void Peaks::lockPots() {
	std::fill(
	    &adc_threshold_[0],
	    &adc_threshold_[kNumAdcChannels],
	    kAdcThresholdLocked);
	std::fill(&snapped_[0], &snapped_[kNumAdcChannels], false);
}

void Peaks::pollPots() {
	for (uint8_t i = 0; i < kNumAdcChannels; ++i) {
		adc_lp_[i] = (int32_t(params[KNOB_1_PARAM + i].value) + adc_lp_[i] * 7) >> 3;
		int32_t value = adc_lp_[i];
		int32_t current_value = adc_value_[i];
		if (value >= current_value + adc_threshold_[i] ||
		        value <= current_value - adc_threshold_[i] ||
		        !adc_threshold_[i]) {
			onPotChanged(i, value);
			adc_value_[i] = value;
			adc_threshold_[i] = kAdcThresholdUnlocked;
		}
	}
}

void Peaks::onPotChanged(uint16_t id, uint16_t value) {
	switch (edit_mode_) {
	case EDIT_MODE_TWIN:
		processors[0].set_parameter(id, value);
		processors[1].set_parameter(id, value);
		pot_value_[id] = value >> 8;
		break;

	case EDIT_MODE_SPLIT:
		if (id < 2) {
			processors[0].set_parameter(id, value);
		}
		else {
			processors[1].set_parameter(id - 2, value);
		}
		pot_value_[id] = value >> 8;
		break;

	case EDIT_MODE_FIRST:
	case EDIT_MODE_SECOND: {
		uint8_t index = id + (edit_mode_ - EDIT_MODE_FIRST) * 4;
		peaks::Processors* p = &processors[edit_mode_ - EDIT_MODE_FIRST];

		int16_t delta = static_cast<int16_t>(pot_value_[index]) - \
		                static_cast<int16_t>(value >> 8);
		if (delta < 0) {
			delta = -delta;
		}

		if (!snap_mode_ || snapped_[id] || delta <= 2) {
			p->set_parameter(id, value);
			pot_value_[index] = value >> 8;
			snapped_[id] = true;
		}
	}
	break;

	case EDIT_MODE_LAST:
		break;
	}
}

long long Peaks::getSystemTimeMs() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
	           std::chrono::steady_clock::now().time_since_epoch()
	       ).count();
}

void Peaks::poll() {
	for (uint8_t i = 0; i < 2; ++i) {
		if (switches_[i].process(params[BUTTON_1_PARAM + i].value)) {
			press_time_[i] = getSystemTimeMs();
		}

		if (switches_[i].isHigh() && press_time_[i] != 0) {
			int32_t pressed_time = getSystemTimeMs() - press_time_[i];
			if (pressed_time > kLongPressDuration) {
				onSwitchReleased(SWITCH_TWIN_MODE + i, pressed_time);
				press_time_[i] = 0;  // Inhibit next release event
			}
		}
		if (!switches_[i].isHigh() && press_time_[i] != 0) {
			int32_t delta = getSystemTimeMs() - press_time_[i] + 1;
			onSwitchReleased(SWITCH_TWIN_MODE + i, delta);
			press_time_[i] = 0; // Not in original code!
		}
	}

	refreshLeds();
}

void Peaks::saveState() {
	settings_.edit_mode = edit_mode_;
	settings_.function[0] = function_[0];
	settings_.function[1] = function_[1];
	std::copy(&pot_value_[0], &pot_value_[8], &settings_.pot_value[0]);
	settings_.snap_mode = snap_mode_;
}

void Peaks::refreshLeds() {
	uint8_t flash = (getSystemTimeMs() >> 7) & 7;
	switch (edit_mode_) {
	case EDIT_MODE_FIRST:
		lights[TWIN_MODE_LIGHT].value = (flash == 1) ? 1.0f : 0.0f;
		break;
	case EDIT_MODE_SECOND:
		lights[TWIN_MODE_LIGHT].value = (flash == 1 || flash == 3) ? 1.0f : 0.0f;
		break;
	default:
		lights[TWIN_MODE_LIGHT].value = (edit_mode_ & 1) ? 1.0f : 0.0f;
		break;
	}
	if ((getSystemTimeMs() & 256) && function() >= FUNCTION_FIRST_ALTERNATE_FUNCTION) {
		for (size_t i = 0; i < 4; ++i) {
			lights[FUNC_1_LIGHT + i].value = 0.0f;
		}
	}
	else {
		for (size_t i = 0; i < 4; ++i) {
			lights[FUNC_1_LIGHT + i].value = ((function() & 3) == i) ? 1.0f : 0.0f;
		}
	}

	uint8_t b[2];
	for (uint8_t i = 0; i < 2; ++i) {
		switch (function_[i]) {
		case FUNCTION_DRUM_GENERATOR:
		case FUNCTION_FM_DRUM_GENERATOR:
			b[i] = (int16_t) std::abs(gBrightness[i]) >> 8;
			b[i] = b[i] >= 255 ? 255 : b[i];
			break;
		case FUNCTION_LFO:
		case FUNCTION_TAP_LFO:
		case FUNCTION_MINI_SEQUENCER: {
			int32_t brightness = int32_t(gBrightness[i]) * 409 >> 8;
			brightness += 32768;
			brightness >>= 8;
			CONSTRAIN(brightness, 0, 255);
			b[i] = brightness;
		}
		break;
		default:
			b[i] = gBrightness[i] >> 7;
			break;
		}
	}

	if (processors[0].function() == peaks::PROCESSOR_FUNCTION_NUMBER_STATION) {
		uint8_t pattern = processors[0].number_station().digit()
			^ processors[1].number_station().digit();
		for (size_t i = 0; i < 4; ++i) {
			lights[FUNC_1_LIGHT + i].value = (pattern & 1) ? 1.0f : 0.0f;
			pattern = pattern >> 1;
		}
		b[0] = processors[0].number_station().gate() ? 255 : 0;
		b[1] = processors[1].number_station().gate() ? 255 : 0;
	}

	lights[TRIG_1_LIGHT].value = rescale(static_cast<float>(b[0]), 0.0f, 255.0f, 0.0f, 1.0f);
	lights[TRIG_2_LIGHT].value = rescale(static_cast<float>(b[1]), 0.0f, 255.0f, 0.0f, 1.0f);
}


struct PeaksWidget : ModuleWidget {
	PeaksWidget(Peaks *module) : ModuleWidget(module) {
#ifdef USE_VST2
		setPanel(SVG::load(assetStaticPlugin("AudibleInstruments", "res/Peaks.svg")));
#else
		setPanel(SVG::load(assetPlugin(plugin, "res/Peaks.svg")));
#endif // USE_VST2

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

		addParam(ParamWidget::create<TL1105>(Vec(8.5, 52), module, Peaks::BUTTON_1_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(11.88, 74), module, Peaks::TWIN_MODE_LIGHT));
		addParam(ParamWidget::create<TL1105>(Vec(8.5, 89), module, Peaks::BUTTON_2_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(11.88, 111), module, Peaks::FUNC_1_LIGHT));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(11.88, 126.75), module, Peaks::FUNC_2_LIGHT));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(11.88, 142.5), module, Peaks::FUNC_3_LIGHT));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(11.88, 158), module, Peaks::FUNC_4_LIGHT));

		addParam(ParamWidget::create<Rogan1PSWhite>(Vec(61, 51), module, Peaks::KNOB_1_PARAM, 0.0f, 65535.0f, 16384.0f));
		addParam(ParamWidget::create<Rogan1PSWhite>(Vec(61, 115), module, Peaks::KNOB_2_PARAM, 0.0f, 65535.0f, 16384.0f));
		addParam(ParamWidget::create<Rogan1PSWhite>(Vec(61, 179), module, Peaks::KNOB_3_PARAM, 0.0f, 65535.0f, 32678.0f));
		addParam(ParamWidget::create<Rogan1PSWhite>(Vec(61, 244), module, Peaks::KNOB_4_PARAM, 0.0f, 65535.0f, 32678.0f));

		addParam(ParamWidget::create<LEDBezel>(Vec(11, 188), module, Peaks::TRIG_1_PARAM, 0.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<LEDBezel>(Vec(11, 273), module, Peaks::TRIG_2_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<LEDBezelLight<GreenLight>>(Vec(11, 188).plus(mm2px(Vec(0.75, 0.75))), module, Peaks::TRIG_1_LIGHT));
		addChild(ModuleLightWidget::create<LEDBezelLight<GreenLight>>(Vec(11, 273).plus(mm2px(Vec(0.75, 0.75))), module, Peaks::TRIG_2_LIGHT));

		addInput(Port::create<PJ301MPort>(Vec(10, 230), Port::INPUT, module, Peaks::GATE_1_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(10, 315), Port::INPUT, module, Peaks::GATE_2_INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(53, 315), Port::OUTPUT, module, Peaks::OUT_1_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(86, 315), Port::OUTPUT, module, Peaks::OUT_2_OUTPUT));
	}

	Menu *createContextMenu() override {
		Menu *menu = ModuleWidget::createContextMenu();
		Peaks *peaks = dynamic_cast<Peaks*>(this->module);

		struct SnapModeItem : MenuItem {
			Peaks *peaks;
			void onAction(EventAction &e) override {
				peaks->snap_mode_ = !peaks->snap_mode_;
			}
			void step() override {
				rightText = (peaks->snap_mode_) ? "✔" : "";
				MenuItem::step();
			}
		};

		struct NumberStationItem : MenuItem {
			Peaks *peaks;
			void onAction(EventAction &e) override {
				peaks->initNumberStation = true;
			}
		};

		menu->addChild(construct<MenuLabel>());
		menu->addChild(construct<SnapModeItem>(&SnapModeItem::text, "Snap Mode", &SnapModeItem::peaks, peaks));

		menu->addChild(construct<MenuLabel>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Secret Modes"));
		menu->addChild(construct<NumberStationItem>(&NumberStationItem::text, "Number Station", &NumberStationItem::peaks, peaks));

		return menu;
	}
};



Model *modelPeaks = Model::create<Peaks, PeaksWidget>("Audible Instruments", "Peaks", "Percussive Synthesizer", UTILITY_TAG, LFO_TAG, DRUM_TAG);
