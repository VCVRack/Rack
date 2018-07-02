#include <string.h>
#include "AudibleInstruments.hpp"
#include "dsp/functions.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"
#include "dsp/digital.hpp"
#include "rings/dsp/part.h"
#include "rings/dsp/strummer.h"
#include "rings/dsp/string_synth_part.h"


struct Rings : Module {
	enum ParamIds {
		POLYPHONY_PARAM,
		RESONATOR_PARAM,

		FREQUENCY_PARAM,
		STRUCTURE_PARAM,
		BRIGHTNESS_PARAM,
		DAMPING_PARAM,
		POSITION_PARAM,

		BRIGHTNESS_MOD_PARAM,
		FREQUENCY_MOD_PARAM,
		DAMPING_MOD_PARAM,
		STRUCTURE_MOD_PARAM,
		POSITION_MOD_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		BRIGHTNESS_MOD_INPUT,
		FREQUENCY_MOD_INPUT,
		DAMPING_MOD_INPUT,
		STRUCTURE_MOD_INPUT,
		POSITION_MOD_INPUT,

		STRUM_INPUT,
		PITCH_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ODD_OUTPUT,
		EVEN_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		POLYPHONY_GREEN_LIGHT, POLYPHONY_RED_LIGHT,
		RESONATOR_GREEN_LIGHT, RESONATOR_RED_LIGHT,
		NUM_LIGHTS
	};

	SampleRateConverter<1> inputSrc;
	SampleRateConverter<2> outputSrc;
	DoubleRingBuffer<Frame<1>, 256> inputBuffer;
	DoubleRingBuffer<Frame<2>, 256> outputBuffer;

	uint16_t reverb_buffer[32768] = {};
	rings::Part part;
	rings::StringSynthPart string_synth;
	rings::Strummer strummer;
	bool strum = false;
	bool lastStrum = false;

	SchmittTrigger polyphonyTrigger;
	SchmittTrigger modelTrigger;
	int polyphonyMode = 0;
	rings::ResonatorModel model = rings::RESONATOR_MODEL_MODAL;
	bool easterEgg = false;

	Rings();
	void step() override;

	json_t *toJson() override {
		json_t *rootJ = json_object();

		json_object_set_new(rootJ, "polyphony", json_integer(polyphonyMode));
		json_object_set_new(rootJ, "model", json_integer((int) model));
		json_object_set_new(rootJ, "easterEgg", json_boolean(easterEgg));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *polyphonyJ = json_object_get(rootJ, "polyphony");
		if (polyphonyJ) {
			polyphonyMode = json_integer_value(polyphonyJ);
		}

		json_t *modelJ = json_object_get(rootJ, "model");
		if (modelJ) {
			model = (rings::ResonatorModel) json_integer_value(modelJ);
		}

		json_t *easterEggJ = json_object_get(rootJ, "easterEgg");
		if (easterEggJ) {
			easterEgg = json_boolean_value(easterEggJ);
		}
	}

	void onReset() override {
		polyphonyMode = 0;
		model = rings::RESONATOR_MODEL_MODAL;
	}

	void onRandomize() override {
		polyphonyMode = randomu32() % 3;
		model = (rings::ResonatorModel) (randomu32() % 3);
	}
};


Rings::Rings() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	memset(&strummer, 0, sizeof(strummer));
	memset(&part, 0, sizeof(part));
	memset(&string_synth, 0, sizeof(string_synth));

	strummer.Init(0.01, 44100.0 / 24);
	part.Init(reverb_buffer);
	string_synth.Init(reverb_buffer);
}

