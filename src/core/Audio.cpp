#include <mutex>
#include <chrono>
#include <thread>
#include <condition_variable>

#include "plugin.hpp"
#include <audio.hpp>
#include <context.hpp>


namespace rack {
namespace core {


template <int NUM_AUDIO_INPUTS, int NUM_AUDIO_OUTPUTS>
struct AudioPort : audio::Port {
	Module* module;

	dsp::DoubleRingBuffer<dsp::Frame<NUM_AUDIO_INPUTS>, 32768> engineInputBuffer;
	dsp::DoubleRingBuffer<dsp::Frame<NUM_AUDIO_OUTPUTS>, 32768> engineOutputBuffer;

	dsp::SampleRateConverter<NUM_AUDIO_INPUTS> inputSrc;
	dsp::SampleRateConverter<NUM_AUDIO_OUTPUTS> outputSrc;

	// Port variable caches
	int deviceNumInputs = 0;
	int deviceNumOutputs = 0;
	float deviceSampleRate = 0.f;
	int requestedEngineFrames = 0;

	AudioPort(Module* module) {
		this->module = module;
		maxOutputs = NUM_AUDIO_INPUTS;
		maxInputs = NUM_AUDIO_OUTPUTS;
		inputSrc.setQuality(6);
		outputSrc.setQuality(6);
	}

	void setMaster(bool master = true) {
		if (master) {
			APP->engine->setMasterModule(module);
		}
		else {
			// Unset master only if module is currently master
			if (isMaster())
				APP->engine->setMasterModule(NULL);
		}
	}

	bool isMaster() {
		return APP->engine->getMasterModule() == module;
	}

	void processInput(const float* input, int inputStride, int frames) override {
		deviceNumInputs = std::min(getNumInputs(), NUM_AUDIO_OUTPUTS);
		deviceNumOutputs = std::min(getNumOutputs(), NUM_AUDIO_INPUTS);
		deviceSampleRate = getSampleRate();

		// DEBUG("%p: new device block ____________________________", this);
		// Claim master module if there is none
		if (!APP->engine->getMasterModule()) {
			setMaster();
		}
		bool isMasterCached = isMaster();

		// Set sample rate of engine if engine sample rate is "auto".
		if (isMasterCached) {
			APP->engine->setSuggestedSampleRate(deviceSampleRate);
		}

		float engineSampleRate = APP->engine->getSampleRate();
		float sampleRateRatio = engineSampleRate / deviceSampleRate;

		// DEBUG("%p: %d block, engineOutputBuffer still has %d", this, frames, (int) engineOutputBuffer.size());

		// Consider engine buffers "too full" if they contain a bit more than the audio device's number of frames, converted to engine sample rate.
		int maxEngineFrames = (int) std::ceil(frames * sampleRateRatio * 2.0) - 1;
		// If the engine output buffer is too full, clear it to keep latency low. No need to clear if master because it's always cleared below.
		if (!isMasterCached && (int) engineOutputBuffer.size() > maxEngineFrames) {
			engineOutputBuffer.clear();
			// DEBUG("%p: clearing engine output", this);
		}

		if (deviceNumInputs > 0) {
			// Always clear engine output if master
			if (isMasterCached) {
				engineOutputBuffer.clear();
			}
			// Set up sample rate converter
			outputSrc.setRates(deviceSampleRate, engineSampleRate);
			outputSrc.setChannels(deviceNumInputs);
			int inputFrames = frames;
			int outputFrames = engineOutputBuffer.capacity();
			outputSrc.process(input, inputStride, &inputFrames, (float*) engineOutputBuffer.endData(), NUM_AUDIO_OUTPUTS, &outputFrames);
			engineOutputBuffer.endIncr(outputFrames);
			// Request exactly as many frames as we have in the engine output buffer.
			requestedEngineFrames = engineOutputBuffer.size();
		}
		else {
			// Upper bound on number of frames so that `audioOutputFrames >= frames` when processOutput() is called.
			requestedEngineFrames = std::max((int) std::ceil(frames * sampleRateRatio) - (int) engineInputBuffer.size(), 0);
		}
	}

	void processBuffer(const float* input, int inputStride, float* output, int outputStride, int frames) override {
		// Step engine
		if (isMaster() && requestedEngineFrames > 0) {
			// DEBUG("%p: %d block, stepping %d", this, frames, requestedEngineFrames);
			APP->engine->stepBlock(requestedEngineFrames);
		}
	}

