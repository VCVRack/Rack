#include "AudibleInstruments.hpp"
#include "dsp/digital.hpp"
#include "marbles/random/random_generator.h"
#include "marbles/random/random_stream.h"
#include "marbles/random/t_generator.h"
#include "marbles/random/x_y_generator.h"
#include "marbles/note_filter.h"


static const int BLOCK_SIZE = 5;


static const marbles::Scale preset_scales[6] = {
	// C major
	{
		1.0f,
		12,
		{
			{ 0.0000f, 255 },  // C
			{ 0.0833f, 16 },   // C#
			{ 0.1667f, 96 },   // D
			{ 0.2500f, 24 },   // D#
			{ 0.3333f, 128 },  // E
			{ 0.4167f, 64 },   // F
			{ 0.5000f, 8 },    // F#
			{ 0.5833f, 192 },  // G
			{ 0.6667f, 16 },   // G#
			{ 0.7500f, 96 },   // A
			{ 0.8333f, 24 },   // A#
			{ 0.9167f, 128 },  // B
		}
	},

	// C minor
	{
		1.0f,
		12,
		{
			{ 0.0000f, 255 },  // C
			{ 0.0833f, 16 },   // C#
			{ 0.1667f, 96 },   // D
			{ 0.2500f, 128 },  // Eb
			{ 0.3333f, 8 },    // E
			{ 0.4167f, 64 },   // F
			{ 0.5000f, 4 },    // F#
			{ 0.5833f, 192 },  // G
			{ 0.6667f, 16 },   // G#
			{ 0.7500f, 96 },   // A
			{ 0.8333f, 128 },  // Bb
			{ 0.9167f, 16 },   // B
		}
	},

	// Pentatonic
	{
		1.0f,
		12,
		{
			{ 0.0000f, 255 },  // C
			{ 0.0833f, 4 },    // C#
			{ 0.1667f, 96 },   // D
			{ 0.2500f, 4 },    // Eb
			{ 0.3333f, 4 },    // E
			{ 0.4167f, 140 },  // F
			{ 0.5000f, 4 },    // F#
			{ 0.5833f, 192 },  // G
			{ 0.6667f, 4 },    // G#
			{ 0.7500f, 96 },   // A
			{ 0.8333f, 4 },    // Bb
			{ 0.9167f, 4 },    // B
		}
	},

	// Pelog
	{
		1.0f,
		7,
		{
			{ 0.0000f, 255 },  // C
			{ 0.1275f, 128 },  // Db+
			{ 0.2625f, 32 },  // Eb-
			{ 0.4600f, 8 },    // F#-
			{ 0.5883f, 192 },  // G
			{ 0.7067f, 64 },  // Ab
			{ 0.8817f, 16 },    // Bb+
		}
	},

	// Raag Bhairav That
	{
		1.0f,
		12,
		{
			{ 0.0000f, 255 }, // ** Sa
			{ 0.0752f, 128 }, // ** Komal Re
			{ 0.1699f, 4 },   //    Re
			{ 0.2630f, 4 },   //    Komal Ga
			{ 0.3219f, 128 }, // ** Ga
			{ 0.4150f, 64 },  // ** Ma
			{ 0.4918f, 4 },   //    Tivre Ma
			{ 0.5850f, 192 }, // ** Pa
			{ 0.6601f, 64 },  // ** Komal Dha
			{ 0.7549f, 4 },   //    Dha
			{ 0.8479f, 4 },   //    Komal Ni
			{ 0.9069f, 64 },  // ** Ni
		}
	},

	// Raag Shri
	{
		1.0f,
		12,
		{
			{ 0.0000f, 255 }, // ** Sa
			{ 0.0752f, 4 },   //    Komal Re
			{ 0.1699f, 128 }, // ** Re
			{ 0.2630f, 64 },  // ** Komal Ga
			{ 0.3219f, 4 },   //    Ga
			{ 0.4150f, 128 }, // ** Ma
			{ 0.4918f, 4 },   //    Tivre Ma
			{ 0.5850f, 192 }, // ** Pa
			{ 0.6601f, 4 },   //    Komal Dha
			{ 0.7549f, 64 },  // ** Dha
			{ 0.8479f, 128 }, // ** Komal Ni
			{ 0.9069f, 4 },   //    Ni
		}
	},
};


