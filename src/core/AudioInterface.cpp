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
struct AudioInterface : Module, audio::Port {
	static constexpr int NUM_INPUT_LIGHTS = (NUM_AUDIO_INPUTS > 2) ? (NUM_AUDIO_INPUTS / 2) : 0;
	static constexpr int NUM_OUTPUT_LIGHTS = (NUM_AUDIO_OUTPUTS > 2) ? (NUM_AUDIO_OUTPUTS / 2) : 0;

	enum ParamIds {
		ENUMS(GAIN_PARAM, NUM_AUDIO_INPUTS == 2),
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

	dsp::DoubleRingBuffer<dsp::Frame<NUM_AUDIO_INPUTS>, 32768> inputBuffer;
	dsp::DoubleRingBuffer<dsp::Frame<NUM_AUDIO_OUTPUTS>, 32768> outputBuffer;

	dsp::SampleRateConverter<NUM_AUDIO_INPUTS> inputSrc;
	dsp::SampleRateConverter<NUM_AUDIO_OUTPUTS> outputSrc;

	dsp::ClockDivider lightDivider;
	// For each pair of inputs/outputs
	float inputClipTimers[(NUM_AUDIO_INPUTS > 0) ? NUM_INPUT_LIGHTS : 0] = {};
	float outputClipTimers[(NUM_AUDIO_INPUTS > 0) ? NUM_OUTPUT_LIGHTS : 0] = {};
	dsp::VuMeter2 vuMeter[(NUM_AUDIO_INPUTS == 2) ? 2 : 0];

	// Port variables
	int requestedEngineFrames = 0;
	int maxEngineFrames = 0;

	AudioInterface() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		if (NUM_AUDIO_INPUTS == 2)
			configParam(GAIN_PARAM, 0.f, 2.f, 1.f, "Level", " dB", -10, 40);
		for (int i = 0; i < NUM_AUDIO_INPUTS; i++)
			configInput(AUDIO_INPUTS + i, string::f("To \"device output %d\"", i + 1));
		for (int i = 0; i < NUM_AUDIO_OUTPUTS; i++)
			configOutput(AUDIO_OUTPUTS + i, string::f("From \"device input %d\"", i + 1));
		for (int i = 0; i < NUM_INPUT_LIGHTS; i++)
			configLight(INPUT_LIGHTS + 2 * i, string::f("Device output %d/%d status", 2 * i + 1, 2 * i + 2));
		for (int i = 0; i < NUM_OUTPUT_LIGHTS; i++)
			configLight(OUTPUT_LIGHTS + 2 * i, string::f("Device input %d/%d status", 2 * i + 1, 2 * i + 2));

		lightDivider.setDivision(512);
		maxChannels = std::max(NUM_AUDIO_INPUTS, NUM_AUDIO_OUTPUTS);
		inputSrc.setQuality(6);
		outputSrc.setQuality(6);
	}

	~AudioInterface() {
		// Close stream here before destructing AudioInterfacePort, so processBuffer() etc are not called on another thread while destructing.
		setDriverId(-1);
	}

	void onReset() override {
		setDriverId(-1);
	}

	void onSampleRateChange(const SampleRateChangeEvent& e) override {
		inputBuffer.clear();
		outputBuffer.clear();
	}