	void processOutput(float* output, int outputStride, int frames) override {
		// bool isMasterCached = isMaster();
		float engineSampleRate = APP->engine->getSampleRate();
		float sampleRateRatio = engineSampleRate / deviceSampleRate;

		if (deviceNumOutputs > 0) {
			// Set up sample rate converter
			inputSrc.setRates(engineSampleRate, deviceSampleRate);
			inputSrc.setChannels(deviceNumOutputs);
			// Convert engine input -> audio output
			int inputFrames = engineInputBuffer.size();
			int outputFrames = frames;
			inputSrc.process((const float*) engineInputBuffer.startData(), NUM_AUDIO_INPUTS, &inputFrames, output, outputStride, &outputFrames);
			engineInputBuffer.startIncr(inputFrames);
			// Clamp output samples
			for (int i = 0; i < outputFrames; i++) {
				for (int j = 0; j < deviceNumOutputs; j++) {
					float v = output[i * outputStride + j];
					v = clamp(v, -1.f, 1.f);
					output[i * outputStride + j] = v;
				}
			}
			// Fill the rest of the audio output buffer with zeros
			for (int i = outputFrames; i < frames; i++) {
				for (int j = 0; j < deviceNumOutputs; j++) {
					output[i * outputStride + j] = 0.f;
				}
			}
		}

		// DEBUG("%p: %d block, engineInputBuffer left %d", this, frames, (int) engineInputBuffer.size());

		// If the engine input buffer is too full, clear it to keep latency low.
		int maxEngineFrames = (int) std::ceil(frames * sampleRateRatio * 2.0) - 1;
		if ((int) engineInputBuffer.size() > maxEngineFrames) {
			engineInputBuffer.clear();
			// DEBUG("%p: clearing engine input", this);
		}

		// DEBUG("%p %s:\tframes %d requestedEngineFrames %d\toutputBuffer %d engineInputBuffer %d\t", this, isMasterCached ? "master" : "secondary", frames, requestedEngineFrames, engineOutputBuffer.size(), engineInputBuffer.size());
	}

	void onStartStream() override {
		engineInputBuffer.clear();
		engineOutputBuffer.clear();
		// DEBUG("onStartStream");
	}

	void onStopStream() override {
		deviceNumInputs = 0;
		deviceNumOutputs = 0;
		deviceSampleRate = 0.f;
		engineInputBuffer.clear();
		engineOutputBuffer.clear();
		// We can be in an Engine write-lock here (e.g. onReset() calls this indirectly), so use non-locking master module API.
		// setMaster(false);
		if (APP->engine->getMasterModule() == module)
			APP->engine->setMasterModule_NoLock(NULL);
		// DEBUG("onStopStream");
	}
};


template <int NUM_AUDIO_INPUTS, int NUM_AUDIO_OUTPUTS>
struct Audio : Module {
	static constexpr int NUM_INPUT_LIGHTS = (NUM_AUDIO_INPUTS > 2) ? (NUM_AUDIO_INPUTS / 2) : 0;
	static constexpr int NUM_OUTPUT_LIGHTS = (NUM_AUDIO_OUTPUTS > 2) ? (NUM_AUDIO_OUTPUTS / 2) : 0;

	enum ParamIds {
		ENUMS(LEVEL_PARAM, NUM_AUDIO_INPUTS == 2),
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(AUDIO_INPUTS, NUM_AUDIO_INPUTS),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(AUDIO_OUTPUTS, NUM_AUDIO_OUTPUTS),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(INPUT_LIGHTS, NUM_INPUT_LIGHTS * 2),
		ENUMS(OUTPUT_LIGHTS, NUM_OUTPUT_LIGHTS * 2),
		ENUMS(VU_LIGHTS, (NUM_AUDIO_INPUTS == 2) ? (2 * 6) : 0),
		NUM_LIGHTS
	};

	AudioPort<NUM_AUDIO_INPUTS, NUM_AUDIO_OUTPUTS> port;

	dsp::RCFilter dcFilters[NUM_AUDIO_INPUTS];
	bool dcFilterEnabled = false;

