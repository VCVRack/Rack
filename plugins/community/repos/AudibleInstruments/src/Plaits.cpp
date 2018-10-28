#include "AudibleInstruments.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"
#include "dsp/functions.hpp"
#include "dsp/digital.hpp"
#include "plaits/dsp/voice.h"


struct Plaits : Module {
	enum ParamIds {
		MODEL1_PARAM,
		MODEL2_PARAM,
		FREQ_PARAM,
		HARMONICS_PARAM,
		TIMBRE_PARAM,
		MORPH_PARAM,
		TIMBRE_CV_PARAM,
		FREQ_CV_PARAM,
		MORPH_CV_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ENGINE_INPUT,
		TIMBRE_INPUT,
		FREQ_INPUT,
		MORPH_INPUT,
		HARMONICS_INPUT,
		TRIGGER_INPUT,
		LEVEL_INPUT,
		NOTE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		AUX_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(MODEL_LIGHT, 8 * 2),
		NUM_LIGHTS
	};

	plaits::Voice voice;
	plaits::Patch patch;
	plaits::Modulations modulations;
	char shared_buffer[16384];
	float triPhase = 0.f;

	SampleRateConverter<2> outputSrc;
	DoubleRingBuffer<Frame<2>, 256> outputBuffer;
	bool lowCpu = false;
	bool lpg = false;

	SchmittTrigger model1Trigger;
	SchmittTrigger model2Trigger;

	Plaits() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		memset(shared_buffer, 0, sizeof(shared_buffer));
		stmlib::BufferAllocator allocator(shared_buffer, sizeof(shared_buffer));
		voice.Init(&allocator);

