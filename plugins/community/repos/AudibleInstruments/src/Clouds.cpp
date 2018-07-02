#include <string.h>
#include "AudibleInstruments.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"
#include "dsp/digital.hpp"
#include "dsp/vumeter.hpp"
#include "clouds/dsp/granular_processor.h"


struct Clouds : Module {
	enum ParamIds {
		FREEZE_PARAM,
		MODE_PARAM,
		LOAD_PARAM,
		POSITION_PARAM,
		SIZE_PARAM,
		PITCH_PARAM,
		IN_GAIN_PARAM,
		DENSITY_PARAM,
		TEXTURE_PARAM,
		BLEND_PARAM,
		SPREAD_PARAM,
		FEEDBACK_PARAM,
		REVERB_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FREEZE_INPUT,
		TRIG_INPUT,
		POSITION_INPUT,
		SIZE_INPUT,
		PITCH_INPUT,
		BLEND_INPUT,
		IN_L_INPUT,
		IN_R_INPUT,
		DENSITY_INPUT,
		TEXTURE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_L_OUTPUT,
		OUT_R_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		FREEZE_LIGHT,
		MIX_GREEN_LIGHT, MIX_RED_LIGHT,
		PAN_GREEN_LIGHT, PAN_RED_LIGHT,
		FEEDBACK_GREEN_LIGHT, FEEDBACK_RED_LIGHT,
		REVERB_GREEN_LIGHT, REVERB_RED_LIGHT,
		NUM_LIGHTS
	};

	SampleRateConverter<2> inputSrc;
	SampleRateConverter<2> outputSrc;
	DoubleRingBuffer<Frame<2>, 256> inputBuffer;
	DoubleRingBuffer<Frame<2>, 256> outputBuffer;

	uint8_t *block_mem;
	uint8_t *block_ccm;
	clouds::GranularProcessor *processor;

	bool triggered = false;

	SchmittTrigger freezeTrigger;
	bool freeze = false;
	SchmittTrigger blendTrigger;
	int blendMode = 0;

	clouds::PlaybackMode playback;
	int quality = 0;

	Clouds();
	~Clouds();
	void step() override;

	void onReset() override {
		freeze = false;
		blendMode = 0;
		playback = clouds::PLAYBACK_MODE_GRANULAR;
		quality = 0;
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();

		json_object_set_new(rootJ, "playback", json_integer((int) playback));
		json_object_set_new(rootJ, "quality", json_integer(quality));
		json_object_set_new(rootJ, "blendMode", json_integer(blendMode));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *playbackJ = json_object_get(rootJ, "playback");
		if (playbackJ) {
			playback = (clouds::PlaybackMode) json_integer_value(playbackJ);
		}

		json_t *qualityJ = json_object_get(rootJ, "quality");
		if (qualityJ) {
			quality = json_integer_value(qualityJ);
		}

		json_t *blendModeJ = json_object_get(rootJ, "blendMode");
		if (blendModeJ) {
			blendMode = json_integer_value(blendModeJ);
		}
	}
};


Clouds::Clouds() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	const int memLen = 118784;
	const int ccmLen = 65536 - 128;
	block_mem = new uint8_t[memLen]();
	block_ccm = new uint8_t[ccmLen]();
	processor = new clouds::GranularProcessor();
	memset(processor, 0, sizeof(*processor));

	processor->Init(block_mem, memLen, block_ccm, ccmLen);
	onReset();
}

Clouds::~Clouds() {
	delete processor;
	delete[] block_mem;
	delete[] block_ccm;
}