	dsp::ClockDivider lightDivider;
	// For each pair of inputs/outputs
	float inputClipTimers[(NUM_AUDIO_INPUTS > 0) ? NUM_INPUT_LIGHTS : 0] = {};
	float outputClipTimers[(NUM_AUDIO_INPUTS > 0) ? NUM_OUTPUT_LIGHTS : 0] = {};
	dsp::VuMeter2 vuMeter[(NUM_AUDIO_INPUTS == 2) ? 2 : 0];

	Audio() : port(this) {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		if (NUM_AUDIO_INPUTS == 2)
			configParam(LEVEL_PARAM, 0.f, 2.f, 1.f, "Level", " dB", -10, 40);
		for (int i = 0; i < NUM_AUDIO_INPUTS; i++)
			configInput(AUDIO_INPUTS + i, string::f("To \"device output %d\"", i + 1));
		for (int i = 0; i < NUM_AUDIO_OUTPUTS; i++)
			configOutput(AUDIO_OUTPUTS + i, string::f("From \"device input %d\"", i + 1));
		for (int i = 0; i < NUM_INPUT_LIGHTS; i++)
			configLight(INPUT_LIGHTS + 2 * i, string::f("Device output %d/%d status", 2 * i + 1, 2 * i + 2));
		for (int i = 0; i < NUM_OUTPUT_LIGHTS; i++)
			configLight(OUTPUT_LIGHTS + 2 * i, string::f("Device input %d/%d status", 2 * i + 1, 2 * i + 2));

		lightDivider.setDivision(512);

		float sampleTime = APP->engine->getSampleTime();
		for (int i = 0; i < NUM_AUDIO_INPUTS; i++) {
			dcFilters[i].setCutoffFreq(10.f * sampleTime);
		}

		onReset();
	}

	~Audio() {
		// Close stream here before destructing AudioPort, so processBuffer() etc are not called on another thread while destructing.
		port.setDriverId(-1);
	}

	void onReset() override {
		port.setDriverId(-1);

		if (NUM_AUDIO_INPUTS == 2)
			dcFilterEnabled = true;
		else
			dcFilterEnabled = false;
	}

	void onSampleRateChange(const SampleRateChangeEvent& e) override {
		port.engineInputBuffer.clear();
		port.engineOutputBuffer.clear();

		for (int i = 0; i < NUM_AUDIO_INPUTS; i++) {
			dcFilters[i].setCutoffFreq(10.f * e.sampleTime);
		}
	}

