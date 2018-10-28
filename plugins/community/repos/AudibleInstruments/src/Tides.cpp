#include <string.h>
#include "AudibleInstruments.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/digital.hpp"
#include "tides/generator.h"


struct Tides : Module {
	enum ParamIds {
		MODE_PARAM,
		RANGE_PARAM,

		FREQUENCY_PARAM,
		FM_PARAM,

		SHAPE_PARAM,
		SLOPE_PARAM,
		SMOOTHNESS_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		SHAPE_INPUT,
		SLOPE_INPUT,
		SMOOTHNESS_INPUT,

		TRIG_INPUT,
		FREEZE_INPUT,
		PITCH_INPUT,
		FM_INPUT,
		LEVEL_INPUT,

		CLOCK_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		HIGH_OUTPUT,
		LOW_OUTPUT,
		UNI_OUTPUT,
		BI_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		MODE_GREEN_LIGHT, MODE_RED_LIGHT,
		PHASE_GREEN_LIGHT, PHASE_RED_LIGHT,
		RANGE_GREEN_LIGHT, RANGE_RED_LIGHT,
		NUM_LIGHTS
	};

	bool sheep;
	tides::Generator generator;
	int frame = 0;
	uint8_t lastGate;
	SchmittTrigger modeTrigger;
	SchmittTrigger rangeTrigger;

	Tides();
	void step() override;


	void onReset() override {
		generator.set_range(tides::GENERATOR_RANGE_MEDIUM);
		generator.set_mode(tides::GENERATOR_MODE_LOOPING);
		sheep = false;
	}

	void onRandomize() override {
		generator.set_range((tides::GeneratorRange) (randomu32() % 3));
		generator.set_mode((tides::GeneratorMode) (randomu32() % 3));
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();

		json_object_set_new(rootJ, "mode", json_integer((int) generator.mode()));
		json_object_set_new(rootJ, "range", json_integer((int) generator.range()));
		json_object_set_new(rootJ, "sheep", json_boolean(sheep));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *modeJ = json_object_get(rootJ, "mode");
		if (modeJ) {
			generator.set_mode((tides::GeneratorMode) json_integer_value(modeJ));
		}

		json_t *rangeJ = json_object_get(rootJ, "range");
		if (rangeJ) {
			generator.set_range((tides::GeneratorRange) json_integer_value(rangeJ));
		}

		json_t *sheepJ = json_object_get(rootJ, "sheep");
		if (sheepJ) {
			sheep = json_boolean_value(sheepJ);
		}
	}
};


Tides::Tides() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	memset(&generator, 0, sizeof(generator));
	generator.Init();
	generator.set_sync(false);
	onReset();
}

