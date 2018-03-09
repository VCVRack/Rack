#include <assert.h>
#include <mutex>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "Core.hpp"
#include "audio.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#include <RtAudio.h>
#pragma GCC diagnostic pop


#define OUTPUTS 8
#define INPUTS 8


using namespace rack;


struct AudioInterfaceIO : AudioIO {
	std::mutex engineMutex;
	std::condition_variable engineCv;
	std::mutex audioMutex;
	std::condition_variable audioCv;
	// Audio thread produces, engine thread consumes
	DoubleRingBuffer<Frame<INPUTS>, (1<<15)> inputBuffer;
	// Audio thread consumes, engine thread produces
	DoubleRingBuffer<Frame<OUTPUTS>, (1<<15)> outputBuffer;

	~AudioInterfaceIO() {
		// Close stream here before destructing AudioInterfaceIO, so the mutexes are still valid when waiting to close.
		setDevice(-1, 0);
	}

	void processStream(const float *input, float *output, int frames) override {
		if (numInputs > 0) {
			// TODO Do we need to wait on the input to be consumed here? Experimentally, it works fine if we don't.
			for (int i = 0; i < frames; i++) {
				if (inputBuffer.full())
					break;
				Frame<INPUTS> f;
				memset(&f, 0, sizeof(f));
				memcpy(&f, &input[numInputs * i], numInputs * sizeof(float));
				inputBuffer.push(f);
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
					Frame<OUTPUTS> f = outputBuffer.shift();
					memcpy(&output[numOutputs * i], &f, numOutputs * sizeof(float));
				}
			}
			else {
				// Timed out, fill output with zeros
				memset(output, 0, frames * numOutputs * sizeof(float));
				debug("Audio Interface IO underflow");
			}
		}

		// Notify engine when finished processing
		engineCv.notify_all();
	}

	void onCloseStream() override {
		inputBuffer.clear();
		outputBuffer.clear();
	}
};


struct AudioInterface : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(AUDIO_INPUT, INPUTS),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(AUDIO_OUTPUT, OUTPUTS),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(INPUT_LIGHT, INPUTS / 2),
		ENUMS(OUTPUT_LIGHT, OUTPUTS / 2),
		NUM_LIGHTS
	};

	AudioInterfaceIO audioIO;
	int lastSampleRate = 0;

	SampleRateConverter<INPUTS> inputSrc;
	SampleRateConverter<OUTPUTS> outputSrc;

	// in rack's sample rate
	DoubleRingBuffer<Frame<INPUTS>, 16> inputBuffer;
	DoubleRingBuffer<Frame<OUTPUTS>, 16> outputBuffer;

	AudioInterface() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
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

	void onSampleRateChange() override {
		// for (int i = 0; i < INPUTS; i++) {
		// 	inputSrc[i].setRates(audioIO.sampleRate, engineGetSampleRate());
		// }
		// for (int i = 0; i < OUTPUTS; i++) {
		// 	outputSrc[i].setRates(engineGetSampleRate(), audioIO.sampleRate);
		// }
		inputSrc.setRates(audioIO.sampleRate, engineGetSampleRate());
		outputSrc.setRates(engineGetSampleRate(), audioIO.sampleRate);
	}

	void onReset() override {
		audioIO.setDevice(-1, 0);
	}
};


void AudioInterface::step() {
	Frame<INPUTS> inputFrame;
	memset(&inputFrame, 0, sizeof(inputFrame));

	// Update sample rate if changed by audio driver
	if (audioIO.sampleRate != lastSampleRate) {
		onSampleRateChange();
		lastSampleRate = audioIO.sampleRate;
	}

	if (audioIO.numInputs > 0) {
		if (inputBuffer.empty()) {
			int inLen = audioIO.inputBuffer.size();
			int outLen = inputBuffer.capacity();
			inputSrc.process(audioIO.inputBuffer.startData(), &inLen, inputBuffer.endData(), &outLen);
			audioIO.inputBuffer.startIncr(inLen);
			inputBuffer.endIncr(outLen);
		}
	}

	if (!inputBuffer.empty()) {
		inputFrame = inputBuffer.shift();
	}
	for (int i = 0; i < INPUTS; i++) {
		outputs[AUDIO_OUTPUT + i].value = 10.0 * inputFrame.samples[i];
	}

	if (audioIO.numOutputs > 0) {
		// Get and push output SRC frame
		if (!outputBuffer.full()) {
			Frame<OUTPUTS> f;
			for (int i = 0; i < audioIO.numOutputs; i++) {
				f.samples[i] = inputs[AUDIO_INPUT + i].value / 10.0;
			}
			outputBuffer.push(f);
		}

		if (outputBuffer.full()) {
			// Wait until outputs are needed
			std::unique_lock<std::mutex> lock(audioIO.engineMutex);
			auto cond = [&] {
				return (audioIO.outputBuffer.size() < (size_t) audioIO.blockSize);
			};
			auto timeout = std::chrono::milliseconds(100);
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
				debug("Audio Interface underflow");
			}
		}
	}

	for (int i = 0; i < INPUTS / 2; i++)
		lights[INPUT_LIGHT + i].value = (audioIO.numOutputs >= 2*i+1);
	for (int i = 0; i < OUTPUTS / 2; i++)
		lights[OUTPUT_LIGHT + i].value = (audioIO.numInputs >= 2*i+1);

	audioIO.audioCv.notify_all();
}


