#include <assert.h>
#include <mutex>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "core.hpp"
#include "audio.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#include <RtAudio.h>
#pragma GCC diagnostic pop


#define MAX_OUTPUTS 8
#define MAX_INPUTS 8

static const auto audioTimeout = std::chrono::milliseconds(100);


using namespace rack;


struct AudioInterfaceIO : AudioIO {
	std::mutex engineMutex;
	std::condition_variable engineCv;
	std::mutex audioMutex;
	std::condition_variable audioCv;
	// Audio thread produces, engine thread consumes
	DoubleRingBuffer<Frame<MAX_INPUTS>, (1<<15)> inputBuffer;
	// Audio thread consumes, engine thread produces
	DoubleRingBuffer<Frame<MAX_OUTPUTS>, (1<<15)> outputBuffer;

	AudioInterfaceIO() {
		maxOutputs = MAX_OUTPUTS;
		maxInputs = MAX_INPUTS;
	}

	~AudioInterfaceIO() {
		closeStream();
	}

	void processStream(const float *input, float *output, int length) override {
		if (numInputs > 0) {
			// TODO Do we need to wait on the input to be consumed here? Experimentally, it works fine if we don't.
			for (int i = 0; i < length; i++) {
				if (inputBuffer.full())
					break;
				Frame<MAX_INPUTS> f;
				memset(&f, 0, sizeof(f));
				memcpy(&f, &input[numInputs * i], numInputs * sizeof(float));
				inputBuffer.push(f);
			}
		}

		if (numOutputs > 0) {
			std::unique_lock<std::mutex> lock(audioMutex);
			auto cond = [&] {
				return outputBuffer.size() >= length;
			};
			if (audioCv.wait_for(lock, audioTimeout, cond)) {
				// Consume audio block
				for (int i = 0; i < length; i++) {
					Frame<MAX_OUTPUTS> f = outputBuffer.shift();
					memcpy(&output[numOutputs * i], &f, numOutputs * sizeof(float));
				}
			}
			else {
				// Timed out, fill output with zeros
				memset(output, 0, length * numOutputs * sizeof(float));
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
		ENUMS(AUDIO_INPUT, MAX_INPUTS),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(AUDIO_OUTPUT, MAX_OUTPUTS),
		NUM_OUTPUTS
	};
	enum LightIds {
		ACTIVE_LIGHT,
		NUM_LIGHTS
	};

	AudioInterfaceIO audioIO;
	int lastSampleRate = 0;

	SampleRateConverter<MAX_INPUTS> inputSrc;
	SampleRateConverter<MAX_OUTPUTS> outputSrc;

	// in rack's sample rate
	DoubleRingBuffer<Frame<MAX_INPUTS>, 16> inputBuffer;
	DoubleRingBuffer<Frame<MAX_OUTPUTS>, 16> outputBuffer;

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
		// for (int i = 0; i < MAX_INPUTS; i++) {
		// 	inputSrc[i].setRates(audioIO.sampleRate, engineGetSampleRate());
		// }
		// for (int i = 0; i < MAX_OUTPUTS; i++) {
		// 	outputSrc[i].setRates(engineGetSampleRate(), audioIO.sampleRate);
		// }
		inputSrc.setRates(audioIO.sampleRate, engineGetSampleRate());
		outputSrc.setRates(engineGetSampleRate(), audioIO.sampleRate);
	}

	void onReset() override {
		audioIO.closeStream();
	}
};


void AudioInterface::step() {
	Frame<MAX_INPUTS> inputFrame;
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
	for (int i = 0; i < MAX_INPUTS; i++) {
		outputs[AUDIO_OUTPUT + i].value = 10.0 * inputFrame.samples[i];
	}

	if (audioIO.numOutputs > 0) {
		// Get and push output SRC frame
		if (!outputBuffer.full()) {
			Frame<MAX_OUTPUTS> f;
			for (int i = 0; i < audioIO.numOutputs; i++) {
				f.samples[i] = inputs[AUDIO_INPUT + i].value / 10.0;
			}
			outputBuffer.push(f);
		}

		if (outputBuffer.full()) {
			// Wait until outputs are needed
			std::unique_lock<std::mutex> lock(audioIO.engineMutex);
			auto cond = [&] {
				return audioIO.outputBuffer.size() < audioIO.blockSize;
			};
			if (audioIO.engineCv.wait_for(lock, audioTimeout, cond)) {
				// Push converted output
				int inLen = outputBuffer.size();
				int outLen = audioIO.outputBuffer.capacity();
				outputSrc.process(outputBuffer.startData(), &inLen, audioIO.outputBuffer.endData(), &outLen);
				outputBuffer.startIncr(inLen);
				audioIO.outputBuffer.endIncr(outLen);
			}
			else {
				// Give up on pushing output
			}
		}
	}

	audioIO.audioCv.notify_all();

	// Lights
	lights[ACTIVE_LIGHT].value = audioIO.isActive() ? 1.0 : 0.0;
}


AudioInterfaceWidget::AudioInterfaceWidget() {
	AudioInterface *module = new AudioInterface();
	setModule(module);
	box.size = Vec(15*12, 380);
	{
		Panel *panel = new LightPanel();
		panel->box.size = box.size;
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 365)));

	Vec margin = Vec(5, 2);
	float labelHeight = 15;
	float yPos = margin.y + 100;
	float xPos;

	{
		Label *label = new Label();
		label->box.pos = Vec(margin.x, yPos);
		label->text = "Outputs (DACs)";
		addChild(label);
		yPos += labelHeight + margin.y;
	}

	yPos += 5;
	xPos = 10;
	for (int i = 0; i < 4; i++) {
		addInput(createInput<PJ3410Port>(Vec(xPos, yPos), module, AudioInterface::AUDIO_INPUT + i));
		Label *label = new Label();
		label->box.pos = Vec(xPos + 4, yPos + 28);
		label->text = stringf("%d", i + 1);
		addChild(label);

		xPos += 37 + margin.x;
	}
	yPos += 35 + margin.y;

	yPos += 5;
	xPos = 10;
	for (int i = 4; i < 8; i++) {
		addInput(createInput<PJ3410Port>(Vec(xPos, yPos), module, AudioInterface::AUDIO_INPUT + i));
		Label *label = new Label();
		label->box.pos = Vec(xPos + 4, yPos + 28);
		label->text = stringf("%d", i + 1);
		addChild(label);

		xPos += 37 + margin.x;
	}
	yPos += 35 + margin.y;

	{
		Label *label = new Label();
		label->box.pos = Vec(margin.x, yPos);
		label->text = "Inputs (ADCs)";
		addChild(label);
		yPos += labelHeight + margin.y;
	}

	yPos += 5;
	xPos = 10;
	for (int i = 0; i < 4; i++) {
		Port *port = createOutput<PJ3410Port>(Vec(xPos, yPos), module, AudioInterface::AUDIO_OUTPUT + i);
		addOutput(port);
		Label *label = new Label();
		label->box.pos = Vec(xPos + 4, yPos + 28);
		label->text = stringf("%d", i + 1);
		addChild(label);

		xPos += 37 + margin.x;
	}
	yPos += 35 + margin.y;

	yPos += 5;
	xPos = 10;
	for (int i = 4; i < 8; i++) {
		addOutput(createOutput<PJ3410Port>(Vec(xPos, yPos), module, AudioInterface::AUDIO_OUTPUT + i));
		Label *label = new Label();
		label->box.pos = Vec(xPos + 4, yPos + 28);
		label->text = stringf("%d", i + 1);
		addChild(label);

		xPos += 37 + margin.x;
	}
	yPos += 35 + margin.y;

	AudioWidget *audioWidget = construct<USB_B_AudioWidget>();
	audioWidget->audioIO = &module->audioIO;
	addChild(audioWidget);

	// Lights
	addChild(createLight<SmallLight<GreenLight>>(Vec(40, 20), module, AudioInterface::ACTIVE_LIGHT));
}