void Tides::step() {
	tides::GeneratorMode mode = generator.mode();
	if (modeTrigger.process(params[MODE_PARAM].value)) {
		mode = (tides::GeneratorMode) (((int)mode - 1 + 3) % 3);
		generator.set_mode(mode);
	}
	lights[MODE_GREEN_LIGHT].value = (mode == 2) ? 1.0 : 0.0;
	lights[MODE_RED_LIGHT].value = (mode == 0) ? 1.0 : 0.0;

	tides::GeneratorRange range = generator.range();
	if (rangeTrigger.process(params[RANGE_PARAM].value)) {
		range = (tides::GeneratorRange) (((int)range - 1 + 3) % 3);
		generator.set_range(range);
	}
	lights[RANGE_GREEN_LIGHT].value = (range == 2) ? 1.0 : 0.0;
	lights[RANGE_RED_LIGHT].value = (range == 0) ? 1.0 : 0.0;

	// Buffer loop
	if (++frame >= 16) {
		frame = 0;

		// Pitch
		float pitch = params[FREQUENCY_PARAM].value;
		pitch += 12.0 * inputs[PITCH_INPUT].value;
		pitch += params[FM_PARAM].value * inputs[FM_INPUT].normalize(0.1) / 5.0;
		pitch += 60.0;
		// Scale to the global sample rate
		pitch += log2f(48000.0 / engineGetSampleRate()) * 12.0;
		generator.set_pitch((int) clamp(pitch * 0x80, (float) -0x8000, (float) 0x7fff));

		// Slope, smoothness, pitch
		int16_t shape = clamp(params[SHAPE_PARAM].value + inputs[SHAPE_INPUT].value / 5.0f, -1.0f, 1.0f) * 0x7fff;
		int16_t slope = clamp(params[SLOPE_PARAM].value + inputs[SLOPE_INPUT].value / 5.0f, -1.0f, 1.0f) * 0x7fff;
		int16_t smoothness = clamp(params[SMOOTHNESS_PARAM].value + inputs[SMOOTHNESS_INPUT].value / 5.0f, -1.0f, 1.0f) * 0x7fff;
		generator.set_shape(shape);
		generator.set_slope(slope);
		generator.set_smoothness(smoothness);

		// Sync
		// Slight deviation from spec here.
		// Instead of toggling sync by holding the range button, just enable it if the clock port is plugged in.
		generator.set_sync(inputs[CLOCK_INPUT].active && !sheep);

		// Generator
		generator.Process(sheep);
	}

	// Level
	uint16_t level = clamp(inputs[LEVEL_INPUT].normalize(8.0) / 8.0f, 0.0f, 1.0f) * 0xffff;
	if (level < 32)
		level = 0;

	uint8_t gate = 0;
	if (inputs[FREEZE_INPUT].value >= 0.7)
		gate |= tides::CONTROL_FREEZE;
	if (inputs[TRIG_INPUT].value >= 0.7)
		gate |= tides::CONTROL_GATE;
	if (inputs[CLOCK_INPUT].value >= 0.7)
		gate |= tides::CONTROL_CLOCK;
	if (!(lastGate & tides::CONTROL_CLOCK) && (gate & tides::CONTROL_CLOCK))
		gate |= tides::CONTROL_GATE_RISING;
	if (!(lastGate & tides::CONTROL_GATE) && (gate & tides::CONTROL_GATE))
		gate |= tides::CONTROL_GATE_RISING;
	if ((lastGate & tides::CONTROL_GATE) && !(gate & tides::CONTROL_GATE))
		gate |= tides::CONTROL_GATE_FALLING;
	lastGate = gate;

	const tides::GeneratorSample& sample = generator.Process(gate);
	uint32_t uni = sample.unipolar;
	int32_t bi = sample.bipolar;

	uni = uni * level >> 16;
	bi = -bi * level >> 16;
	float unif = (float) uni / 0xffff;
	float bif = (float) bi / 0x8000;

	outputs[HIGH_OUTPUT].value = sample.flags & tides::FLAG_END_OF_ATTACK ? 0.0 : 5.0;
	outputs[LOW_OUTPUT].value = sample.flags & tides::FLAG_END_OF_RELEASE ? 0.0 : 5.0;
	outputs[UNI_OUTPUT].value = unif * 8.0;
	outputs[BI_OUTPUT].value = bif * 5.0;

	if (sample.flags & tides::FLAG_END_OF_ATTACK)
		unif *= -1.0;
	lights[PHASE_GREEN_LIGHT].setBrightnessSmooth(fmaxf(0.0, unif));
	lights[PHASE_RED_LIGHT].setBrightnessSmooth(fmaxf(0.0, -unif));
}


struct TidesWidget : ModuleWidget {
	SVGPanel *tidesPanel;
	SVGPanel *sheepPanel;