void Clouds::step() {
	// Get input
	Frame<2> inputFrame = {};
	if (!inputBuffer.full()) {
		inputFrame.samples[0] = inputs[IN_L_INPUT].value * params[IN_GAIN_PARAM].value / 5.0;
		inputFrame.samples[1] = inputs[IN_R_INPUT].active ? inputs[IN_R_INPUT].value * params[IN_GAIN_PARAM].value / 5.0 : inputFrame.samples[0];
		inputBuffer.push(inputFrame);
	}

	if (freezeTrigger.process(params[FREEZE_PARAM].value)) {
		freeze ^= true;
	}
	if (blendTrigger.process(params[MODE_PARAM].value)) {
		blendMode = (blendMode + 1) % 4;
	}

	// Trigger
	if (inputs[TRIG_INPUT].value >= 1.0) {
		triggered = true;
	}

	// Render frames
	if (outputBuffer.empty()) {
		clouds::ShortFrame input[32] = {};
		// Convert input buffer
		{
			inputSrc.setRates(engineGetSampleRate(), 32000);
			Frame<2> inputFrames[32];
			int inLen = inputBuffer.size();
			int outLen = 32;
			inputSrc.process(inputBuffer.startData(), &inLen, inputFrames, &outLen);
			inputBuffer.startIncr(inLen);

			// We might not fill all of the input buffer if there is a deficiency, but this cannot be avoided due to imprecisions between the input and output SRC.
			for (int i = 0; i < outLen; i++) {
				input[i].l = clamp(inputFrames[i].samples[0] * 32767.0f, -32768.0f, 32767.0f);
				input[i].r = clamp(inputFrames[i].samples[1] * 32767.0f, -32768.0f, 32767.0f);
			}
		}

		// Set up processor
		processor->set_playback_mode(playback);
		processor->set_quality(quality);
		processor->Prepare();

		clouds::Parameters *p = processor->mutable_parameters();
		p->trigger = triggered;
		p->gate = triggered;
		p->freeze = freeze || (inputs[FREEZE_INPUT].value >= 1.0);
		p->position = clamp(params[POSITION_PARAM].value + inputs[POSITION_INPUT].value / 5.0f, 0.0f, 1.0f);
		p->size = clamp(params[SIZE_PARAM].value + inputs[SIZE_INPUT].value / 5.0f, 0.0f, 1.0f);
		p->pitch = clamp((params[PITCH_PARAM].value + inputs[PITCH_INPUT].value) * 12.0f, -48.0f, 48.0f);
		p->density = clamp(params[DENSITY_PARAM].value + inputs[DENSITY_INPUT].value / 5.0f, 0.0f, 1.0f);
		p->texture = clamp(params[TEXTURE_PARAM].value + inputs[TEXTURE_INPUT].value / 5.0f, 0.0f, 1.0f);
		p->dry_wet = params[BLEND_PARAM].value;
		p->stereo_spread = params[SPREAD_PARAM].value;
		p->feedback = params[FEEDBACK_PARAM].value;
		// TODO
		// Why doesn't dry audio get reverbed?
		p->reverb = params[REVERB_PARAM].value;
		float blend = inputs[BLEND_INPUT].value / 5.0f;
		switch (blendMode) {
			case 0:
				p->dry_wet += blend;
				p->dry_wet = clamp(p->dry_wet, 0.0f, 1.0f);
				break;
			case 1:
				p->stereo_spread += blend;
				p->stereo_spread = clamp(p->stereo_spread, 0.0f, 1.0f);
				break;
			case 2:
				p->feedback += blend;
				p->feedback = clamp(p->feedback, 0.0f, 1.0f);
				break;
			case 3:
				p->reverb += blend;
				p->reverb = clamp(p->reverb, 0.0f, 1.0f);
				break;
		}

		clouds::ShortFrame output[32];
		processor->Process(input, output, 32);

		// Convert output buffer
		{
			Frame<2> outputFrames[32];
			for (int i = 0; i < 32; i++) {
				outputFrames[i].samples[0] = output[i].l / 32768.0;
				outputFrames[i].samples[1] = output[i].r / 32768.0;
			}

			outputSrc.setRates(32000, engineGetSampleRate());
			int inLen = 32;
			int outLen = outputBuffer.capacity();
			outputSrc.process(outputFrames, &inLen, outputBuffer.endData(), &outLen);
			outputBuffer.endIncr(outLen);
		}

		triggered = false;
	}

	// Set output
	Frame<2> outputFrame = {};
	if (!outputBuffer.empty()) {
		outputFrame = outputBuffer.shift();
		outputs[OUT_L_OUTPUT].value = 5.0 * outputFrame.samples[0];
		outputs[OUT_R_OUTPUT].value = 5.0 * outputFrame.samples[1];
	}

	// Lights
	clouds::Parameters *p = processor->mutable_parameters();
	VUMeter vuMeter;
	vuMeter.dBInterval = 6.0;
	Frame<2> lightFrame = p->freeze ? outputFrame : inputFrame;
	vuMeter.setValue(fmaxf(fabsf(lightFrame.samples[0]), fabsf(lightFrame.samples[1])));
	lights[FREEZE_LIGHT].setBrightness(p->freeze ? 0.75 : 0.0);
	lights[MIX_GREEN_LIGHT].setBrightnessSmooth(vuMeter.getBrightness(3));
	lights[PAN_GREEN_LIGHT].setBrightnessSmooth(vuMeter.getBrightness(2));
	lights[FEEDBACK_GREEN_LIGHT].setBrightnessSmooth(vuMeter.getBrightness(1));
	lights[REVERB_GREEN_LIGHT].setBrightness(0.0);
	lights[MIX_RED_LIGHT].setBrightness(0.0);
	lights[PAN_RED_LIGHT].setBrightness(0.0);
	lights[FEEDBACK_RED_LIGHT].setBrightnessSmooth(vuMeter.getBrightness(1));
	lights[REVERB_RED_LIGHT].setBrightnessSmooth(vuMeter.getBrightness(0));
}



