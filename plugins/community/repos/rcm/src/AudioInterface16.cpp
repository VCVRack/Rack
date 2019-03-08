#include <assert.h>
#include <mutex>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "GVerbWidget.hpp"
#include "audio.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"


#define AUDIO_OUTPUTS 16
#define AUDIO_INPUTS 16


using namespace rack;

// Sound by Yaroslav Samoylov from the Noun Project

struct AudioInterfaceIO16 : AudioIO {
	std::mutex engineMutex;
	std::condition_variable engineCv;
	std::mutex audioMutex;
	std::condition_variable audioCv;
	// Audio thread produces, engine thread consumes
	DoubleRingBuffer<Frame<AUDIO_INPUTS>, (1<<15)> inputBuffer;
	// Audio thread consumes, engine thread produces
	DoubleRingBuffer<Frame<AUDIO_OUTPUTS>, (1<<15)> outputBuffer;
	bool active = false;

	AudioInterfaceIO16() {
		maxChannels = 16;
	}

	~AudioInterfaceIO16() {
		// Close stream here before destructing AudioInterfaceIO16, so the mutexes are still valid when waiting to close.
		setDevice(-1, 0);
	}

	void processStream(const float *input, float *output, int frames) override {
		// Reactivate idle stream
		if (!active) {
			active = true;
			inputBuffer.clear();
			outputBuffer.clear();
		}

		if (numInputs > 0) {
			// TODO Do we need to wait on the input to be consumed here? Experimentally, it works fine if we don't.
			for (int i = 0; i < frames; i++) {
				if (inputBuffer.full())
					break;
				Frame<AUDIO_INPUTS> inputFrame;
				memset(&inputFrame, 0, sizeof(inputFrame));
				memcpy(&inputFrame, &input[numInputs * i], numInputs * sizeof(float));
				inputBuffer.push(inputFrame);
			}
		}

		if (numOutputs > 0) {
			std::unique_lock<std::mutex> lock(audioMutex);
			auto cond = [&] {
				return (outputBuffer.size() >= (size_t) frames);
			};
			auto timeout = std::chrono::milliseconds(100);
			if (audioCv.wait_for(lock, timeout, cond)) {
				// Consume audio block
				for (int i = 0; i < frames; i++) {
					Frame<AUDIO_OUTPUTS> f = outputBuffer.shift();
					for (int j = 0; j < numOutputs; j++) {
						output[numOutputs*i + j] = clamp(f.samples[j], -1.f, 1.f);
					}
				}
			}
			else {
				// Timed out, fill output with zeros
				memset(output, 0, frames * numOutputs * sizeof(float));
				debug("Audio Interface IO underflow");
			}
		}

		// Notify engine when finished processing
		engineCv.notify_one();
	}

	void onCloseStream() override {
		inputBuffer.clear();
		outputBuffer.clear();
	}

	void onChannelsChange() override {
	}
};


struct AudioInterface16 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(AUDIO_INPUT, AUDIO_INPUTS),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(AUDIO_OUTPUT, AUDIO_OUTPUTS),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(INPUT_LIGHT, AUDIO_INPUTS / 2),
		ENUMS(OUTPUT_LIGHT, AUDIO_OUTPUTS / 2),
		NUM_LIGHTS
	};

	AudioInterfaceIO16 audioIO;
	int lastSampleRate = 0;
	int lastNumOutputs = -1;
	int lastNumInputs = -1;

	SampleRateConverter<AUDIO_INPUTS> inputSrc;
	SampleRateConverter<AUDIO_OUTPUTS> outputSrc;

	// in rack's sample rate
	DoubleRingBuffer<Frame<AUDIO_INPUTS>, 16> inputBuffer;
	DoubleRingBuffer<Frame<AUDIO_OUTPUTS>, 16> outputBuffer;

	AudioInterface16() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onSampleRateChange();
	}

	void step() override;

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "audio", audioIO.toJson());
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *audioJ = json_object_get(rootJ, "audio");
		audioIO.fromJson(audioJ);
	}

	void onReset() override {
		audioIO.setDevice(-1, 0);
	}
};