	void process(const ProcessArgs& args) override {
		const float clipTime = 0.25f;

		// Push inputs to buffer
		if (port.deviceNumOutputs > 0) {
			dsp::Frame<NUM_AUDIO_INPUTS> inputFrame = {};
			for (int i = 0; i < port.deviceNumOutputs; i++) {
				// Get input
				float v = 0.f;
				if (inputs[AUDIO_INPUTS + i].isConnected())
					v = inputs[AUDIO_INPUTS + i].getVoltageSum() / 10.f;
				// Normalize right input to left on Audio-2
				else if (i == 1 && NUM_AUDIO_INPUTS == 2)
					v = inputFrame.samples[0];

				// Apply DC filter
				if (dcFilterEnabled) {
					dcFilters[i].process(v);
					v = dcFilters[i].highpass();
				}

				// Detect clipping
				if (NUM_AUDIO_INPUTS > 2) {
					if (std::fabs(v) >= 1.f)
						inputClipTimers[i / 2] = clipTime;
				}
				inputFrame.samples[i] = v;
			}

			// Audio-2: Apply gain from knob
			if (NUM_AUDIO_INPUTS == 2) {
				float gain = std::pow(params[LEVEL_PARAM].getValue(), 2.f);
				for (int i = 0; i < NUM_AUDIO_INPUTS; i++) {
					inputFrame.samples[i] *= gain;
				}
			}

			if (!port.engineInputBuffer.full()) {
				port.engineInputBuffer.push(inputFrame);
			}

			// Audio-2: VU meter process
			if (NUM_AUDIO_INPUTS == 2) {
				for (int i = 0; i < NUM_AUDIO_INPUTS; i++) {
					vuMeter[i].process(args.sampleTime, inputFrame.samples[i]);
				}
			}
		}
		else {
			// Audio-2: Clear VU meter
			if (NUM_AUDIO_INPUTS == 2) {
				for (int i = 0; i < NUM_AUDIO_INPUTS; i++) {
					vuMeter[i].reset();
				}
			}
		}

		// Pull outputs from buffer
		if (!port.engineOutputBuffer.empty()) {
			dsp::Frame<NUM_AUDIO_OUTPUTS> outputFrame = port.engineOutputBuffer.shift();
			for (int i = 0; i < NUM_AUDIO_OUTPUTS; i++) {
				float v = outputFrame.samples[i];
				outputs[AUDIO_OUTPUTS + i].setVoltage(10.f * v);

				// Detect clipping
				if (NUM_AUDIO_OUTPUTS > 2) {
					if (std::fabs(v) >= 1.f)
						outputClipTimers[i / 2] = clipTime;
				}
			}
		}
		else {
			// Zero outputs
			for (int i = 0; i < NUM_AUDIO_OUTPUTS; i++) {
				outputs[AUDIO_OUTPUTS + i].setVoltage(0.f);
			}
		}

		// Lights
		if (lightDivider.process()) {
			float lightTime = args.sampleTime * lightDivider.getDivision();
			// Audio-2: VU meter
			if (NUM_AUDIO_INPUTS == 2) {
				for (int i = 0; i < NUM_AUDIO_INPUTS; i++) {
					lights[VU_LIGHTS + i * 6 + 0].setBrightness(vuMeter[i].getBrightness(0, 0));
					lights[VU_LIGHTS + i * 6 + 1].setBrightness(vuMeter[i].getBrightness(-6, -3));
					lights[VU_LIGHTS + i * 6 + 2].setBrightness(vuMeter[i].getBrightness(-12, -6));
					lights[VU_LIGHTS + i * 6 + 3].setBrightness(vuMeter[i].getBrightness(-24, -12));
					lights[VU_LIGHTS + i * 6 + 4].setBrightness(vuMeter[i].getBrightness(-36, -24));
					lights[VU_LIGHTS + i * 6 + 5].setBrightness(vuMeter[i].getBrightness(-48, -36));
				}
			}
			// Audio-8 and Audio-16: pair state lights
			else {
				// Turn on light if at least one port is enabled in the nearby pair.
				for (int i = 0; i < NUM_AUDIO_INPUTS / 2; i++) {
					bool active = port.deviceNumOutputs >= 2 * i + 1;
					bool clip = inputClipTimers[i] > 0.f;
					if (clip)
						inputClipTimers[i] -= lightTime;
					lights[INPUT_LIGHTS + i * 2 + 0].setBrightness(active && !clip);
					lights[INPUT_LIGHTS + i * 2 + 1].setBrightness(active && clip);
				}
				for (int i = 0; i < NUM_AUDIO_OUTPUTS / 2; i++) {
					bool active = port.deviceNumInputs >= 2 * i + 1;
					bool clip = outputClipTimers[i] > 0.f;
					if (clip)
						outputClipTimers[i] -= lightTime;
					lights[OUTPUT_LIGHTS + i * 2 + 0].setBrightness(active & !clip);
					lights[OUTPUT_LIGHTS + i * 2 + 1].setBrightness(active & clip);
				}
			}
		}
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "audio", port.toJson());

		json_object_set_new(rootJ, "dcFilter", json_boolean(dcFilterEnabled));

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* audioJ = json_object_get(rootJ, "audio");
		if (audioJ)
			port.fromJson(audioJ);

		json_t* dcFilterJ = json_object_get(rootJ, "dcFilter");
		if (dcFilterJ)
			dcFilterEnabled = json_boolean_value(dcFilterJ);
	}
};


/** For Audio-2 module. */
struct Audio2Display : LedDisplay {
	AudioDeviceMenuChoice* deviceChoice;
	LedDisplaySeparator* deviceSeparator;