struct FreezeLight : YellowLight {
	FreezeLight() {
		box.size = Vec(28-6, 28-6);
		bgColor = COLOR_BLACK_TRANSPARENT;
	}
};


struct CloudsBlendItem : MenuItem {
	Clouds *module;
	int blendMode;
	void onAction(EventAction &e) override {
		module->blendMode = blendMode;
	}
	void step() override {
		rightText = (module->blendMode == blendMode) ? "✔" : "";
		MenuItem::step();
	}
};


struct CloudsPlaybackItem : MenuItem {
	Clouds *module;
	clouds::PlaybackMode playback;
	void onAction(EventAction &e) override {
		module->playback = playback;
	}
	void step() override {
		rightText = (module->playback == playback) ? "✔" : "";
		MenuItem::step();
	}
};


struct CloudsQualityItem : MenuItem {
	Clouds *module;
	int quality;
	void onAction(EventAction &e) override {
		module->quality = quality;
	}
	void step() override {
		rightText = (module->quality == quality) ? "✔" : "";
		MenuItem::step();
	}
};


struct CloudsWidget : ModuleWidget {
	ParamWidget *blendParam;
	ParamWidget *spreadParam;
	ParamWidget *feedbackParam;
	ParamWidget *reverbParam;

	CloudsWidget(Clouds *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Clouds.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(240, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(240, 365)));