	void process(const ProcessArgs& args) override {
		const float clipTime = 0.25f;

		// Push inputs to buffer
		dsp::Frame<NUM_AUDIO_INPUTS> inputFrame;
		for (int i = 0; i < NUM_AUDIO_INPUTS; i++) {
			// Get input
			float v = 0.f;
			if (inputs[AUDIO_INPUTS + i].isConnected())
				v = inputs[AUDIO_INPUTS + i].getVoltageSum() / 10.f;
			// Normalize right input to left on Audio-2
			else if (i > 0 && NUM_AUDIO_INPUTS == 2)
				v = inputFrame.samples[i - 1];

			// Detect clipping
			if (NUM_AUDIO_INPUTS > 2) {
				if (std::fabs(v) >= 1.f)
					inputClipTimers[i / 2] = clipTime;
			}
			inputFrame.samples[i] = v;
		}

		// Apply gain from knob
		if (NUM_AUDIO_INPUTS == 2) {
			float gain = std::pow(params[GAIN_PARAM].getValue(), 2.f);
			for (int i = 0; i < NUM_AUDIO_INPUTS; i++) {
				inputFrame.samples[i] *= gain;
			}
		}

		if (!inputBuffer.full()) {
			inputBuffer.push(inputFrame);
		}

		// Pull outputs from buffer
		if (!outputBuffer.empty()) {
			dsp::Frame<NUM_AUDIO_OUTPUTS> outputFrame = outputBuffer.shift();
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
			for (int i = 0; i < NUM_AUDIO_OUTPUTS; i++) {
				outputs[AUDIO_OUTPUTS + i].setVoltage(0.f);
			}
		}

		// Lights
		if (NUM_AUDIO_INPUTS == 2) {
			for (int i = 0; i < 2; i++) {
				vuMeter[i].process(args.sampleTime, inputFrame.samples[i]);
			}
		}
		if (lightDivider.process()) {
			float lightTime = args.sampleTime * lightDivider.getDivision();
			if (NUM_AUDIO_INPUTS == 2) {
				for (int i = 0; i < 2; i++) {
					lights[VU_LIGHTS + i * 6 + 0].setBrightness(vuMeter[i].getBrightness(0, 0));
					lights[VU_LIGHTS + i * 6 + 1].setBrightness(vuMeter[i].getBrightness(-3, 0));
					lights[VU_LIGHTS + i * 6 + 2].setBrightness(vuMeter[i].getBrightness(-6, -3));
					lights[VU_LIGHTS + i * 6 + 3].setBrightness(vuMeter[i].getBrightness(-12, -6));
					lights[VU_LIGHTS + i * 6 + 4].setBrightness(vuMeter[i].getBrightness(-24, -12));
					lights[VU_LIGHTS + i * 6 + 5].setBrightness(vuMeter[i].getBrightness(-36, -24));
				}
			}
			else {
				int numDeviceInputs = getNumInputs();
				int numDeviceOutputs = getNumOutputs();
				// Turn on light if at least one port is enabled in the nearby pair.
				for (int i = 0; i < NUM_AUDIO_INPUTS / 2; i++) {
					bool active = numDeviceOutputs >= 2 * i + 1;
					bool clip = inputClipTimers[i] > 0.f;
					if (clip)
						inputClipTimers[i] -= lightTime;
					lights[INPUT_LIGHTS + i * 2 + 0].setBrightness(active && !clip);
					lights[INPUT_LIGHTS + i * 2 + 1].setBrightness(active && clip);
				}
				for (int i = 0; i < NUM_AUDIO_OUTPUTS / 2; i++) {
					bool active = numDeviceInputs >= 2 * i + 1;
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
		json_object_set_new(rootJ, "audio", audio::Port::toJson());
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* audioJ = json_object_get(rootJ, "audio");
		if (audioJ)
			audio::Port::fromJson(audioJ);
	}

	// audio::Port

	void processInput(const float* input, int inputStride, int frames) override {
		// Claim primary module if there is none
		if (!APP->engine->getPrimaryModule()) {
			APP->engine->setPrimaryModule(this);
		}
		bool isPrimary = (APP->engine->getPrimaryModule() == this);

		// Set sample rate of engine if engine sample rate is "auto".
		float sampleRate = getSampleRate();
		if (isPrimary) {
			APP->engine->setSuggestedSampleRate(sampleRate);
		}

		// Initialize sample rate converters
		int numInputs = getNumInputs();
		int engineSampleRate = (int) APP->engine->getSampleRate();
		double sampleRateRatio = (double) engineSampleRate / sampleRate;
		outputSrc.setRates(sampleRate, engineSampleRate);
		outputSrc.setChannels(numInputs);

		// Consider engine buffers "too full" if they contain a bit more than the audio device's number of frames, converted to engine sample rate.
		maxEngineFrames = (int) std::ceil(frames * sampleRateRatio * 1.5);
		// If this is a secondary audio module and the engine output buffer is too full, flush it.
		if (!isPrimary && (int) outputBuffer.size() > maxEngineFrames) {
			outputBuffer.clear();
			// DEBUG("%p: flushing engine output", this);
		}

		if (numInputs > 0) {
			// audio input -> engine output
			dsp::Frame<NUM_AUDIO_OUTPUTS> inputAudioBuffer[frames];
			std::memset(inputAudioBuffer, 0, sizeof(inputAudioBuffer));
			for (int i = 0; i < frames; i++) {
				for (int j = 0; j < std::min(numInputs, NUM_AUDIO_OUTPUTS); j++) {
					float v = input[i * inputStride + j];
					inputAudioBuffer[i].samples[j] = v;
				}
			}
			int inputAudioFrames = frames;
			int outputFrames = outputBuffer.capacity();
			outputSrc.process(inputAudioBuffer, &inputAudioFrames, outputBuffer.endData(), &outputFrames);
			outputBuffer.endIncr(outputFrames);
			// Request exactly as many frames as we have.
			requestedEngineFrames = outputBuffer.size();
		}
		else {
			// Upper bound on number of frames so that `outputAudioFrames >= frames` at the end of this method.
			requestedEngineFrames = (int) std::ceil(frames * sampleRateRatio) - inputBuffer.size();
		}
	}

	void processBuffer(const float* input, int inputStride, float* output, int outputStride, int frames) override {
		bool isPrimary = (APP->engine->getPrimaryModule() == this);
		// Step engine
		if (isPrimary && requestedEngineFrames > 0) {
			APP->engine->stepBlock(requestedEngineFrames);
		}
	}

	void processOutput(float* output, int outputStride, int frames) override {
		bool isPrimary = (APP->engine->getPrimaryModule() == this);
		int numOutputs = getNumOutputs();
		int engineSampleRate = (int) APP->engine->getSampleRate();
		float sampleRate = getSampleRate();
		inputSrc.setRates(engineSampleRate, sampleRate);
		inputSrc.setChannels(numOutputs);

		if (numOutputs > 0) {
			// engine input -> audio output
			dsp::Frame<NUM_AUDIO_OUTPUTS> outputAudioBuffer[frames];
			int inputFrames = inputBuffer.size();
			int outputAudioFrames = frames;
			inputSrc.process(inputBuffer.startData(), &inputFrames, outputAudioBuffer, &outputAudioFrames);
			inputBuffer.startIncr(inputFrames);
			for (int i = 0; i < outputAudioFrames; i++) {
				for (int j = 0; j < std::min(numOutputs, NUM_AUDIO_INPUTS); j++) {
					float v = outputAudioBuffer[i].samples[j];
					v = clamp(v, -1.f, 1.f);
					output[i * outputStride + j] = v;
				}
			}
		}

		// If this is a secondary audio module and the engine input buffer is too full, flush it.
		if (!isPrimary && (int) inputBuffer.size() > maxEngineFrames) {
			inputBuffer.clear();
			// DEBUG("%p: flushing engine input", this);
		}

		// DEBUG("%p %s:\tframes %d requestedEngineFrames %d\toutputBuffer %d inputBuffer %d\t", this, isPrimary ? "primary" : "secondary", frames, requestedEngineFrames, outputBuffer.size(), inputBuffer.size());
	}

	void onOpenStream() override {
		inputBuffer.clear();
		outputBuffer.clear();
	}

	void onCloseStream() override {
		inputBuffer.clear();
		outputBuffer.clear();
	}
};


template <typename TAudioInterface>
struct PrimaryModuleItem : MenuItem {
	TAudioInterface* module;

	void onAction(const event::Action& e) override {
		APP->engine->setPrimaryModule(module);
	}
};


template <int NUM_AUDIO_INPUTS, int NUM_AUDIO_OUTPUTS>
struct AudioInterfaceWidget : ModuleWidget {
	typedef AudioInterface<NUM_AUDIO_INPUTS, NUM_AUDIO_OUTPUTS> TAudioInterface;

	AudioInterfaceWidget(TAudioInterface* module) {
		setModule(module);

		if (NUM_AUDIO_INPUTS == 8 && NUM_AUDIO_OUTPUTS == 8) {
			setPanel(APP->window->loadSvg(asset::system("res/Core/AudioInterface.svg")));

			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

			addInput(createInput<PJ301MPort>(mm2px(Vec(3.7069211, 55.530807)), module, TAudioInterface::AUDIO_INPUTS + 0));
			addInput(createInput<PJ301MPort>(mm2px(Vec(15.307249, 55.530807)), module, TAudioInterface::AUDIO_INPUTS + 1));
			addInput(createInput<PJ301MPort>(mm2px(Vec(26.906193, 55.530807)), module, TAudioInterface::AUDIO_INPUTS + 2));
			addInput(createInput<PJ301MPort>(mm2px(Vec(38.506519, 55.530807)), module, TAudioInterface::AUDIO_INPUTS + 3));
			addInput(createInput<PJ301MPort>(mm2px(Vec(3.7069209, 70.144905)), module, TAudioInterface::AUDIO_INPUTS + 4));
			addInput(createInput<PJ301MPort>(mm2px(Vec(15.307249, 70.144905)), module, TAudioInterface::AUDIO_INPUTS + 5));
			addInput(createInput<PJ301MPort>(mm2px(Vec(26.906193, 70.144905)), module, TAudioInterface::AUDIO_INPUTS + 6));
			addInput(createInput<PJ301MPort>(mm2px(Vec(38.506519, 70.144905)), module, TAudioInterface::AUDIO_INPUTS + 7));

			addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.7069209, 92.143906)), module, TAudioInterface::AUDIO_OUTPUTS + 0));
			addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.307249, 92.143906)), module, TAudioInterface::AUDIO_OUTPUTS + 1));
			addOutput(createOutput<PJ301MPort>(mm2px(Vec(26.906193, 92.143906)), module, TAudioInterface::AUDIO_OUTPUTS + 2));
			addOutput(createOutput<PJ301MPort>(mm2px(Vec(38.506519, 92.143906)), module, TAudioInterface::AUDIO_OUTPUTS + 3));
			addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.7069209, 108.1443)), module, TAudioInterface::AUDIO_OUTPUTS + 4));
			addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.307249, 108.1443)), module, TAudioInterface::AUDIO_OUTPUTS + 5));
			addOutput(createOutput<PJ301MPort>(mm2px(Vec(26.906193, 108.1443)), module, TAudioInterface::AUDIO_OUTPUTS + 6));
			addOutput(createOutput<PJ301MPort>(mm2px(Vec(38.506523, 108.1443)), module, TAudioInterface::AUDIO_OUTPUTS + 7));

			addChild(createLight<SmallLight<GreenRedLight>>(mm2px(Vec(12.524985, 54.577202)), module, TAudioInterface::INPUT_LIGHTS + 0 * 2));
			addChild(createLight<SmallLight<GreenRedLight>>(mm2px(Vec(35.725647, 54.577202)), module, TAudioInterface::INPUT_LIGHTS + 1 * 2));
			addChild(createLight<SmallLight<GreenRedLight>>(mm2px(Vec(12.524985, 69.158226)), module, TAudioInterface::INPUT_LIGHTS + 2 * 2));
			addChild(createLight<SmallLight<GreenRedLight>>(mm2px(Vec(35.725647, 69.158226)), module, TAudioInterface::INPUT_LIGHTS + 3 * 2));
			addChild(createLight<SmallLight<GreenRedLight>>(mm2px(Vec(12.524985, 91.147583)), module, TAudioInterface::OUTPUT_LIGHTS + 0 * 2));
			addChild(createLight<SmallLight<GreenRedLight>>(mm2px(Vec(35.725647, 91.147583)), module, TAudioInterface::OUTPUT_LIGHTS + 1 * 2));
			addChild(createLight<SmallLight<GreenRedLight>>(mm2px(Vec(12.524985, 107.17003)), module, TAudioInterface::OUTPUT_LIGHTS + 2 * 2));
			addChild(createLight<SmallLight<GreenRedLight>>(mm2px(Vec(35.725647, 107.17003)), module, TAudioInterface::OUTPUT_LIGHTS + 3 * 2));

			AudioWidget* audioWidget = createWidget<AudioWidget>(mm2px(Vec(3.2122073, 14.837339)));
			audioWidget->box.size = mm2px(Vec(44, 28));
			audioWidget->setAudioPort(module);
			addChild(audioWidget);
		}
		else if (NUM_AUDIO_INPUTS == 16 && NUM_AUDIO_OUTPUTS == 16) {
			setPanel(APP->window->loadSvg(asset::system("res/Core/AudioInterface16.svg")));

			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.661, 59.638)), module, TAudioInterface::AUDIO_INPUTS + 0));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.26, 59.638)), module, TAudioInterface::AUDIO_INPUTS + 1));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.86, 59.638)), module, TAudioInterface::AUDIO_INPUTS + 2));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.461, 59.638)), module, TAudioInterface::AUDIO_INPUTS + 3));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.06, 59.638)), module, TAudioInterface::AUDIO_INPUTS + 4));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(65.661, 59.638)), module, TAudioInterface::AUDIO_INPUTS + 5));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(77.26, 59.638)), module, TAudioInterface::AUDIO_INPUTS + 6));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(88.86, 59.638)), module, TAudioInterface::AUDIO_INPUTS + 7));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.661, 74.251)), module, TAudioInterface::AUDIO_INPUTS + 8));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.26, 74.251)), module, TAudioInterface::AUDIO_INPUTS + 9));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.86, 74.251)), module, TAudioInterface::AUDIO_INPUTS + 10));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.461, 74.251)), module, TAudioInterface::AUDIO_INPUTS + 11));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.06, 74.251)), module, TAudioInterface::AUDIO_INPUTS + 12));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(65.661, 74.251)), module, TAudioInterface::AUDIO_INPUTS + 13));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(77.26, 74.251)), module, TAudioInterface::AUDIO_INPUTS + 14));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(88.86, 74.251)), module, TAudioInterface::AUDIO_INPUTS + 15));

			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.661, 96.251)), module, TAudioInterface::AUDIO_OUTPUTS + 0));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19.26, 96.251)), module, TAudioInterface::AUDIO_OUTPUTS + 1));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.86, 96.251)), module, TAudioInterface::AUDIO_OUTPUTS + 2));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.461, 96.251)), module, TAudioInterface::AUDIO_OUTPUTS + 3));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.06, 96.251)), module, TAudioInterface::AUDIO_OUTPUTS + 4));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(65.661, 96.251)), module, TAudioInterface::AUDIO_OUTPUTS + 5));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(77.26, 96.251)), module, TAudioInterface::AUDIO_OUTPUTS + 6));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(88.86, 96.251)), module, TAudioInterface::AUDIO_OUTPUTS + 7));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.661, 112.252)), module, TAudioInterface::AUDIO_OUTPUTS + 8));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19.26, 112.252)), module, TAudioInterface::AUDIO_OUTPUTS + 9));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.86, 112.252)), module, TAudioInterface::AUDIO_OUTPUTS + 10));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.461, 112.252)), module, TAudioInterface::AUDIO_OUTPUTS + 11));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.06, 112.252)), module, TAudioInterface::AUDIO_OUTPUTS + 12));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(65.661, 112.252)), module, TAudioInterface::AUDIO_OUTPUTS + 13));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(77.26, 112.252)), module, TAudioInterface::AUDIO_OUTPUTS + 14));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(88.86, 112.252)), module, TAudioInterface::AUDIO_OUTPUTS + 15));

			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(13.46, 55.667)), module, TAudioInterface::INPUT_LIGHTS + 0 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(36.661, 55.667)), module, TAudioInterface::INPUT_LIGHTS + 1 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(59.861, 55.667)), module, TAudioInterface::INPUT_LIGHTS + 2 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(83.061, 55.667)), module, TAudioInterface::INPUT_LIGHTS + 3 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(13.46, 70.248)), module, TAudioInterface::INPUT_LIGHTS + 4 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(36.661, 70.248)), module, TAudioInterface::INPUT_LIGHTS + 5 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(59.861, 70.248)), module, TAudioInterface::INPUT_LIGHTS + 6 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(83.061, 70.248)), module, TAudioInterface::INPUT_LIGHTS + 7 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(13.46, 92.238)), module, TAudioInterface::OUTPUT_LIGHTS + 0 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(36.661, 92.238)), module, TAudioInterface::OUTPUT_LIGHTS + 1 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(59.861, 92.238)), module, TAudioInterface::OUTPUT_LIGHTS + 2 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(83.061, 92.238)), module, TAudioInterface::OUTPUT_LIGHTS + 3 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(13.46, 108.259)), module, TAudioInterface::OUTPUT_LIGHTS + 4 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(36.661, 108.259)), module, TAudioInterface::OUTPUT_LIGHTS + 5 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(59.861, 108.259)), module, TAudioInterface::OUTPUT_LIGHTS + 6 * 2));
			addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(83.061, 108.259)), module, TAudioInterface::OUTPUT_LIGHTS + 7 * 2));

			AudioWidget* audioWidget = createWidget<AudioWidget>(mm2px(Vec(2.57, 14.839)));
			audioWidget->box.size = mm2px(Vec(91.382, 28.0));
			audioWidget->setAudioPort(module);
			addChild(audioWidget);
		}
		else if (NUM_AUDIO_INPUTS == 2 && NUM_AUDIO_OUTPUTS == 2) {
			setPanel(APP->window->loadSvg(asset::system("res/Core/AudioInterface2.svg")));

			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

			addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(12.7, 74.019)), module, TAudioInterface::GAIN_PARAM));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.697, 94.253)), module, TAudioInterface::AUDIO_INPUTS + 0));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.703, 94.253)), module, TAudioInterface::AUDIO_INPUTS + 1));

			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.699, 112.254)), module, TAudioInterface::AUDIO_OUTPUTS + 0));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.7, 112.254)), module, TAudioInterface::AUDIO_OUTPUTS + 1));

			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(6.7, 29.759)), module, TAudioInterface::VU_LIGHTS + 0 * 6 + 0));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(18.7, 29.759)), module, TAudioInterface::VU_LIGHTS + 1 * 6 + 0));
			addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(6.7, 34.753)), module, TAudioInterface::VU_LIGHTS + 0 * 6 + 1));
			addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(18.7, 34.753)), module, TAudioInterface::VU_LIGHTS + 1 * 6 + 1));
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(6.7, 39.749)), module, TAudioInterface::VU_LIGHTS + 0 * 6 + 2));
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(18.7, 39.749)), module, TAudioInterface::VU_LIGHTS + 1 * 6 + 2));
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(6.7, 44.744)), module, TAudioInterface::VU_LIGHTS + 0 * 6 + 3));
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(18.7, 44.744)), module, TAudioInterface::VU_LIGHTS + 1 * 6 + 3));
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(6.7, 49.744)), module, TAudioInterface::VU_LIGHTS + 0 * 6 + 4));
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(18.7, 49.744)), module, TAudioInterface::VU_LIGHTS + 1 * 6 + 4));
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(6.7, 54.745)), module, TAudioInterface::VU_LIGHTS + 0 * 6 + 5));
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(18.7, 54.745)), module, TAudioInterface::VU_LIGHTS + 1 * 6 + 5));

			AudioDeviceWidget* audioWidget = createWidget<AudioDeviceWidget>(mm2px(Vec(2.135, 14.259)));
			audioWidget->box.size = mm2px(Vec(21.128, 6.725));
			audioWidget->setAudioPort(module);
			// Adjust deviceChoice position
			audioWidget->deviceChoice->textOffset = Vec(6, 14);
			addChild(audioWidget);
		}
	}

	void appendContextMenu(Menu* menu) override {
		TAudioInterface* module = dynamic_cast<TAudioInterface*>(this->module);

		menu->addChild(new MenuSeparator);

		PrimaryModuleItem<TAudioInterface>* primaryModuleItem = new PrimaryModuleItem<TAudioInterface>;
		primaryModuleItem->text = "Primary audio module";
		primaryModuleItem->rightText = CHECKMARK(APP->engine->getPrimaryModule() == module);
		primaryModuleItem->module = module;
		menu->addChild(primaryModuleItem);
	}
};


Model* modelAudioInterface2 = createModel<AudioInterface<2, 2>, AudioInterfaceWidget<2, 2>>("AudioInterface2");
// Legacy name for Audio-8
Model* modelAudioInterface = createModel<AudioInterface<8, 8>, AudioInterfaceWidget<8, 8>>("AudioInterface");
Model* modelAudioInterface16 = createModel<AudioInterface<16, 16>, AudioInterfaceWidget<16, 16>>("AudioInterface16");


} // namespace core
} // namespace rack
