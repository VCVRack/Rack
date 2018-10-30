#include "AudibleInstruments.hpp"
#include "dsp/digital.hpp"
#include "stages/segment_generator.h"
#include "stages/oscillator.h"


// Must match io_buffer.h
static const int NUM_CHANNELS = 6;
static const int BLOCK_SIZE = 8;

struct LongPressButton {
	enum Events {
		NO_PRESS,
		SHORT_PRESS,
		LONG_PRESS
	};

	float pressedTime = 0.f;
	BooleanTrigger trigger;

	Events step(Param &param) {
		Events result = NO_PRESS;

		bool pressed = param.value > 0.f;
		if (pressed && pressedTime >= 0.f) {
			pressedTime += engineGetSampleTime();
			if (pressedTime >= 1.f) {
				pressedTime = -1.f;
				result = LONG_PRESS;
			}
		}

		// Check if released
		if (trigger.process(!pressed)) {
			if (pressedTime >= 0.f) {
				result = SHORT_PRESS;
			}
			pressedTime = 0.f;
		}

		return result;
	}
};

struct GroupInfo {
	int first_segment = 0;
	int segment_count = 0;
	bool gated = false;
};

struct GroupBuilder {

	GroupInfo groups[NUM_CHANNELS];
	int groupCount = 0;

	bool buildGroups(std::vector<Input> *gateInputs, size_t first, size_t count) {
		bool any_gates = false;

		GroupInfo nextGroups[NUM_CHANNELS];

		int currentGroup = 0;
		for (int i = 0; i < NUM_CHANNELS; i++) {
			bool gated = (*gateInputs)[first + i].active;

			if (!any_gates) {
				if (!gated) {
					// No gates at all yet, segments are all single segment groups
					nextGroups[currentGroup].first_segment = i;
					nextGroups[currentGroup].segment_count = 1;
					nextGroups[currentGroup].gated = false;
					currentGroup++;
				}
				else {
					// first gate, current group is start of a segment group
					any_gates = true;
					nextGroups[currentGroup].first_segment = i;
					nextGroups[currentGroup].segment_count = 1;
					nextGroups[currentGroup].gated = true;
					currentGroup++;
				}
			}
			else {
				if (!gated) {
					// We've had a gate, this ungated segment is part of the previous group
					nextGroups[currentGroup-1].segment_count++;
				}
				else {
					// This gated input indicates the start of the next group
					nextGroups[currentGroup].first_segment = i;
					nextGroups[currentGroup].segment_count = 1;
					nextGroups[currentGroup].gated = true;
					currentGroup++;
				}
			}
		}

		bool changed = false;

		if (currentGroup != groupCount) {
			changed = true;
			groupCount = currentGroup;
		}

		for (int i = 0; i < groupCount; i++) {
			if (nextGroups[i].segment_count != groups[i].segment_count || 
					nextGroups[i].gated != groups[i].gated ||
					nextGroups[i].first_segment != groups[i].first_segment) {
				changed = true;
			}

			groups[i].first_segment = nextGroups[i].first_segment;
			groups[i].segment_count = nextGroups[i].segment_count;
			groups[i].gated = nextGroups[i].gated;
		}

		return changed;
	}
};

struct Stages : Module {
	enum ParamIds {
		ENUMS(SHAPE_PARAMS, NUM_CHANNELS),
		ENUMS(TYPE_PARAMS, NUM_CHANNELS),
		ENUMS(LEVEL_PARAMS, NUM_CHANNELS),
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(LEVEL_INPUTS, NUM_CHANNELS),
		ENUMS(GATE_INPUTS, NUM_CHANNELS),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(ENVELOPE_OUTPUTS, NUM_CHANNELS),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(TYPE_LIGHTS, NUM_CHANNELS * 2),
		ENUMS(ENVELOPE_LIGHTS, NUM_CHANNELS),
		NUM_LIGHTS
	};

	stages::segment::Configuration configurations[NUM_CHANNELS];
	bool configuration_changed[NUM_CHANNELS];
	stages::SegmentGenerator segment_generator[NUM_CHANNELS];
	float lightOscillatorPhase;

	// Buttons
	LongPressButton typeButtons[NUM_CHANNELS];

	// Buffers
	float envelopeBuffer[NUM_CHANNELS][BLOCK_SIZE] = {};
	stmlib::GateFlags last_gate_flags[NUM_CHANNELS] = {};
	stmlib::GateFlags gate_flags[NUM_CHANNELS][BLOCK_SIZE] = {};
	int blockIndex = 0;
	GroupBuilder groupBuilder;