		addParam(ParamWidget::create<Rogan3PSRed>(Vec(27, 93), module, Clouds::POSITION_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Rogan3PSGreen>(Vec(108, 93), module, Clouds::SIZE_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Rogan3PSWhite>(Vec(190, 93), module, Clouds::PITCH_PARAM, -2.0, 2.0, 0.0));

		addParam(ParamWidget::create<Rogan1PSRed>(Vec(14, 180), module, Clouds::IN_GAIN_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Rogan1PSRed>(Vec(81, 180), module, Clouds::DENSITY_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Rogan1PSGreen>(Vec(146, 180), module, Clouds::TEXTURE_PARAM, 0.0, 1.0, 0.5));
		blendParam = ParamWidget::create<Rogan1PSWhite>(Vec(213, 180), module, Clouds::BLEND_PARAM, 0.0, 1.0, 0.5);
		addParam(blendParam);
		spreadParam = ParamWidget::create<Rogan1PSRed>(Vec(213, 180), module, Clouds::SPREAD_PARAM, 0.0, 1.0, 0.0);
		addParam(spreadParam);
		feedbackParam = ParamWidget::create<Rogan1PSGreen>(Vec(213, 180), module, Clouds::FEEDBACK_PARAM, 0.0, 1.0, 0.0);
		addParam(feedbackParam);
		reverbParam = ParamWidget::create<Rogan1PSBlue>(Vec(213, 180), module, Clouds::REVERB_PARAM, 0.0, 1.0, 0.0);
		addParam(reverbParam);

		addParam(ParamWidget::create<CKD6>(Vec(12, 43), module, Clouds::FREEZE_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<TL1105>(Vec(211, 50), module, Clouds::MODE_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<TL1105>(Vec(239, 50), module, Clouds::LOAD_PARAM, 0.0, 1.0, 0.0));

		addInput(Port::create<PJ301MPort>(Vec(15, 274), Port::INPUT, module, Clouds::FREEZE_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(58, 274), Port::INPUT, module, Clouds::TRIG_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(101, 274), Port::INPUT, module, Clouds::POSITION_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(144, 274), Port::INPUT, module, Clouds::SIZE_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(188, 274), Port::INPUT, module, Clouds::PITCH_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(230, 274), Port::INPUT, module, Clouds::BLEND_INPUT));

		addInput(Port::create<PJ301MPort>(Vec(15, 317), Port::INPUT, module, Clouds::IN_L_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(58, 317), Port::INPUT, module, Clouds::IN_R_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(101, 317), Port::INPUT, module, Clouds::DENSITY_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(144, 317), Port::INPUT, module, Clouds::TEXTURE_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(188, 317), Port::OUTPUT, module, Clouds::OUT_L_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(230, 317), Port::OUTPUT, module, Clouds::OUT_R_OUTPUT));

		addChild(ModuleLightWidget::create<FreezeLight>(Vec(12+3, 43+3), module, Clouds::FREEZE_LIGHT));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(82.5, 53), module, Clouds::MIX_GREEN_LIGHT));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(114.5, 53), module, Clouds::PAN_GREEN_LIGHT));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(145.5, 53), module, Clouds::FEEDBACK_GREEN_LIGHT));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(177.5, 53), module, Clouds::REVERB_GREEN_LIGHT));
	}

	void step() override {
		Clouds *module = dynamic_cast<Clouds*>(this->module);

		blendParam->visible = (module->blendMode == 0);
		spreadParam->visible = (module->blendMode == 1);
		feedbackParam->visible = (module->blendMode == 2);
		reverbParam->visible = (module->blendMode == 3);

		ModuleWidget::step();
	}

	void appendContextMenu(Menu *menu) override {
		Clouds *module = dynamic_cast<Clouds*>(this->module);
		assert(module);

		menu->addChild(construct<MenuLabel>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Blend knob"));
		menu->addChild(construct<CloudsBlendItem>(&MenuItem::text, "Wet/dry", &CloudsBlendItem::module, module, &CloudsBlendItem::blendMode, 0));
		menu->addChild(construct<CloudsBlendItem>(&MenuItem::text, "Spread", &CloudsBlendItem::module, module, &CloudsBlendItem::blendMode, 1));
		menu->addChild(construct<CloudsBlendItem>(&MenuItem::text, "Feedback", &CloudsBlendItem::module, module, &CloudsBlendItem::blendMode, 2));
		menu->addChild(construct<CloudsBlendItem>(&MenuItem::text, "Reverb", &CloudsBlendItem::module, module, &CloudsBlendItem::blendMode, 3));

		menu->addChild(construct<MenuLabel>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Alternative mode"));
		menu->addChild(construct<CloudsPlaybackItem>(&MenuItem::text, "Granular", &CloudsPlaybackItem::module, module, &CloudsPlaybackItem::playback, clouds::PLAYBACK_MODE_GRANULAR));
		menu->addChild(construct<CloudsPlaybackItem>(&MenuItem::text, "Pitch-shifter/time-stretcher", &CloudsPlaybackItem::module, module, &CloudsPlaybackItem::playback, clouds::PLAYBACK_MODE_STRETCH));
		menu->addChild(construct<CloudsPlaybackItem>(&MenuItem::text, "Looping delay", &CloudsPlaybackItem::module, module, &CloudsPlaybackItem::playback, clouds::PLAYBACK_MODE_LOOPING_DELAY));
		menu->addChild(construct<CloudsPlaybackItem>(&MenuItem::text, "Spectral madness", &CloudsPlaybackItem::module, module, &CloudsPlaybackItem::playback, clouds::PLAYBACK_MODE_SPECTRAL));

		menu->addChild(construct<MenuLabel>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Quality"));
		menu->addChild(construct<CloudsQualityItem>(&MenuItem::text, "1s 32kHz 16-bit stereo", &CloudsQualityItem::module, module, &CloudsQualityItem::quality, 0));
		menu->addChild(construct<CloudsQualityItem>(&MenuItem::text, "2s 32kHz 16-bit mono", &CloudsQualityItem::module, module, &CloudsQualityItem::quality, 1));
		menu->addChild(construct<CloudsQualityItem>(&MenuItem::text, "4s 16kHz 8-bit µ-law stereo", &CloudsQualityItem::module, module, &CloudsQualityItem::quality, 2));
		menu->addChild(construct<CloudsQualityItem>(&MenuItem::text, "8s 16kHz 8-bit µ-law mono", &CloudsQualityItem::module, module, &CloudsQualityItem::quality, 3));
	}
};


RACK_PLUGIN_MODEL_INIT(AudibleInstruments, Clouds) {
   Model *modelClouds = Model::create<Clouds, CloudsWidget>("Audible Instruments", "Clouds", "Texture Synthesizer", GRANULAR_TAG, REVERB_TAG);
   return modelClouds;
}