struct Marbles : Module {
	enum ParamIds {
		T_DEJA_VU_PARAM,
		X_DEJA_VU_PARAM,
		DEJA_VU_PARAM,
		T_RATE_PARAM,
		X_SPREAD_PARAM,
		T_MODE_PARAM,
		X_MODE_PARAM,
		DEJA_VU_LENGTH_PARAM,
		T_BIAS_PARAM,
		X_BIAS_PARAM,
		T_RANGE_PARAM,
		X_RANGE_PARAM,
		EXTERNAL_PARAM,
		T_JITTER_PARAM,
		X_STEPS_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		T_BIAS_INPUT,
		X_BIAS_INPUT,
		T_CLOCK_INPUT,
		T_RATE_INPUT,
		T_JITTER_INPUT,
		DEJA_VU_INPUT,
		X_STEPS_INPUT,
		X_SPREAD_INPUT,
		X_CLOCK_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		T1_OUTPUT,
		T2_OUTPUT,
		T3_OUTPUT,
		Y_OUTPUT,
		X1_OUTPUT,
		X2_OUTPUT,
		X3_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		T_DEJA_VU_LIGHT,
		X_DEJA_VU_LIGHT,
		ENUMS(T_MODE_LIGHTS, 2),
		ENUMS(X_MODE_LIGHTS, 2),
		ENUMS(T_RANGE_LIGHTS, 2),
		ENUMS(X_RANGE_LIGHTS, 2),
		EXTERNAL_LIGHT,
		T1_LIGHT,
		T2_LIGHT,
		T3_LIGHT,
		Y_LIGHT,
		X1_LIGHT,
		X2_LIGHT,
		X3_LIGHT,
		NUM_LIGHTS
	};

	marbles::RandomGenerator random_generator;
	marbles::RandomStream random_stream;
	marbles::TGenerator t_generator;
	marbles::XYGenerator xy_generator;
	marbles::NoteFilter note_filter;

	// State
	BooleanTrigger tDejaVuTrigger;
	BooleanTrigger xDejaVuTrigger;
	BooleanTrigger tModeTrigger;
	BooleanTrigger xModeTrigger;
	BooleanTrigger tRangeTrigger;
	BooleanTrigger xRangeTrigger;
	BooleanTrigger externalTrigger;
	bool t_deja_vu;
	bool x_deja_vu;
	int t_mode;
	int x_mode;
	int t_range;
	int x_range;
	bool external;
	int x_scale;
	int y_divider_index;
	int x_clock_source_internal;

	// Buffers
	stmlib::GateFlags t_clocks[BLOCK_SIZE] = {};
	stmlib::GateFlags last_t_clock = 0;
	stmlib::GateFlags xy_clocks[BLOCK_SIZE] = {};
	stmlib::GateFlags last_xy_clock = 0;
	float ramp_master[BLOCK_SIZE] = {};
	float ramp_external[BLOCK_SIZE] = {};
	float ramp_slave[2][BLOCK_SIZE] = {};
	bool gates[BLOCK_SIZE * 2] = {};
	float voltages[BLOCK_SIZE * 4] = {};
	int blockIndex = 0;