	Stages() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void onReset() override {
		for (size_t i = 0; i < NUM_CHANNELS; ++i) {
			segment_generator[i].Init();

			configurations[i].type = stages::segment::TYPE_RAMP;
			configurations[i].loop = false;
			configuration_changed[i] = true;
		}

      configurations[4].type = stages::segment::TYPE_HOLD;
      configurations[4].loop = true;

		lightOscillatorPhase = 0.f;
		onSampleRateChange();
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();

		json_t *configurationsJ = json_array();
		for (int i = 0; i < NUM_CHANNELS; i++) {
			json_t *configurationJ = json_object();
			json_object_set_new(configurationJ, "type", json_integer(configurations[i].type));
			json_object_set_new(configurationJ, "loop", json_boolean(configurations[i].loop));
			json_array_insert_new(configurationsJ, i, configurationJ);
		}
		json_object_set_new(rootJ, "configurations", configurationsJ);

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *configurationsJ = json_object_get(rootJ, "configurations");
		for (int i = 0; i < NUM_CHANNELS; i++) {
			json_t *configurationJ = json_array_get(configurationsJ, i);
			if (configurationJ) {
				json_t *typeJ = json_object_get(configurationJ, "type");
				if (typeJ)
					configurations[i].type = (stages::segment::Type) json_integer_value(typeJ);

				json_t *loopJ = json_object_get(configurationJ, "loop");
				if (loopJ)
					configurations[i].loop = json_boolean_value(loopJ);
			}
		}
	}

	void onSampleRateChange() override {
		for (int i = 0; i < NUM_CHANNELS; i++) {
			segment_generator[i].SetSampleRate(engineGetSampleRate());
		}
	}

	void stepBlock() {
		// Get parameters
		float primaries[NUM_CHANNELS];
		float secondaries[NUM_CHANNELS];
		for (int i = 0; i < NUM_CHANNELS; i++) {
			primaries[i] = clamp(params[LEVEL_PARAMS + i].value + inputs[LEVEL_INPUTS + i].value / 8.f, 0.f, 1.f);
			secondaries[i] = params[SHAPE_PARAMS + i].value;
		}

		// See if the group associations have changed since the last group
		bool groups_changed = groupBuilder.buildGroups(&inputs, GATE_INPUTS, NUM_CHANNELS);

		// Process block
		stages::SegmentGenerator::Output out[BLOCK_SIZE] = {};
		for (int i = 0; i < groupBuilder.groupCount; i++) {
			GroupInfo &group = groupBuilder.groups[i];

			// Check if the config needs applying to the segment generator for this group
			bool apply_config = groups_changed;
			int numberOfLoopsInGroup = 0;
			for (int j = 0; j < group.segment_count; j++) {
				int segment = group.first_segment + j;
				numberOfLoopsInGroup += configurations[segment].loop ? 1 : 0;
				apply_config |= configuration_changed[segment];
				configuration_changed[segment] = false;
			}

			if (numberOfLoopsInGroup > 2) {
				// Too many segments are looping, turn them all off
				apply_config = true;
				for (int j = 0; j < group.segment_count; j++) {
					configurations[group.first_segment + j].loop = false;
				}
			}

			if (apply_config) {
				segment_generator[i].Configure(group.gated, &configurations[group.first_segment], group.segment_count);
			}

			// Set the segment parameters on the generator we're about to process
			for (int j = 0; j < group.segment_count; j++) {
				segment_generator[i].set_segment_parameters(j, primaries[group.first_segment + j], secondaries[group.first_segment + j]);
			}

			segment_generator[i].Process(gate_flags[group.first_segment], out, BLOCK_SIZE);

			for (int j = 0; j < BLOCK_SIZE; j++) {
				for (int k = 1; k < group.segment_count; k++) {
					int segment = group.first_segment + k;
					if (k == out[j].segment) {
						// Set the phase output for the active segment
						envelopeBuffer[segment][j] = 1.f - out[j].phase;
					}
					else {
						// Non active segments have 0.f output
						envelopeBuffer[segment][j] = 0.f;
					}
				}
				// First group segment gets the actual output
				envelopeBuffer[group.first_segment][j] = out[j].value;
			}
		}
	}

	void toggleMode(int i) {
		configurations[i].type = (stages::segment::Type) ((configurations[i].type + 1) % 3);
		configuration_changed[i] = true;
	}

	void toggleLoop(int segment) {
		configuration_changed[segment] = true;
		configurations[segment].loop = !configurations[segment].loop;

		// ensure that we don't have too many looping segments in the group
		if (configurations[segment].loop) {
			int segment_count = 0;
			for (int i = 0; i < groupBuilder.groupCount; i++) {
				segment_count += groupBuilder.groups[i].segment_count;

				if (segment_count > segment) {
					GroupInfo &group = groupBuilder.groups[i];

					// See how many loop items we have
					int numberOfLoopsInGroup = 0;

					for (int j = 0; j < group.segment_count; j++) {
						numberOfLoopsInGroup += configurations[group.first_segment + j].loop ? 1 : 0;
					}

					// If we've got too many loop items, clear down to the one looping segment
					if (numberOfLoopsInGroup > 2) {
						for (int j = 0; j < group.segment_count; j++) {
							configurations[group.first_segment + j].loop = (group.first_segment + j) == segment;
						}
					}

					break;
				}
			}
		}
	}