void AudioInterface16::step() {
	// Update SRC states
	int sampleRate = (int) engineGetSampleRate();
	inputSrc.setRates(audioIO.sampleRate, sampleRate);
	outputSrc.setRates(sampleRate, audioIO.sampleRate);

	inputSrc.setChannels(audioIO.numInputs);
	outputSrc.setChannels(audioIO.numOutputs);

	// Inputs: audio engine -> rack engine
	if (audioIO.active && audioIO.numInputs > 0) {
		// Wait until inputs are present
		// Give up after a timeout in case the audio device is being unresponsive.
		std::unique_lock<std::mutex> lock(audioIO.engineMutex);
		auto cond = [&] {
			return (!audioIO.inputBuffer.empty());
		};
		auto timeout = std::chrono::milliseconds(200);
		if (audioIO.engineCv.wait_for(lock, timeout, cond)) {
			// Convert inputs
			int inLen = audioIO.inputBuffer.size();
			int outLen = inputBuffer.capacity();
			inputSrc.process(audioIO.inputBuffer.startData(), &inLen, inputBuffer.endData(), &outLen);
			audioIO.inputBuffer.startIncr(inLen);
			inputBuffer.endIncr(outLen);
		}
		else {
			// Give up on pulling input
			audioIO.active = false;
			debug("Audio Interface underflow");
		}
	}

	// Take input from buffer
	Frame<AUDIO_INPUTS> inputFrame;
	if (!inputBuffer.empty()) {
		inputFrame = inputBuffer.shift();
	}
	else {
		memset(&inputFrame, 0, sizeof(inputFrame));
	}
	for (int i = 0; i < audioIO.numInputs; i++) {
		outputs[AUDIO_OUTPUT + i].value = 10.f * inputFrame.samples[i];
	}
	for (int i = audioIO.numInputs; i < AUDIO_INPUTS; i++) {
		outputs[AUDIO_OUTPUT + i].value = 0.f;
	}

	// Outputs: rack engine -> audio engine
	if (audioIO.active && audioIO.numOutputs > 0) {
		// Get and push output SRC frame
		if (!outputBuffer.full()) {
			Frame<AUDIO_OUTPUTS> outputFrame;
			for (int i = 0; i < AUDIO_OUTPUTS; i++) {
				outputFrame.samples[i] = inputs[AUDIO_INPUT + i].value / 10.f;
			}
			outputBuffer.push(outputFrame);
		}

		if (outputBuffer.full()) {
			// Wait until enough outputs are consumed
			// Give up after a timeout in case the audio device is being unresponsive.
			std::unique_lock<std::mutex> lock(audioIO.engineMutex);
			auto cond = [&] {
				return (audioIO.outputBuffer.size() < (size_t) audioIO.blockSize);
			};
			auto timeout = std::chrono::milliseconds(200);
			if (audioIO.engineCv.wait_for(lock, timeout, cond)) {
				// Push converted output
				int inLen = outputBuffer.size();
				int outLen = audioIO.outputBuffer.capacity();
				outputSrc.process(outputBuffer.startData(), &inLen, audioIO.outputBuffer.endData(), &outLen);
				outputBuffer.startIncr(inLen);
				audioIO.outputBuffer.endIncr(outLen);
			}
			else {
				// Give up on pushing output
				audioIO.active = false;
				outputBuffer.clear();
				debug("Audio Interface underflow");
			}
		}

		// Notify audio thread that an output is potentially ready
		audioIO.audioCv.notify_one();
	}

	// Turn on light if at least one port is enabled in the nearby pair
	for (int i = 0; i < AUDIO_INPUTS / 2; i++)
		lights[INPUT_LIGHT + i].value = (audioIO.active && audioIO.numOutputs >= 2*i+1);
	for (int i = 0; i < AUDIO_OUTPUTS / 2; i++)
		lights[OUTPUT_LIGHT + i].value = (audioIO.active && audioIO.numInputs >= 2*i+1);
}


struct AudioInterfaceWidget16 : ModuleWidget {
	SVGWidget* backgroundPanel;
	std::shared_ptr<SVG> background_SB;
	std::shared_ptr<SVG> background_Plastic;