	Marbles() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		random_generator.Init(1);
		random_stream.Init(&random_generator);
		note_filter.Init();
		onSampleRateChange();
		onReset();
	}

	void onReset() override {
		t_deja_vu = false;
		x_deja_vu = false;
		t_mode = 0;
		x_mode = 0;
		t_range = 1;
		x_range = 1;
		external = false;
		x_scale = 0;
		y_divider_index = 8;
		x_clock_source_internal = 0;
	}

	void onRandomize() override {
		t_mode = randomu32() % 3;
		x_mode = randomu32() % 3;
		t_range = randomu32() % 3;
		x_range = randomu32() % 3;
	}

	void onSampleRateChange() override {
		float sampleRate = engineGetSampleRate();
		t_generator.Init(&random_stream, sampleRate);
		xy_generator.Init(&random_stream, sampleRate);

		// Set scales
		for (int i = 0; i < 6; i++) {
			xy_generator.LoadScale(i, preset_scales[i]);
		}
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();

		json_object_set_new(rootJ, "t_deja_vu", json_boolean(t_deja_vu));
		json_object_set_new(rootJ, "x_deja_vu", json_boolean(x_deja_vu));
		json_object_set_new(rootJ, "t_mode", json_integer(t_mode));
		json_object_set_new(rootJ, "x_mode", json_integer(x_mode));
		json_object_set_new(rootJ, "t_range", json_integer(t_range));
		json_object_set_new(rootJ, "x_range", json_integer(x_range));
		json_object_set_new(rootJ, "external", json_boolean(external));
		json_object_set_new(rootJ, "x_scale", json_integer(x_scale));
		json_object_set_new(rootJ, "y_divider_index", json_integer(y_divider_index));
		json_object_set_new(rootJ, "x_clock_source_internal", json_integer(x_clock_source_internal));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *t_deja_vuJ = json_object_get(rootJ, "t_deja_vu");
		if (t_deja_vuJ)
			t_deja_vu = json_boolean_value(t_deja_vuJ);

		json_t *x_deja_vuJ = json_object_get(rootJ, "x_deja_vu");
		if (x_deja_vuJ)
			x_deja_vu = json_boolean_value(x_deja_vuJ);

		json_t *t_modeJ = json_object_get(rootJ, "t_mode");
		if (t_modeJ)
			t_mode = json_integer_value(t_modeJ);

		json_t *x_modeJ = json_object_get(rootJ, "x_mode");
		if (x_modeJ)
			x_mode = json_integer_value(x_modeJ);

		json_t *t_rangeJ = json_object_get(rootJ, "t_range");
		if (t_rangeJ)
			t_range = json_integer_value(t_rangeJ);

		json_t *x_rangeJ = json_object_get(rootJ, "x_range");
		if (x_rangeJ)
			x_range = json_integer_value(x_rangeJ);

		json_t *externalJ = json_object_get(rootJ, "external");
		if (externalJ)
			external = json_boolean_value(externalJ);

		json_t *x_scaleJ = json_object_get(rootJ, "x_scale");
		if (x_scaleJ)
			x_scale = json_integer_value(x_scaleJ);

		json_t *y_divider_indexJ = json_object_get(rootJ, "y_divider_index");
		if (y_divider_indexJ)
			y_divider_index = json_integer_value(y_divider_indexJ);

		json_t *x_clock_source_internalJ = json_object_get(rootJ, "x_clock_source_internal");
		if (x_clock_source_internalJ)
			x_clock_source_internal = json_integer_value(x_clock_source_internalJ);
	}

	void step() override {
		// Buttons
		if (tDejaVuTrigger.process(params[T_DEJA_VU_PARAM].value <= 0.f)) {
			t_deja_vu = !t_deja_vu;
		}
		if (xDejaVuTrigger.process(params[X_DEJA_VU_PARAM].value <= 0.f)) {
			x_deja_vu = !x_deja_vu;
		}
		if (tModeTrigger.process(params[T_MODE_PARAM].value <= 0.f)) {
			t_mode = (t_mode + 1) % 3;
		}
		if (xModeTrigger.process(params[X_MODE_PARAM].value <= 0.f)) {
			x_mode = (x_mode + 1) % 3;
		}
		if (tRangeTrigger.process(params[T_RANGE_PARAM].value <= 0.f)) {
			t_range = (t_range + 1) % 3;
		}
		if (xRangeTrigger.process(params[X_RANGE_PARAM].value <= 0.f)) {
			x_range = (x_range + 1) % 3;
		}
		if (externalTrigger.process(params[EXTERNAL_PARAM].value <= 0.f)) {
			external = !external;
		}

		// Clocks
		bool t_gate = (inputs[T_CLOCK_INPUT].value >= 1.7f);
		last_t_clock = stmlib::ExtractGateFlags(last_t_clock, t_gate);
		t_clocks[blockIndex] = last_t_clock;

		bool x_gate = (inputs[X_CLOCK_INPUT].value >= 1.7f);
		last_xy_clock = stmlib::ExtractGateFlags(last_xy_clock, x_gate);
		xy_clocks[blockIndex] = last_xy_clock;

		// Process block
		if (++blockIndex >= BLOCK_SIZE) {
			blockIndex = 0;
			stepBlock();
		}

		// Lights and outputs

		lights[T_DEJA_VU_LIGHT].setBrightness(t_deja_vu);
		lights[X_DEJA_VU_LIGHT].setBrightness(x_deja_vu);

		lights[T_MODE_LIGHTS + 0].setBrightness(t_mode == 0 || t_mode == 1);
		lights[T_MODE_LIGHTS + 1].setBrightness(t_mode == 1 || t_mode == 2);

		lights[X_MODE_LIGHTS + 0].setBrightness(x_mode == 0 || x_mode == 1);
		lights[X_MODE_LIGHTS + 1].setBrightness(x_mode == 1 || x_mode == 2);

		lights[T_RANGE_LIGHTS + 0].setBrightness(t_range == 0 || t_range == 1);
		lights[T_RANGE_LIGHTS + 1].setBrightness(t_range == 1 || t_range == 2);

		lights[X_RANGE_LIGHTS + 0].setBrightness(x_range == 0 || x_range == 1);
		lights[X_RANGE_LIGHTS + 1].setBrightness(x_range == 1 || x_range == 2);

		lights[EXTERNAL_LIGHT].setBrightness(external);

		outputs[T1_OUTPUT].value = gates[blockIndex*2 + 0] ? 10.f : 0.f;
		lights[T1_LIGHT].setBrightnessSmooth(gates[blockIndex*2 + 0]);
		outputs[T2_OUTPUT].value = (ramp_master[blockIndex] < 0.5f) ? 10.f : 0.f;
		lights[T2_LIGHT].setBrightnessSmooth(ramp_master[blockIndex] < 0.5f);
		outputs[T3_OUTPUT].value = gates[blockIndex*2 + 1] ? 10.f : 0.f;
		lights[T3_LIGHT].setBrightnessSmooth(gates[blockIndex*2 + 1]);

		outputs[X1_OUTPUT].value = voltages[blockIndex*4 + 0];
		lights[X1_LIGHT].setBrightnessSmooth(voltages[blockIndex*4 + 0]);
		outputs[X2_OUTPUT].value = voltages[blockIndex*4 + 1];
		lights[X2_LIGHT].setBrightnessSmooth(voltages[blockIndex*4 + 1]);
		outputs[X3_OUTPUT].value = voltages[blockIndex*4 + 2];
		lights[X3_LIGHT].setBrightnessSmooth(voltages[blockIndex*4 + 2]);
		outputs[Y_OUTPUT].value = voltages[blockIndex*4 + 3];
		lights[Y_LIGHT].setBrightnessSmooth(voltages[blockIndex*4 + 3]);
	}

	void stepBlock() {
		// Ramps

		marbles::Ramps ramps;
		ramps.master = ramp_master;
		ramps.external = ramp_external;
		ramps.slave[0] = ramp_slave[0];
		ramps.slave[1] = ramp_slave[1];

		float deja_vu = clamp(params[DEJA_VU_PARAM].value + inputs[DEJA_VU_INPUT].value / 5.f, 0.f, 1.f);
		static const int loop_length[] = {
			1, 1, 1, 2, 2,
			2, 2, 2, 3, 3,
			3, 3, 4, 4, 4,
			4, 4, 5, 5, 6,
			6, 6, 7, 7, 8,
			8, 8, 10, 10, 12,
			12, 12, 14, 14, 16,
			16
		};
		float deja_vu_length_index = params[DEJA_VU_LENGTH_PARAM].value * (LENGTHOF(loop_length) - 1);
		int deja_vu_length = loop_length[(int) roundf(deja_vu_length_index)];

		// Set up TGenerator

		bool t_external_clock = inputs[T_CLOCK_INPUT].active;

		t_generator.set_model((marbles::TGeneratorModel) t_mode);
		t_generator.set_range((marbles::TGeneratorRange) t_range);
		float t_rate = 60.f * (params[T_RATE_PARAM].value + inputs[T_RATE_INPUT].value / 5.f);
		t_generator.set_rate(t_rate);
		float t_bias = clamp(params[T_BIAS_PARAM].value + inputs[T_BIAS_INPUT].value / 5.f, 0.f, 1.f);
		t_generator.set_bias(t_bias);
		float t_jitter = clamp(params[T_JITTER_PARAM].value + inputs[T_JITTER_INPUT].value / 5.f, 0.f, 1.f);
		t_generator.set_jitter(t_jitter);
		t_generator.set_deja_vu(t_deja_vu ? deja_vu : 0.f);
		t_generator.set_length(deja_vu_length);
		// TODO
		t_generator.set_pulse_width_mean(0.f);
		t_generator.set_pulse_width_std(0.f);

		t_generator.Process(t_external_clock, t_clocks, ramps, gates, BLOCK_SIZE);

		// Set up XYGenerator

		marbles::ClockSource x_clock_source = (marbles::ClockSource) x_clock_source_internal;
		if (inputs[X_CLOCK_INPUT].active)
			x_clock_source = marbles::CLOCK_SOURCE_EXTERNAL;

		marbles::GroupSettings x;
		x.control_mode = (marbles::ControlMode) x_mode;
		x.voltage_range = (marbles::VoltageRange) x_range;
		// TODO Fix the scaling
		float note_cv = 0.5f * (params[X_SPREAD_PARAM].value + inputs[X_SPREAD_INPUT].value / 5.f);
		float u = note_filter.Process(0.5f * (note_cv + 1.f));
		x.register_mode = external;
		x.register_value = u;

		float x_spread = clamp(params[X_SPREAD_PARAM].value + inputs[X_SPREAD_INPUT].value / 5.f, 0.f, 1.f);
		x.spread = x_spread;
		float x_bias = clamp(params[X_BIAS_PARAM].value + inputs[X_BIAS_INPUT].value / 5.f, 0.f, 1.f);
		x.bias = x_bias;
		float x_steps = clamp(params[X_STEPS_PARAM].value + inputs[X_STEPS_INPUT].value / 5.f, 0.f, 1.f);
		x.steps = x_steps;
		x.deja_vu = x_deja_vu ? deja_vu : 0.f;
		x.length = deja_vu_length;
		x.ratio.p = 1;
		x.ratio.q = 1;
		x.scale_index = x_scale;

		marbles::GroupSettings y;
		y.control_mode = marbles::CONTROL_MODE_IDENTICAL;
		// TODO
		y.voltage_range = (marbles::VoltageRange) x_range;
		y.register_mode = false;
		y.register_value = 0.0f;
		// TODO
		y.spread = x_spread;
		y.bias = x_bias;
		y.steps = x_steps;
		y.deja_vu = 0.0f;
		y.length = 1;
		static const marbles::Ratio y_divider_ratios[] = {
			{ 1, 64 },
			{ 1, 48 },
			{ 1, 32 },
			{ 1, 24 },
			{ 1, 16 },
			{ 1, 12 },
			{ 1, 8 },
			{ 1, 6 },
			{ 1, 4 },
			{ 1, 3 },
			{ 1, 2 },
			{ 1, 1 },
		};
		y.ratio = y_divider_ratios[y_divider_index];
		y.scale_index = x_scale;

		xy_generator.Process(x_clock_source, x, y, xy_clocks, ramps, voltages, BLOCK_SIZE);
	}
};