	void step() override {
		// Oscillate flashing the type lights
		lightOscillatorPhase += 0.5f * engineGetSampleTime();
		if (lightOscillatorPhase >= 1.0f)
			lightOscillatorPhase -= 1.0f;

		// Buttons
		for (int i = 0; i < NUM_CHANNELS; i++) {
			switch (typeButtons[i].step(params[TYPE_PARAMS + i])) {
				default:
				case LongPressButton::NO_PRESS: break;
				case LongPressButton::SHORT_PRESS: toggleMode(i); break;
				case LongPressButton::LONG_PRESS: toggleLoop(i); break;
			}
		}

		// Input
		for (int i = 0; i < NUM_CHANNELS; i++) {
			bool gate = (inputs[GATE_INPUTS + i].value >= 1.7f);
			last_gate_flags[i] = stmlib::ExtractGateFlags(last_gate_flags[i], gate);
			gate_flags[i][blockIndex] = last_gate_flags[i];
		}

		// Process block
		if (++blockIndex >= BLOCK_SIZE) {
			blockIndex = 0;
			stepBlock();
		}

		// Output
		for (int i = 0; i < groupBuilder.groupCount; i++) {
			GroupInfo &group = groupBuilder.groups[i];

			int numberOfLoopsInGroup = 0;
			for (int j = 0; j < group.segment_count; j++) {
				int segment = group.first_segment + j;

				float envelope = envelopeBuffer[segment][blockIndex];
				outputs[ENVELOPE_OUTPUTS + segment].value = envelope * 8.f;
				lights[ENVELOPE_LIGHTS + segment].setBrightnessSmooth(envelope);

				numberOfLoopsInGroup += configurations[segment].loop ? 1 : 0;
				float flashlevel = 1.f;

				if (configurations[segment].loop && numberOfLoopsInGroup == 1) {
					flashlevel = abs(sinf(2.0f * M_PI * lightOscillatorPhase));
				}
				else if (configurations[segment].loop && numberOfLoopsInGroup > 1) {
					float advancedPhase = lightOscillatorPhase + 0.25f;
					if (advancedPhase > 1.0f)
						advancedPhase -= 1.0f;

					flashlevel = abs(sinf(2.0f * M_PI * advancedPhase));
				}

				lights[TYPE_LIGHTS + segment * 2 + 0].setBrightness((configurations[segment].type == 0 || configurations[segment].type == 1) * flashlevel);
				lights[TYPE_LIGHTS + segment * 2 + 1].setBrightness((configurations[segment].type == 1 || configurations[segment].type == 2) * flashlevel);
			}
		}
	}
};