	TidesWidget(Tides *module) : ModuleWidget(module) {
		box.size = Vec(15*14, 380);
		{
			tidesPanel = new SVGPanel();
#ifdef USE_VST2
			tidesPanel->setBackground(SVG::load(assetStaticPlugin("AudibleInstruments", "res/Tides.svg")));
#else
			tidesPanel->setBackground(SVG::load(assetPlugin(plugin, "res/Tides.svg")));
#endif // USE_VST2
			tidesPanel->box.size = box.size;
			addChild(tidesPanel);
		}
		{
			sheepPanel = new SVGPanel();
#ifdef USE_VST2
			sheepPanel->setBackground(SVG::load(assetStaticPlugin("AudibleInstruments", "res/Sheep.svg")));
#else
			sheepPanel->setBackground(SVG::load(assetPlugin(plugin, "res/Sheep.svg")));
#endif // USE_VST2
			sheepPanel->box.size = box.size;
			addChild(sheepPanel);
		}

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(180, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(180, 365)));

		addParam(ParamWidget::create<CKD6>(Vec(19, 52), module, Tides::MODE_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<CKD6>(Vec(19, 93), module, Tides::RANGE_PARAM, 0.0, 1.0, 0.0));

		addParam(ParamWidget::create<Rogan3PSGreen>(Vec(78, 60), module, Tides::FREQUENCY_PARAM, -48.0, 48.0, 0.0));
		addParam(ParamWidget::create<Rogan1PSGreen>(Vec(156, 66), module, Tides::FM_PARAM, -12.0, 12.0, 0.0));

		addParam(ParamWidget::create<Rogan1PSWhite>(Vec(13, 155), module, Tides::SHAPE_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Rogan1PSWhite>(Vec(85, 155), module, Tides::SLOPE_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Rogan1PSWhite>(Vec(156, 155), module, Tides::SMOOTHNESS_PARAM, -1.0, 1.0, 0.0));

		addInput(Port::create<PJ301MPort>(Vec(21, 219), Port::INPUT, module, Tides::SHAPE_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(93, 219), Port::INPUT, module, Tides::SLOPE_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(164, 219), Port::INPUT, module, Tides::SMOOTHNESS_INPUT));

		addInput(Port::create<PJ301MPort>(Vec(21, 274), Port::INPUT, module, Tides::TRIG_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(57, 274), Port::INPUT, module, Tides::FREEZE_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(93, 274), Port::INPUT, module, Tides::PITCH_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(128, 274), Port::INPUT, module, Tides::FM_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(164, 274), Port::INPUT, module, Tides::LEVEL_INPUT));

		addInput(Port::create<PJ301MPort>(Vec(21, 316), Port::INPUT, module, Tides::CLOCK_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(57, 316), Port::OUTPUT, module, Tides::HIGH_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(93, 316), Port::OUTPUT, module, Tides::LOW_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(128, 316), Port::OUTPUT, module, Tides::UNI_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(164, 316), Port::OUTPUT, module, Tides::BI_OUTPUT));

		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(56, 61), module, Tides::MODE_GREEN_LIGHT));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(56, 82), module, Tides::PHASE_GREEN_LIGHT));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(56, 102), module, Tides::RANGE_GREEN_LIGHT));
	}

	void step() override {
		Tides *tides = dynamic_cast<Tides*>(module);
		assert(tides);

		tidesPanel->visible = !tides->sheep;
		sheepPanel->visible = tides->sheep;

		ModuleWidget::step();
	}


	void appendContextMenu(Menu *menu) override {
		Tides *module = dynamic_cast<Tides*>(this->module);

		struct SheepItem : MenuItem {
			Tides *module;
			void onAction(EventAction &e) override {
				module->sheep ^= true;
			}
		};

		menu->addChild(MenuEntry::create());
		SheepItem *sheepItem = MenuItem::create<SheepItem>("Sheep", CHECKMARK(module->sheep));
		sheepItem->module = module;
		menu->addChild(sheepItem);
	}
};


RACK_PLUGIN_MODEL_INIT(AudibleInstruments, Tides) {
   Model *modelTides = Model::create<Tides, TidesWidget>("Audible Instruments", "Tides", "Tidal Modulator", LFO_TAG, OSCILLATOR_TAG, WAVESHAPER_TAG, FUNCTION_GENERATOR_TAG);
   return modelTides;
}