	void setAudioPort(audio::Port* port) {
		math::Vec pos;

		deviceChoice = createWidget<AudioDeviceMenuChoice>(math::Vec());
		deviceChoice->box.size.x = box.size.x;
		deviceChoice->port = port;
		addChild(deviceChoice);
		pos = deviceChoice->box.getBottomLeft();

		deviceSeparator = createWidget<LedDisplaySeparator>(pos);
		deviceSeparator->box.size.x = box.size.x;
		addChild(deviceSeparator);
	}

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer == 1) {
			static const std::vector<float> posY = {
				mm2px(28.899 - 13.039),
				mm2px(34.196 - 13.039),
				mm2px(39.494 - 13.039),
				mm2px(44.791 - 13.039),
				mm2px(50.089 - 13.039),
				mm2px(55.386 - 13.039),
			};
			static const std::vector<std::string> texts = {
				" 0", "-3", "-6", "-12", "-24", "-36",
			};

			std::string fontPath = asset::system("res/fonts/Nunito-Bold.ttf");
			std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
			if (!font)
				return;

			nvgSave(args.vg);
			nvgFontFaceId(args.vg, font->handle);
			nvgFontSize(args.vg, 11);
			nvgTextLetterSpacing(args.vg, 0.0);
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
			nvgFillColor(args.vg, nvgRGB(99, 99, 99));

			for (int i = 0; i < 6; i++) {
				nvgText(args.vg, 36.0, posY[i], texts[i].c_str(), NULL);
			}
			nvgRestore(args.vg);
		}
		LedDisplay::drawLayer(args, layer);
	}
};


template <int NUM_AUDIO_INPUTS, int NUM_AUDIO_OUTPUTS>
struct AudioWidget : ModuleWidget {
	typedef Audio<NUM_AUDIO_INPUTS, NUM_AUDIO_OUTPUTS> TAudio;