		memset(&patch, 0, sizeof(patch));
		memset(&modulations, 0, sizeof(modulations));
		onReset();
	}

	void onReset() override {
		patch.engine = 0;
		patch.lpg_colour = 0.5f;
		patch.decay = 0.5f;
	}

	void onRandomize() override {
		patch.engine = randomu32() % 16;
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();

		json_object_set_new(rootJ, "lowCpu", json_boolean(lowCpu));
		json_object_set_new(rootJ, "model", json_integer(patch.engine));
		json_object_set_new(rootJ, "lpgColor", json_real(patch.lpg_colour));
		json_object_set_new(rootJ, "decay", json_real(patch.decay));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *lowCpuJ = json_object_get(rootJ, "lowCpu");
		if (lowCpuJ)
			lowCpu = json_boolean_value(lowCpuJ);

		json_t *modelJ = json_object_get(rootJ, "model");
		if (modelJ)
			patch.engine = json_integer_value(modelJ);

		json_t *lpgColorJ = json_object_get(rootJ, "lpgColor");
		if (lpgColorJ)
			patch.lpg_colour = json_number_value(lpgColorJ);

		json_t *decayJ = json_object_get(rootJ, "decay");
		if (decayJ)
			patch.decay = json_number_value(decayJ);
	}

	void step() override {
		if (outputBuffer.empty()) {
			const int blockSize = 12;

			// Model buttons
			if (model1Trigger.process(params[MODEL1_PARAM].value)) {
				if (patch.engine >= 8) {
					patch.engine -= 8;
				}
				else {
					patch.engine = (patch.engine + 1) % 8;
				}
			}
			if (model2Trigger.process(params[MODEL2_PARAM].value)) {
				if (patch.engine < 8) {
					patch.engine += 8;
				}
				else {
					patch.engine = (patch.engine + 1) % 8 + 8;
				}
			}

			// Model lights
			int activeEngine = voice.active_engine();
			triPhase += 2.f * engineGetSampleTime() * blockSize;
			if (triPhase >= 1.f)
				triPhase -= 1.f;
			float tri = (triPhase < 0.5f) ? triPhase * 2.f : (1.f - triPhase) * 2.f;

			for (int i = 0; i < 8; i++) {
				lights[MODEL_LIGHT + 2*i + 0].setBrightness((activeEngine == i) ? 1.f : (patch.engine == i) ? tri : 0.f);
				lights[MODEL_LIGHT + 2*i + 1].setBrightness((activeEngine == i + 8) ? 1.f : (patch.engine == i + 8) ? tri : 0.f);
			}

			// Calculate pitch for lowCpu mode if needed
			float pitch = params[FREQ_PARAM].value;
			if (lowCpu)
				pitch += log2f(48000.f * engineGetSampleTime());
			// Update patch
			patch.note = 60.f + pitch * 12.f;
			patch.harmonics = params[HARMONICS_PARAM].value;
			if (!lpg) {
				patch.timbre = params[TIMBRE_PARAM].value;
				patch.morph = params[MORPH_PARAM].value;
			}
			else {
				patch.lpg_colour = params[TIMBRE_PARAM].value;
				patch.decay = params[MORPH_PARAM].value;
			}
			patch.frequency_modulation_amount = params[FREQ_CV_PARAM].value;
			patch.timbre_modulation_amount = params[TIMBRE_CV_PARAM].value;
			patch.morph_modulation_amount = params[MORPH_CV_PARAM].value;

			// Update modulations
			modulations.engine = inputs[ENGINE_INPUT].value / 5.f;
			modulations.note = inputs[NOTE_INPUT].value * 12.f;
			modulations.frequency = inputs[FREQ_INPUT].value * 6.f;
			modulations.harmonics = inputs[HARMONICS_INPUT].value / 5.f;
			modulations.timbre = inputs[TIMBRE_INPUT].value / 8.f;
			modulations.morph = inputs[MORPH_INPUT].value / 8.f;
			// Triggers at around 0.7 V
			modulations.trigger = inputs[TRIGGER_INPUT].value / 3.f;
			modulations.level = inputs[LEVEL_INPUT].value / 8.f;

			modulations.frequency_patched = inputs[FREQ_INPUT].active;
			modulations.timbre_patched = inputs[TIMBRE_INPUT].active;
			modulations.morph_patched = inputs[MORPH_INPUT].active;
			modulations.trigger_patched = inputs[TRIGGER_INPUT].active;
			modulations.level_patched = inputs[LEVEL_INPUT].active;

			// Render frames
			plaits::Voice::Frame output[blockSize];
			voice.Render(patch, modulations, output, blockSize);

			// Convert output to frames
			Frame<2> outputFrames[blockSize];
			for (int i = 0; i < blockSize; i++) {
				outputFrames[i].samples[0] = output[i].out / 32768.f;
				outputFrames[i].samples[1] = output[i].aux / 32768.f;
			}

			// Convert output
			if (lowCpu) {
				int len = min(outputBuffer.capacity(), blockSize);
				memcpy(outputBuffer.endData(), outputFrames, len * sizeof(Frame<2>));
				outputBuffer.endIncr(len);
			}
			else {
				outputSrc.setRates(48000, engineGetSampleRate());
				int inLen = blockSize;
				int outLen = outputBuffer.capacity();
				outputSrc.process(outputFrames, &inLen, outputBuffer.endData(), &outLen);
				outputBuffer.endIncr(outLen);
			}
		}

		// Set output
		if (!outputBuffer.empty()) {
			Frame<2> outputFrame = outputBuffer.shift();
			// Inverting op-amp on outputs
			outputs[OUT_OUTPUT].value = -outputFrame.samples[0] * 5.f;
			outputs[AUX_OUTPUT].value = -outputFrame.samples[1] * 5.f;
		}
	}
};


static const std::string modelLabels[16] = {
	"Pair of classic waveforms",
	"Waveshaping oscillator",
	"Two operator FM",
	"Granular formant oscillator",
	"Harmonic oscillator",
	"Wavetable oscillator",
	"Chords",
	"Vowel and speech synthesis",
	"Granular cloud",
	"Filtered noise",
	"Particle noise",
	"Inharmonic string modeling",
	"Modal resonator",
	"Analog bass drum",
	"Analog snare drum",
	"Analog hi-hat",
};