	AudioInterfaceWidget16(AudioInterface16 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/AudioInterface16_Plastic.svg")));

		addInput(Port::create<PJ301MPort>(mm2px(Vec(3.7069211, 55.530807)), Port::INPUT, module, AudioInterface16::AUDIO_INPUT + 0));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(15.307249, 55.530807)), Port::INPUT, module, AudioInterface16::AUDIO_INPUT + 1));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(26.906193, 55.530807)), Port::INPUT, module, AudioInterface16::AUDIO_INPUT + 2));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(38.506519, 55.530807)), Port::INPUT, module, AudioInterface16::AUDIO_INPUT + 3));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(50.106845, 55.530807)), Port::INPUT, module, AudioInterface16::AUDIO_INPUT + 4));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(61.707171, 55.530807)), Port::INPUT, module, AudioInterface16::AUDIO_INPUT + 5));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(73.307497, 55.530807)), Port::INPUT, module, AudioInterface16::AUDIO_INPUT + 6));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(84.907823, 55.530807)), Port::INPUT, module, AudioInterface16::AUDIO_INPUT + 7));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(3.7069211, 70.144905)), Port::INPUT, module, AudioInterface16::AUDIO_INPUT + 8));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(15.307249, 70.144905)), Port::INPUT, module, AudioInterface16::AUDIO_INPUT + 9));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(26.906193, 70.144905)), Port::INPUT, module, AudioInterface16::AUDIO_INPUT + 10));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(38.506519, 70.144905)), Port::INPUT, module, AudioInterface16::AUDIO_INPUT + 11));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(50.106845, 70.144905)), Port::INPUT, module, AudioInterface16::AUDIO_INPUT + 12));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(61.707171, 70.144905)), Port::INPUT, module, AudioInterface16::AUDIO_INPUT + 13));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(73.307497, 70.144905)), Port::INPUT, module, AudioInterface16::AUDIO_INPUT + 14));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(84.907823, 70.144905)), Port::INPUT, module, AudioInterface16::AUDIO_INPUT + 15));

		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.7069209, 92.143906)), Port::OUTPUT, module, AudioInterface16::AUDIO_OUTPUT + 0));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.307249, 92.143906)), Port::OUTPUT, module, AudioInterface16::AUDIO_OUTPUT + 1));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(26.906193, 92.143906)), Port::OUTPUT, module, AudioInterface16::AUDIO_OUTPUT + 2));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.506519, 92.143906)), Port::OUTPUT, module, AudioInterface16::AUDIO_OUTPUT + 3));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(50.106845, 92.143906)), Port::OUTPUT, module, AudioInterface16::AUDIO_OUTPUT + 4));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(61.707171, 92.143906)), Port::OUTPUT, module, AudioInterface16::AUDIO_OUTPUT + 5));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(73.307497, 92.143906)), Port::OUTPUT, module, AudioInterface16::AUDIO_OUTPUT + 6));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(84.907823, 92.143906)), Port::OUTPUT, module, AudioInterface16::AUDIO_OUTPUT + 7));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.7069209, 108.1443)), Port::OUTPUT, module, AudioInterface16::AUDIO_OUTPUT + 8));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.307249, 108.1443)), Port::OUTPUT, module, AudioInterface16::AUDIO_OUTPUT + 9));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(26.906193, 108.1443)), Port::OUTPUT, module, AudioInterface16::AUDIO_OUTPUT + 10));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.506519, 108.1443)), Port::OUTPUT, module, AudioInterface16::AUDIO_OUTPUT + 11));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(50.106845, 108.1443)), Port::OUTPUT, module, AudioInterface16::AUDIO_OUTPUT + 12));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(61.707171, 108.1443)), Port::OUTPUT, module, AudioInterface16::AUDIO_OUTPUT + 13));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(73.307497, 108.1443)), Port::OUTPUT, module, AudioInterface16::AUDIO_OUTPUT + 14));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(84.907823, 108.1443)), Port::OUTPUT, module, AudioInterface16::AUDIO_OUTPUT + 15));

		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(12.524985, 54.577202)), module, AudioInterface16::INPUT_LIGHT + 0));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(35.725647, 54.577202)), module, AudioInterface16::INPUT_LIGHT + 1));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(58.926309, 54.577202)), module, AudioInterface16::INPUT_LIGHT + 2));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(82.126971, 54.577202)), module, AudioInterface16::INPUT_LIGHT + 3));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(12.524985, 69.158226)), module, AudioInterface16::INPUT_LIGHT + 4));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(35.725647, 69.158226)), module, AudioInterface16::INPUT_LIGHT + 5));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(58.926309, 69.158226)), module, AudioInterface16::INPUT_LIGHT + 6));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(82.126971, 69.158226)), module, AudioInterface16::INPUT_LIGHT + 7));

		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(12.524985, 91.147583)), module, AudioInterface16::OUTPUT_LIGHT + 0));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(35.725647, 91.147583)), module, AudioInterface16::OUTPUT_LIGHT + 1));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(58.926309, 91.147583)), module, AudioInterface16::OUTPUT_LIGHT + 2));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(82.126971, 91.147583)), module, AudioInterface16::OUTPUT_LIGHT + 3));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(12.524985, 107.17003)), module, AudioInterface16::OUTPUT_LIGHT + 4));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(35.725647, 107.17003)), module, AudioInterface16::OUTPUT_LIGHT + 5));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(58.926309, 107.17003)), module, AudioInterface16::OUTPUT_LIGHT + 6));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(82.126971, 107.17003)), module, AudioInterface16::OUTPUT_LIGHT + 7));

		AudioWidget* audioWidget = Widget::create<AudioWidget>(mm2px(Vec(3.2122073, 10.837339)));
		audioWidget->box.size = mm2px(Vec(90.5, 28));
		audioWidget->audioIO = &module->audioIO;
		addChild(audioWidget);
	}


};

Model *modelAudioInterface16 = Model::create<AudioInterface16, AudioInterfaceWidget16>("rcm", "AudioInterface16", "Audio Interface 16", EXTERNAL_TAG);