struct AudioInterfaceWidget : ModuleWidget {
	AudioInterfaceWidget(AudioInterface *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetGlobal("res/Core/AudioInterface.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(Port::create<PJ301MPort>(mm2px(Vec(3.7069211, 55.530807)), Port::INPUT, module, AudioInterface::AUDIO_INPUT + 0));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(15.307249, 55.530807)), Port::INPUT, module, AudioInterface::AUDIO_INPUT + 1));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(26.906193, 55.530807)), Port::INPUT, module, AudioInterface::AUDIO_INPUT + 2));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(38.506519, 55.530807)), Port::INPUT, module, AudioInterface::AUDIO_INPUT + 3));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(3.7069209, 70.144905)), Port::INPUT, module, AudioInterface::AUDIO_INPUT + 4));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(15.307249, 70.144905)), Port::INPUT, module, AudioInterface::AUDIO_INPUT + 5));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(26.906193, 70.144905)), Port::INPUT, module, AudioInterface::AUDIO_INPUT + 6));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(38.506519, 70.144905)), Port::INPUT, module, AudioInterface::AUDIO_INPUT + 7));

		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.7069209, 92.143906)), Port::OUTPUT, module, AudioInterface::AUDIO_OUTPUT + 0));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.307249, 92.143906)), Port::OUTPUT, module, AudioInterface::AUDIO_OUTPUT + 1));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(26.906193, 92.143906)), Port::OUTPUT, module, AudioInterface::AUDIO_OUTPUT + 2));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.506519, 92.143906)), Port::OUTPUT, module, AudioInterface::AUDIO_OUTPUT + 3));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.7069209, 108.1443)), Port::OUTPUT, module, AudioInterface::AUDIO_OUTPUT + 4));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.307249, 108.1443)), Port::OUTPUT, module, AudioInterface::AUDIO_OUTPUT + 5));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(26.906193, 108.1443)), Port::OUTPUT, module, AudioInterface::AUDIO_OUTPUT + 6));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.506523, 108.1443)), Port::OUTPUT, module, AudioInterface::AUDIO_OUTPUT + 7));

		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(12.524985, 54.577202)), module, AudioInterface::INPUT_LIGHT + 0));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(35.725647, 54.577202)), module, AudioInterface::INPUT_LIGHT + 1));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(12.524985, 69.158226)), module, AudioInterface::INPUT_LIGHT + 2));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(35.725647, 69.158226)), module, AudioInterface::INPUT_LIGHT + 3));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(12.524985, 91.147583)), module, AudioInterface::OUTPUT_LIGHT + 0));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(35.725647, 91.147583)), module, AudioInterface::OUTPUT_LIGHT + 1));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(12.524985, 107.17003)), module, AudioInterface::OUTPUT_LIGHT + 2));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(35.725647, 107.17003)), module, AudioInterface::OUTPUT_LIGHT + 3));

		AudioWidget *audioWidget = Widget::create<AudioWidget>(mm2px(Vec(3.2122073, 14.837339)));
		audioWidget->box.size = mm2px(Vec(44, 28));
		audioWidget->audioIO = &module->audioIO;
		addChild(audioWidget);
	}
};


Model *modelAudioInterface = Model::create<AudioInterface, AudioInterfaceWidget>("Core", "AudioInterface", "Audio", EXTERNAL_TAG);