void Rings::step() {
	// TODO
	// "Normalized to a pulse/burst generator that reacts to note changes on the V/OCT input."
	// Get input
	if (!inputBuffer.full()) {
		Frame<1> f;
		f.samples[0] = inputs[IN_INPUT].value / 5.0;
		inputBuffer.push(f);
	}

	if (!strum) {
		strum = inputs[STRUM_INPUT].value >= 1.0;
	}

	// Polyphony / model
	if (polyphonyTrigger.process(params[POLYPHONY_PARAM].value)) {
		polyphonyMode = (polyphonyMode + 1) % 3;
	}
	lights[POLYPHONY_GREEN_LIGHT].value = (polyphonyMode == 0 || polyphonyMode == 1) ? 1.0 : 0.0;
	lights[POLYPHONY_RED_LIGHT].value = (polyphonyMode == 1 || polyphonyMode == 2) ? 1.0 : 0.0;

	if (modelTrigger.process(params[RESONATOR_PARAM].value)) {
		model = (rings::ResonatorModel) ((model + 1) % 3);
	}
	int modelColor = model % 3;
	lights[RESONATOR_GREEN_LIGHT].value = (modelColor == 0 || modelColor == 1) ? 1.0 : 0.0;
	lights[RESONATOR_RED_LIGHT].value = (modelColor == 1 || modelColor == 2) ? 1.0 : 0.0;

	// Render frames
	if (outputBuffer.empty()) {
		float in[24] = {};
		// Convert input buffer
		{
			inputSrc.setRates(engineGetSampleRate(), 48000);
			int inLen = inputBuffer.size();
			int outLen = 24;
			inputSrc.process(inputBuffer.startData(), &inLen, (Frame<1>*) in, &outLen);
			inputBuffer.startIncr(inLen);
		}

		// Polyphony
		int polyphony = 1 << polyphonyMode;
		if (part.polyphony() != polyphony)
			part.set_polyphony(polyphony);
		// Model
		if (easterEgg)
			string_synth.set_fx((rings::FxType) model);
		else
			part.set_model(model);

		// Patch
		rings::Patch patch;
		float structure = params[STRUCTURE_PARAM].value + 3.3*quadraticBipolar(params[STRUCTURE_MOD_PARAM].value)*inputs[STRUCTURE_MOD_INPUT].value/5.0;
		patch.structure = clamp(structure, 0.0f, 0.9995f);
		patch.brightness = clamp(params[BRIGHTNESS_PARAM].value + 3.3*quadraticBipolar(params[BRIGHTNESS_MOD_PARAM].value)*inputs[BRIGHTNESS_MOD_INPUT].value/5.0, 0.0f, 1.0f);
		patch.damping = clamp(params[DAMPING_PARAM].value + 3.3*quadraticBipolar(params[DAMPING_MOD_PARAM].value)*inputs[DAMPING_MOD_INPUT].value/5.0, 0.0f, 0.9995f);
		patch.position = clamp(params[POSITION_PARAM].value + 3.3*quadraticBipolar(params[POSITION_MOD_PARAM].value)*inputs[POSITION_MOD_INPUT].value/5.0, 0.0f, 0.9995f);

		// Performance
		rings::PerformanceState performance_state;
		performance_state.note = 12.0*inputs[PITCH_INPUT].normalize(1/12.0);
		float transpose = params[FREQUENCY_PARAM].value;
		// Quantize transpose if pitch input is connected
		if (inputs[PITCH_INPUT].active) {
			transpose = roundf(transpose);
		}
		performance_state.tonic = 12.0 + clamp(transpose, 0.0f, 60.0f);
		performance_state.fm = clamp(48.0 * 3.3*quarticBipolar(params[FREQUENCY_MOD_PARAM].value) * inputs[FREQUENCY_MOD_INPUT].normalize(1.0)/5.0, -48.0f, 48.0f);

		performance_state.internal_exciter = !inputs[IN_INPUT].active;
		performance_state.internal_strum = !inputs[STRUM_INPUT].active;
		performance_state.internal_note = !inputs[PITCH_INPUT].active;

		// TODO
		// "Normalized to a step detector on the V/OCT input and a transient detector on the IN input."
		performance_state.strum = strum && !lastStrum;
		lastStrum = strum;
		strum = false;

		performance_state.chord = clamp((int) roundf(structure * (rings::kNumChords - 1)), 0, rings::kNumChords - 1);

		// Process audio
		float out[24];
		float aux[24];
		if (easterEgg) {
			strummer.Process(NULL, 24, &performance_state);
			string_synth.Process(performance_state, patch, in, out, aux, 24);
		}
		else {
			strummer.Process(in, 24, &performance_state);
			part.Process(performance_state, patch, in, out, aux, 24);
		}

		// Convert output buffer
		{
			Frame<2> outputFrames[24];
			for (int i = 0; i < 24; i++) {
				outputFrames[i].samples[0] = out[i];
				outputFrames[i].samples[1] = aux[i];
			}

			outputSrc.setRates(48000, engineGetSampleRate());
			int inLen = 24;
			int outLen = outputBuffer.capacity();
			outputSrc.process(outputFrames, &inLen, outputBuffer.endData(), &outLen);
			outputBuffer.endIncr(outLen);
		}
	}

	// Set output
	if (!outputBuffer.empty()) {
		Frame<2> outputFrame = outputBuffer.shift();
		// "Note that you need to insert a jack into each output to split the signals: when only one jack is inserted, both signals are mixed together."
		if (outputs[ODD_OUTPUT].active && outputs[EVEN_OUTPUT].active) {
			outputs[ODD_OUTPUT].value = clamp(outputFrame.samples[0], -1.0, 1.0)*5.0;
			outputs[EVEN_OUTPUT].value = clamp(outputFrame.samples[1], -1.0, 1.0)*5.0;
		}
		else {
			float v = clamp(outputFrame.samples[0] + outputFrame.samples[1], -1.0, 1.0)*5.0;
			outputs[ODD_OUTPUT].value = v;
			outputs[EVEN_OUTPUT].value = v;
		}
	}
}


struct RingsWidget : ModuleWidget {
	RingsWidget(Rings *module) : ModuleWidget(module) {
#ifdef USE_VST2
		setPanel(SVG::load(assetStaticPlugin("AudibleInstruments", "res/Rings.svg")));
#else
		setPanel(SVG::load(assetPlugin(plugin, "res/Rings.svg")));
#endif // USE_VST2

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(180, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(180, 365)));