	AudioWidget(TAudio* module) {
		setModule(module);

		if (NUM_AUDIO_INPUTS == 8 && NUM_AUDIO_OUTPUTS == 8) {
			setPanel(Svg::load(asset::system("res/Core/Audio8.svg")));

			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.81, 57.929)), module, TAudio::AUDIO_INPUTS + 0));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.359, 57.929)), module, TAudio::AUDIO_INPUTS + 1));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.909, 57.929)), module, TAudio::AUDIO_INPUTS + 2));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.459, 57.929)), module, TAudio::AUDIO_INPUTS + 3));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.81, 74.286)), module, TAudio::AUDIO_INPUTS + 4));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.359, 74.286)), module, TAudio::AUDIO_INPUTS + 5));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.909, 74.286)), module, TAudio::AUDIO_INPUTS + 6));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.459, 74.286)), module, TAudio::AUDIO_INPUTS + 7));

			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.81, 96.859)), module, TAudio::AUDIO_OUTPUTS + 0));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19.359, 96.859)), module, TAudio::AUDIO_OUTPUTS + 1));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.909, 96.859)), module, TAudio::AUDIO_OUTPUTS + 2));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.459, 96.859)), module, TAudio::AUDIO_OUTPUTS + 3));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.81, 113.115)), module, TAudio::AUDIO_OUTPUTS + 4));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19.359, 113.115)), module, TAudio::AUDIO_OUTPUTS + 5));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.909, 113.115)), module, TAudio::AUDIO_OUTPUTS + 6));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.459, 113.115)), module, TAudio::AUDIO_OUTPUTS + 7));

			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(13.54, 52.168)), module, TAudio::INPUT_LIGHTS + 2 * 0));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(36.774, 52.168)), module, TAudio::INPUT_LIGHTS + 2 * 1));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(13.54, 68.53)), module, TAudio::INPUT_LIGHTS + 2 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(36.774, 68.53)), module, TAudio::INPUT_LIGHTS + 2 * 3));

			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(13.54, 90.791)), module, TAudio::OUTPUT_LIGHTS + 2 * 0));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(36.638, 90.791)), module, TAudio::OUTPUT_LIGHTS + 2 * 1));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(13.54, 107.097)), module, TAudio::OUTPUT_LIGHTS + 2 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(36.638, 107.097)), module, TAudio::OUTPUT_LIGHTS + 2 * 3));

			AudioDisplay* display = createWidget<AudioDisplay>(mm2px(Vec(0.0, 13.039)));
			display->box.size = mm2px(Vec(50.8, 29.021));
			display->setAudioPort(module ? &module->port : NULL);
			addChild(display);
		}
		else if (NUM_AUDIO_INPUTS == 16 && NUM_AUDIO_OUTPUTS == 16) {
			setPanel(Svg::load(asset::system("res/Core/Audio16.svg")));

			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.815, 57.929)), module, TAudio::AUDIO_INPUTS + 0));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.364, 57.929)), module, TAudio::AUDIO_INPUTS + 1));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.914, 57.929)), module, TAudio::AUDIO_INPUTS + 2));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.464, 57.929)), module, TAudio::AUDIO_INPUTS + 3));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.015, 57.929)), module, TAudio::AUDIO_INPUTS + 4));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(65.565, 57.914)), module, TAudio::AUDIO_INPUTS + 5));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(77.114, 57.914)), module, TAudio::AUDIO_INPUTS + 6));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(88.664, 57.914)), module, TAudio::AUDIO_INPUTS + 7));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.815, 74.276)), module, TAudio::AUDIO_INPUTS + 8));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.364, 74.276)), module, TAudio::AUDIO_INPUTS + 9));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.914, 74.276)), module, TAudio::AUDIO_INPUTS + 10));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.464, 74.276)), module, TAudio::AUDIO_INPUTS + 11));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.015, 74.291)), module, TAudio::AUDIO_INPUTS + 12));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(65.565, 74.276)), module, TAudio::AUDIO_INPUTS + 13));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(77.114, 74.276)), module, TAudio::AUDIO_INPUTS + 14));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(88.664, 74.276)), module, TAudio::AUDIO_INPUTS + 15));

			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.815, 96.859)), module, TAudio::AUDIO_OUTPUTS + 0));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19.364, 96.859)), module, TAudio::AUDIO_OUTPUTS + 1));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.914, 96.859)), module, TAudio::AUDIO_OUTPUTS + 2));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.464, 96.859)), module, TAudio::AUDIO_OUTPUTS + 3));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.015, 96.859)), module, TAudio::AUDIO_OUTPUTS + 4));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(65.565, 96.859)), module, TAudio::AUDIO_OUTPUTS + 5));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(77.114, 96.859)), module, TAudio::AUDIO_OUTPUTS + 6));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(88.664, 96.859)), module, TAudio::AUDIO_OUTPUTS + 7));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.815, 113.115)), module, TAudio::AUDIO_OUTPUTS + 8));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19.364, 113.115)), module, TAudio::AUDIO_OUTPUTS + 9));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.914, 113.115)), module, TAudio::AUDIO_OUTPUTS + 10));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.464, 113.115)), module, TAudio::AUDIO_OUTPUTS + 11));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.015, 113.115)), module, TAudio::AUDIO_OUTPUTS + 12));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(65.565, 113.115)), module, TAudio::AUDIO_OUTPUTS + 13));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(77.114, 113.115)), module, TAudio::AUDIO_OUTPUTS + 14));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(88.664, 113.115)), module, TAudio::AUDIO_OUTPUTS + 15));

			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(13.545, 52.168)), module, TAudio::INPUT_LIGHTS + 2 * 0));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(36.779, 52.168)), module, TAudio::INPUT_LIGHTS + 2 * 1));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(59.745, 52.168)), module, TAudio::INPUT_LIGHTS + 2 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(82.98, 52.168)), module, TAudio::INPUT_LIGHTS + 2 * 3));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(13.545, 68.53)), module, TAudio::INPUT_LIGHTS + 2 * 4));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(36.779, 68.53)), module, TAudio::INPUT_LIGHTS + 2 * 5));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(59.745, 68.53)), module, TAudio::INPUT_LIGHTS + 2 * 6));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(82.98, 68.53)), module, TAudio::INPUT_LIGHTS + 2 * 7));

			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(13.545, 90.791)), module, TAudio::OUTPUT_LIGHTS + 2 * 0));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(36.644, 90.791)), module, TAudio::OUTPUT_LIGHTS + 2 * 1));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(59.745, 90.791)), module, TAudio::OUTPUT_LIGHTS + 2 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(82.844, 90.791)), module, TAudio::OUTPUT_LIGHTS + 2 * 3));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(13.545, 107.097)), module, TAudio::OUTPUT_LIGHTS + 2 * 4));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(36.644, 107.097)), module, TAudio::OUTPUT_LIGHTS + 2 * 5));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(59.745, 107.097)), module, TAudio::OUTPUT_LIGHTS + 2 * 6));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(82.844, 107.097)), module, TAudio::OUTPUT_LIGHTS + 2 * 7));

			AudioDisplay* display = createWidget<AudioDisplay>(mm2px(Vec(0.0, 13.039)));
			display->box.size = mm2px(Vec(96.52, 29.021));
			display->setAudioPort(module ? &module->port : NULL);
			addChild(display);
		}
		else if (NUM_AUDIO_INPUTS == 2 && NUM_AUDIO_OUTPUTS == 2) {
			setPanel(Svg::load(asset::system("res/Core/Audio2.svg")));

			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

			addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(12.869, 77.362)), module, TAudio::LEVEL_PARAM));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.285, 96.859)), module, TAudio::AUDIO_INPUTS + 0));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.122, 96.859)), module, TAudio::AUDIO_INPUTS + 1));

			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.285, 113.115)), module, TAudio::AUDIO_OUTPUTS + 0));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.122, 113.115)), module, TAudio::AUDIO_OUTPUTS + 1));

			Audio2Display* display = createWidget<Audio2Display>(mm2px(Vec(0.0, 13.039)));
			display->box.size = mm2px(Vec(25.4, 47.726));
			display->setAudioPort(module ? &module->port : NULL);
			addChild(display);

			addChild(createLightCentered<SmallSimpleLight<RedLight>>(mm2px(Vec(6.691, 28.899)), module, TAudio::VU_LIGHTS + 6 * 0 + 0));
			addChild(createLightCentered<SmallSimpleLight<RedLight>>(mm2px(Vec(18.709, 28.899)), module, TAudio::VU_LIGHTS + 6 * 1 + 0));
			addChild(createLightCentered<SmallSimpleLight<YellowLight>>(mm2px(Vec(6.691, 34.196)), module, TAudio::VU_LIGHTS + 6 * 0 + 1));
			addChild(createLightCentered<SmallSimpleLight<YellowLight>>(mm2px(Vec(18.709, 34.196)), module, TAudio::VU_LIGHTS + 6 * 1 + 1));
			addChild(createLightCentered<SmallSimpleLight<GreenLight>>(mm2px(Vec(6.691, 39.494)), module, TAudio::VU_LIGHTS + 6 * 0 + 2));
			addChild(createLightCentered<SmallSimpleLight<GreenLight>>(mm2px(Vec(18.709, 39.494)), module, TAudio::VU_LIGHTS + 6 * 1 + 2));
			addChild(createLightCentered<SmallSimpleLight<GreenLight>>(mm2px(Vec(6.691, 44.791)), module, TAudio::VU_LIGHTS + 6 * 0 + 3));
			addChild(createLightCentered<SmallSimpleLight<GreenLight>>(mm2px(Vec(18.709, 44.791)), module, TAudio::VU_LIGHTS + 6 * 1 + 3));
			addChild(createLightCentered<SmallSimpleLight<GreenLight>>(mm2px(Vec(6.691, 50.089)), module, TAudio::VU_LIGHTS + 6 * 0 + 4));
			addChild(createLightCentered<SmallSimpleLight<GreenLight>>(mm2px(Vec(18.709, 50.089)), module, TAudio::VU_LIGHTS + 6 * 1 + 4));
			addChild(createLightCentered<SmallSimpleLight<GreenLight>>(mm2px(Vec(6.691, 55.386)), module, TAudio::VU_LIGHTS + 6 * 0 + 5));
			addChild(createLightCentered<SmallSimpleLight<GreenLight>>(mm2px(Vec(18.709, 55.386)), module, TAudio::VU_LIGHTS + 6 * 1 + 5));

			// AudioButton example
			// AudioButton* audioButton_ADAT = createWidget<AudioButton_ADAT>(Vec(0, 0));
			// audioButton_ADAT->setAudioPort(module ? &module->port : NULL);
			// addChild(audioButton_ADAT);

			// AudioButton* audioButton_USB_B = createWidget<AudioButton_USB_B>(Vec(0, 40));
			// audioButton_USB_B->setAudioPort(module ? &module->port : NULL);
			// addChild(audioButton_USB_B);
		}
	}

	void appendContextMenu(Menu* menu) override {
		TAudio* module = dynamic_cast<TAudio*>(this->module);

		menu->addChild(new MenuSeparator);

		menu->addChild(createBoolMenuItem("Master audio module", "",
			[=]() {return module->port.isMaster();},
			[=](bool master) {module->port.setMaster(master);}
		));

		menu->addChild(createBoolPtrMenuItem("DC blocker", "", &module->dcFilterEnabled));
	}
};


Model* modelAudio2 = createModel<Audio<2, 2>, AudioWidget<2, 2>>("AudioInterface2");
Model* modelAudio8 = createModel<Audio<8, 8>, AudioWidget<8, 8>>("AudioInterface");
Model* modelAudio16 = createModel<Audio<16, 16>, AudioWidget<16, 16>>("AudioInterface16");


} // namespace core
} // namespace rack