template <typename BASE>
struct CKD6Light : BASE {
	CKD6Light() {
		this->box.size = Vec(22, 22);
	}
};


struct MarblesWidget : ModuleWidget {
	MarblesWidget(Marbles *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Marbles.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<CKD6>(mm2px(Vec(16.545, 17.794)), module, Marbles::T_DEJA_VU_PARAM, 0.0, 1.0, 0.0));
		addParam(createParamCentered<CKD6>(mm2px(Vec(74.845, 17.794)), module, Marbles::X_DEJA_VU_PARAM, 0.0, 1.0, 0.0));
		addParam(createParamCentered<Rogan2PSWhite>(mm2px(Vec(45.695, 22.244)), module, Marbles::DEJA_VU_PARAM, 0.0, 1.0, 0.5));
		addParam(createParamCentered<Rogan3PSWhite>(mm2px(Vec(23.467, 35.264)), module, Marbles::T_RATE_PARAM, -1.0, 1.0, 0.0));
		addParam(createParamCentered<Rogan3PSWhite>(mm2px(Vec(67.945, 35.243)), module, Marbles::X_SPREAD_PARAM, 0.0, 1.0, 0.5));
		addParam(createParamCentered<TL1105>(mm2px(Vec(6.945, 38.794)), module, Marbles::T_MODE_PARAM, 0.0, 1.0, 0.0));
		addParam(createParamCentered<TL1105>(mm2px(Vec(84.445, 38.793)), module, Marbles::X_MODE_PARAM, 0.0, 1.0, 0.0));
		addParam(createParamCentered<Rogan2PSWhite>(mm2px(Vec(45.695, 51.144)), module, Marbles::DEJA_VU_LENGTH_PARAM, 0.0, 1.0, 0.0));
		addParam(createParamCentered<Rogan2PSWhite>(mm2px(Vec(9.545, 58.394)), module, Marbles::T_BIAS_PARAM, 0.0, 1.0, 0.5));
		addParam(createParamCentered<Rogan2PSWhite>(mm2px(Vec(81.844, 58.394)), module, Marbles::X_BIAS_PARAM, 0.0, 1.0, 0.5));
		addParam(createParamCentered<TL1105>(mm2px(Vec(26.644, 59.694)), module, Marbles::T_RANGE_PARAM, 0.0, 1.0, 0.0));
		addParam(createParamCentered<TL1105>(mm2px(Vec(64.744, 59.694)), module, Marbles::X_RANGE_PARAM, 0.0, 1.0, 0.0));
		addParam(createParamCentered<TL1105>(mm2px(Vec(45.694, 67.294)), module, Marbles::EXTERNAL_PARAM, 0.0, 1.0, 0.0));
		addParam(createParamCentered<Rogan2PSWhite>(mm2px(Vec(31.544, 73.694)), module, Marbles::T_JITTER_PARAM, 0.0, 1.0, 0.0));
		addParam(createParamCentered<Rogan2PSWhite>(mm2px(Vec(59.845, 73.694)), module, Marbles::X_STEPS_PARAM, 0.0, 1.0, 0.5));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.545, 81.944)), module, Marbles::T_BIAS_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(81.844, 81.944)), module, Marbles::X_BIAS_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.545, 96.544)), module, Marbles::T_CLOCK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(21.595, 96.544)), module, Marbles::T_RATE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.644, 96.544)), module, Marbles::T_JITTER_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(45.695, 96.544)), module, Marbles::DEJA_VU_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(57.745, 96.544)), module, Marbles::X_STEPS_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(69.795, 96.544)), module, Marbles::X_SPREAD_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(81.844, 96.544)), module, Marbles::X_CLOCK_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(9.545, 111.144)), module, Marbles::T1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(21.595, 111.144)), module, Marbles::T2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(33.644, 111.144)), module, Marbles::T3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(45.695, 111.144)), module, Marbles::Y_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(57.745, 111.144)), module, Marbles::X1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(69.795, 111.144)), module, Marbles::X2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(81.844, 111.144)), module, Marbles::X3_OUTPUT));

		addChild(createLightCentered<CKD6Light<GreenLight>>(mm2px(Vec(16.545, 17.794)), module, Marbles::T_DEJA_VU_LIGHT));
		addChild(createLightCentered<CKD6Light<GreenLight>>(mm2px(Vec(74.845, 17.794)), module, Marbles::X_DEJA_VU_LIGHT));
		addChild(createLightCentered<MediumLight<GreenRedLight>>(mm2px(Vec(6.944, 29.894)), module, Marbles::T_MODE_LIGHTS));
		addChild(createLightCentered<MediumLight<GreenRedLight>>(mm2px(Vec(84.444, 29.894)), module, Marbles::X_MODE_LIGHTS));
		addChild(createLightCentered<MediumLight<GreenRedLight>>(mm2px(Vec(26.644, 53.994)), module, Marbles::T_RANGE_LIGHTS));
		addChild(createLightCentered<MediumLight<GreenRedLight>>(mm2px(Vec(64.744, 53.994)), module, Marbles::X_RANGE_LIGHTS));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(45.695, 76.194)), module, Marbles::EXTERNAL_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(6.044, 104.794)), module, Marbles::T1_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(18.094, 104.794)), module, Marbles::T2_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(30.145, 104.794)), module, Marbles::T3_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(42.194, 104.794)), module, Marbles::Y_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(54.244, 104.794)), module, Marbles::X1_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(66.294, 104.794)), module, Marbles::X2_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(78.344, 104.794)), module, Marbles::X3_LIGHT));
	}

	void appendContextMenu(Menu *menu) override {
		Marbles *module = dynamic_cast<Marbles*>(this->module);

		struct ScaleItem : MenuItem {
			Marbles *module;
			int scale;
			void onAction(EventAction &e) override {
				module->x_scale = scale;
			}
		};

		menu->addChild(MenuEntry::create());
		menu->addChild(MenuLabel::create("Scales"));
		const std::string scaleLabels[] = {
			"Major",
			"Minor",
			"Pentatonic",
			"Pelog",
			"Raag Bhairav That",
			"Raag Shri",
		};
		for (int i = 0; i < (int) LENGTHOF(scaleLabels); i++) {
			ScaleItem *item = MenuItem::create<ScaleItem>(scaleLabels[i], CHECKMARK(module->x_scale == i));
			item->module = module;
			item->scale = i;
			menu->addChild(item);
		}

		struct XClockSourceInternal : MenuItem {
			Marbles *module;
			int source;
			void onAction(EventAction &e) override {
				module->x_clock_source_internal = source;
			}
		};

		menu->addChild(MenuEntry::create());
		menu->addChild(MenuLabel::create("Internal X clock source"));
		const std::string sourceLabels[] = {
			"T₁ → X₁, T₂ → X₂, T₃ → X₃",
			"T₁ → X₁, X₂, X₃",
			"T₂ → X₁, X₂, X₃",
			"T₃ → X₁, X₂, X₃",
		};
		for (int i = 0; i < (int) LENGTHOF(sourceLabels); i++) {
			XClockSourceInternal *item = MenuItem::create<XClockSourceInternal>(sourceLabels[i], CHECKMARK(module->x_clock_source_internal == i));
			item->module = module;
			item->source = i;
			menu->addChild(item);
		}

		struct YDividerIndexItem : MenuItem {
			Marbles *module;
			int index;
			void onAction(EventAction &e) override {
				module->y_divider_index = index;
			}
		};

		struct YDividerItem : MenuItem {
			Marbles *module;
			Menu *createChildMenu() override {
				Menu *menu = new Menu();
				const std::string yDividerRatioLabels[] = {
					"1/64",
					"1/48",
					"1/32",
					"1/24",
					"1/16",
					"1/12",
					"1/8",
					"1/6",
					"1/4",
					"1/3",
					"1/2",
					"1",
				};
				for (int i = 0; i < (int) LENGTHOF(yDividerRatioLabels); i++) {
					YDividerIndexItem *item = MenuItem::create<YDividerIndexItem>(yDividerRatioLabels[i], CHECKMARK(module->y_divider_index == i));
					item->module = module;
					item->index = i;
					menu->addChild(item);
				}
				return menu;
			}
		};

		menu->addChild(MenuEntry::create());
		YDividerItem *yDividerItem = MenuItem::create<YDividerItem>("Y divider ratio");
		yDividerItem->module = module;
		menu->addChild(yDividerItem);
	}
};


RACK_PLUGIN_MODEL_INIT(AudibleInstruments, Marbles) {
   Model *modelMarbles = createModel<Marbles, MarblesWidget>("Audible Instruments", "Marbles", "Random Sampler");
   return modelMarbles;
}