		addParam(ParamWidget::create<TL1105>(Vec(14, 40), module, Rings::POLYPHONY_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<TL1105>(Vec(179, 40), module, Rings::RESONATOR_PARAM, 0.0, 1.0, 0.0));

		addParam(ParamWidget::create<Rogan3PSWhite>(Vec(29, 72), module, Rings::FREQUENCY_PARAM, 0.0, 60.0, 30.0));
		addParam(ParamWidget::create<Rogan3PSWhite>(Vec(126, 72), module, Rings::STRUCTURE_PARAM, 0.0, 1.0, 0.5));

		addParam(ParamWidget::create<Rogan1PSWhite>(Vec(13, 158), module, Rings::BRIGHTNESS_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Rogan1PSWhite>(Vec(83, 158), module, Rings::DAMPING_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Rogan1PSWhite>(Vec(154, 158), module, Rings::POSITION_PARAM, 0.0, 1.0, 0.5));

		addParam(ParamWidget::create<Trimpot>(Vec(19, 229), module, Rings::BRIGHTNESS_MOD_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Trimpot>(Vec(57, 229), module, Rings::FREQUENCY_MOD_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Trimpot>(Vec(96, 229), module, Rings::DAMPING_MOD_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Trimpot>(Vec(134, 229), module, Rings::STRUCTURE_MOD_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Trimpot>(Vec(173, 229), module, Rings::POSITION_MOD_PARAM, -1.0, 1.0, 0.0));

		addInput(Port::create<PJ301MPort>(Vec(15, 273), Port::INPUT, module, Rings::BRIGHTNESS_MOD_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(54, 273), Port::INPUT, module, Rings::FREQUENCY_MOD_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(92, 273), Port::INPUT, module, Rings::DAMPING_MOD_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(131, 273), Port::INPUT, module, Rings::STRUCTURE_MOD_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(169, 273), Port::INPUT, module, Rings::POSITION_MOD_INPUT));

		addInput(Port::create<PJ301MPort>(Vec(15, 316), Port::INPUT, module, Rings::STRUM_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(54, 316), Port::INPUT, module, Rings::PITCH_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(92, 316), Port::INPUT, module, Rings::IN_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(131, 316), Port::OUTPUT, module, Rings::ODD_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(169, 316), Port::OUTPUT, module, Rings::EVEN_OUTPUT));

		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(37, 43), module, Rings::POLYPHONY_GREEN_LIGHT));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(162, 43), module, Rings::RESONATOR_GREEN_LIGHT));
	}

	void appendContextMenu(Menu *menu) override {
		Rings *rings = dynamic_cast<Rings*>(module);
		assert(rings);

		struct RingsModelItem : MenuItem {
			Rings *rings;
			rings::ResonatorModel model;
			void onAction(EventAction &e) override {
				rings->model = model;
			}
			void step() override {
				rightText = (rings->model == model) ? "✔" : "";
				MenuItem::step();
			}
		};

		struct RingsEasterEggItem : MenuItem {
			Rings *rings;
			void onAction(EventAction &e) override {
				rings->easterEgg = !rings->easterEgg;
			}
			void step() override {
				rightText = (rings->easterEgg) ? "✔" : "";
				MenuItem::step();
			}
		};

		menu->addChild(construct<MenuLabel>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Resonator"));
		menu->addChild(construct<RingsModelItem>(&MenuItem::text, "Modal resonator", &RingsModelItem::rings, rings, &RingsModelItem::model, rings::RESONATOR_MODEL_MODAL));
		menu->addChild(construct<RingsModelItem>(&MenuItem::text, "Sympathetic strings", &RingsModelItem::rings, rings, &RingsModelItem::model, rings::RESONATOR_MODEL_SYMPATHETIC_STRING));
		menu->addChild(construct<RingsModelItem>(&MenuItem::text, "Modulated/inharmonic string", &RingsModelItem::rings, rings, &RingsModelItem::model, rings::RESONATOR_MODEL_STRING));
		menu->addChild(construct<RingsModelItem>(&MenuItem::text, "FM voice", &RingsModelItem::rings, rings, &RingsModelItem::model, rings::RESONATOR_MODEL_FM_VOICE));
		menu->addChild(construct<RingsModelItem>(&MenuItem::text, "Quantized sympathetic strings", &RingsModelItem::rings, rings, &RingsModelItem::model, rings::RESONATOR_MODEL_SYMPATHETIC_STRING_QUANTIZED));
		menu->addChild(construct<RingsModelItem>(&MenuItem::text, "Reverb string", &RingsModelItem::rings, rings, &RingsModelItem::model, rings::RESONATOR_MODEL_STRING_AND_REVERB));

		menu->addChild(construct<MenuLabel>());
		menu->addChild(construct<RingsEasterEggItem>(&MenuItem::text, "Disastrous Peace", &RingsEasterEggItem::rings, rings));
	}
};


RACK_PLUGIN_MODEL_INIT(AudibleInstruments, Rings) {
   Model *modelRings = Model::create<Rings, RingsWidget>("Audible Instruments", "Rings", "Resonator", PHYSICAL_MODELING_TAG);
   return modelRings;
}