struct PlaitsWidget : ModuleWidget {
	PlaitsWidget(Plaits *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Plaits.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(ParamWidget::create<TL1105>(mm2px(Vec(23.32685, 14.6539)), module, Plaits::MODEL1_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<TL1105>(mm2px(Vec(32.22764, 14.6539)), module, Plaits::MODEL2_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<Rogan3PSWhite>(mm2px(Vec(3.1577, 20.21088)), module, Plaits::FREQ_PARAM, -4.0, 4.0, 0.0));
		addParam(ParamWidget::create<Rogan3PSWhite>(mm2px(Vec(39.3327, 20.21088)), module, Plaits::HARMONICS_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Rogan1PSWhite>(mm2px(Vec(4.04171, 49.6562)), module, Plaits::TIMBRE_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Rogan1PSWhite>(mm2px(Vec(42.71716, 49.6562)), module, Plaits::MORPH_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Trimpot>(mm2px(Vec(7.88712, 77.60705)), module, Plaits::TIMBRE_CV_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Trimpot>(mm2px(Vec(27.2245, 77.60705)), module, Plaits::FREQ_CV_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Trimpot>(mm2px(Vec(46.56189, 77.60705)), module, Plaits::MORPH_CV_PARAM, -1.0, 1.0, 0.0));

		addInput(Port::create<PJ301MPort>(mm2px(Vec(3.31381, 92.48067)), Port::INPUT, module, Plaits::ENGINE_INPUT));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(14.75983, 92.48067)), Port::INPUT, module, Plaits::TIMBRE_INPUT));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(26.20655, 92.48067)), Port::INPUT, module, Plaits::FREQ_INPUT));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(37.65257, 92.48067)), Port::INPUT, module, Plaits::MORPH_INPUT));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(49.0986, 92.48067)), Port::INPUT, module, Plaits::HARMONICS_INPUT));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(3.31381, 107.08103)), Port::INPUT, module, Plaits::TRIGGER_INPUT));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(14.75983, 107.08103)), Port::INPUT, module, Plaits::LEVEL_INPUT));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(26.20655, 107.08103)), Port::INPUT, module, Plaits::NOTE_INPUT));

		addOutput(Port::create<PJ301MPort>(mm2px(Vec(37.65257, 107.08103)), Port::OUTPUT, module, Plaits::OUT_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(49.0986, 107.08103)), Port::OUTPUT, module, Plaits::AUX_OUTPUT));

		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(mm2px(Vec(28.79498, 23.31649)), module, Plaits::MODEL_LIGHT + 0 * 2));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(mm2px(Vec(28.79498, 28.71704)), module, Plaits::MODEL_LIGHT + 1 * 2));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(mm2px(Vec(28.79498, 34.1162)), module, Plaits::MODEL_LIGHT + 2 * 2));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(mm2px(Vec(28.79498, 39.51675)), module, Plaits::MODEL_LIGHT + 3 * 2));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(mm2px(Vec(28.79498, 44.91731)), module, Plaits::MODEL_LIGHT + 4 * 2));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(mm2px(Vec(28.79498, 50.31785)), module, Plaits::MODEL_LIGHT + 5 * 2));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(mm2px(Vec(28.79498, 55.71771)), module, Plaits::MODEL_LIGHT + 6 * 2));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(mm2px(Vec(28.79498, 61.11827)), module, Plaits::MODEL_LIGHT + 7 * 2));
	}

	void appendContextMenu(Menu *menu) override {
		Plaits *module = dynamic_cast<Plaits*>(this->module);

		struct PlaitsLowCpuItem : MenuItem {
			Plaits *module;
			void onAction(EventAction &e) override {
				module->lowCpu ^= true;
			}
		};

		struct PlaitsLPGItem : MenuItem {
			Plaits *module;
			void onAction(EventAction &e) override {
				module->lpg ^= true;
			}
		};

		struct PlaitsModelItem : MenuItem {
			Plaits *module;
			int model;
			void onAction(EventAction &e) override {
				module->patch.engine = model;
			}
		};

		menu->addChild(MenuEntry::create());
		PlaitsLowCpuItem *lowCpuItem = MenuItem::create<PlaitsLowCpuItem>("Low CPU", CHECKMARK(module->lowCpu));
		lowCpuItem->module = module;
		menu->addChild(lowCpuItem);
		PlaitsLPGItem *lpgItem = MenuItem::create<PlaitsLPGItem>("Edit LPG response/decay", CHECKMARK(module->lpg));
		lpgItem->module = module;
		menu->addChild(lpgItem);

		menu->addChild(new MenuEntry());
		menu->addChild(MenuLabel::create("Models"));
		for (int i = 0; i < 16; i++) {
			PlaitsModelItem *modelItem = MenuItem::create<PlaitsModelItem>(modelLabels[i], CHECKMARK(module->patch.engine == i));
			modelItem->module = module;
			modelItem->model = i;
			menu->addChild(modelItem);
		}
	}
};


RACK_PLUGIN_MODEL_INIT(AudibleInstruments, Plaits) {
   Model *modelPlaits = Model::create<Plaits, PlaitsWidget>("Audible Instruments", "Plaits", "Macro Oscillator 2", OSCILLATOR_TAG, WAVESHAPER_TAG);
   return modelPlaits;
}