struct StagesWidget : ModuleWidget {
	StagesWidget(Stages *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Stages.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(ParamWidget::create<Trimpot>(mm2px(Vec(3.72965, 13.98158)), module, Stages::SHAPE_PARAMS + 0, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Trimpot>(mm2px(Vec(15.17012, 13.98158)), module, Stages::SHAPE_PARAMS + 1, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Trimpot>(mm2px(Vec(26.6099, 13.98158)), module, Stages::SHAPE_PARAMS + 2, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Trimpot>(mm2px(Vec(38.07174, 13.98158)), module, Stages::SHAPE_PARAMS + 3, 0.0, 1.0, 0.7));
		addParam(ParamWidget::create<Trimpot>(mm2px(Vec(49.51152, 13.98158)), module, Stages::SHAPE_PARAMS + 4, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Trimpot>(mm2px(Vec(60.95199, 13.98158)), module, Stages::SHAPE_PARAMS + 5, 0.0, 1.0, 0.6));
		addParam(ParamWidget::create<TL1105>(mm2px(Vec(4.17259, 32.37248)), module, Stages::TYPE_PARAMS + 0, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<TL1105>(mm2px(Vec(15.61237, 32.37248)), module, Stages::TYPE_PARAMS + 1, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<TL1105>(mm2px(Vec(27.05284, 32.37248)), module, Stages::TYPE_PARAMS + 2, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<TL1105>(mm2px(Vec(38.51399, 32.37248)), module, Stages::TYPE_PARAMS + 3, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<TL1105>(mm2px(Vec(49.95446, 32.37248)), module, Stages::TYPE_PARAMS + 4, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<TL1105>(mm2px(Vec(61.39424, 32.37248)), module, Stages::TYPE_PARAMS + 5, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<LEDSliderGreen>(mm2px(Vec(3.36193, 43.06508)), module, Stages::LEVEL_PARAMS + 0, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<LEDSliderGreen>(mm2px(Vec(14.81619, 43.06508)), module, Stages::LEVEL_PARAMS + 1, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<LEDSliderGreen>(mm2px(Vec(26.26975, 43.06508)), module, Stages::LEVEL_PARAMS + 2, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<LEDSliderGreen>(mm2px(Vec(37.70265, 43.06508)), module, Stages::LEVEL_PARAMS + 3, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<LEDSliderGreen>(mm2px(Vec(49.15759, 43.06508)), module, Stages::LEVEL_PARAMS + 4, 0.0, 1.0, 0.7));
		addParam(ParamWidget::create<LEDSliderGreen>(mm2px(Vec(60.61184, 43.06508)), module, Stages::LEVEL_PARAMS + 5, 0.0, 1.0, 0.5));

		addInput(Port::create<PJ301MPort>(mm2px(Vec(2.70756, 77.75277)), Port::INPUT, module, Stages::LEVEL_INPUTS + 0));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(14.14734, 77.75277)), Port::INPUT, module, Stages::LEVEL_INPUTS + 1));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(25.58781, 77.75277)), Port::INPUT, module, Stages::LEVEL_INPUTS + 2));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(37.04896, 77.75277)), Port::INPUT, module, Stages::LEVEL_INPUTS + 3));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(48.48943, 77.75277)), Port::INPUT, module, Stages::LEVEL_INPUTS + 4));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(59.92921, 77.75277)), Port::INPUT, module, Stages::LEVEL_INPUTS + 5));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(2.70756, 92.35239)), Port::INPUT, module, Stages::GATE_INPUTS + 0));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(14.14734, 92.35239)), Port::INPUT, module, Stages::GATE_INPUTS + 1));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(25.58781, 92.35239)), Port::INPUT, module, Stages::GATE_INPUTS + 2));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(37.04896, 92.35239)), Port::INPUT, module, Stages::GATE_INPUTS + 3));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(48.48943, 92.35239)), Port::INPUT, module, Stages::GATE_INPUTS + 4));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(59.92921, 92.35239)), Port::INPUT, module, Stages::GATE_INPUTS + 5));

		addOutput(Port::create<PJ301MPort>(mm2px(Vec(2.70756, 106.95203)), Port::OUTPUT, module, Stages::ENVELOPE_OUTPUTS + 0));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(14.14734, 106.95203)), Port::OUTPUT, module, Stages::ENVELOPE_OUTPUTS + 1));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(25.58781, 106.95203)), Port::OUTPUT, module, Stages::ENVELOPE_OUTPUTS + 2));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(37.04896, 106.95203)), Port::OUTPUT, module, Stages::ENVELOPE_OUTPUTS + 3));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(48.48943, 106.95203)), Port::OUTPUT, module, Stages::ENVELOPE_OUTPUTS + 4));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(59.92921, 106.95203)), Port::OUTPUT, module, Stages::ENVELOPE_OUTPUTS + 5));

		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(mm2px(Vec(5.27737, 26.74447)), module, Stages::TYPE_LIGHTS + 0 * 2));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(mm2px(Vec(16.73784, 26.74447)), module, Stages::TYPE_LIGHTS + 1 * 2));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(mm2px(Vec(28.1783, 26.74447)), module, Stages::TYPE_LIGHTS + 2 * 2));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(mm2px(Vec(39.61877, 26.74447)), module, Stages::TYPE_LIGHTS + 3 * 2));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(mm2px(Vec(51.07923, 26.74447)), module, Stages::TYPE_LIGHTS + 4 * 2));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(mm2px(Vec(62.51971, 26.74447)), module, Stages::TYPE_LIGHTS + 5 * 2));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(mm2px(Vec(2.29462, 103.19253)), module, Stages::ENVELOPE_LIGHTS + 0));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(mm2px(Vec(13.73509, 103.19253)), module, Stages::ENVELOPE_LIGHTS + 1));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(mm2px(Vec(25.17556, 103.19253)), module, Stages::ENVELOPE_LIGHTS + 2));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(mm2px(Vec(36.63671, 103.19253)), module, Stages::ENVELOPE_LIGHTS + 3));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(mm2px(Vec(48.07649, 103.19253)), module, Stages::ENVELOPE_LIGHTS + 4));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(mm2px(Vec(59.51696, 103.19253)), module, Stages::ENVELOPE_LIGHTS + 5));
	}
};


RACK_PLUGIN_MODEL_INIT(AudibleInstruments, Stages) {
   Model *modelStages = Model::create<Stages, StagesWidget>("Audible Instruments", "Stages", "Segment Generator", FUNCTION_GENERATOR_TAG, ENVELOPE_GENERATOR_TAG);
   return modelStages;
}
