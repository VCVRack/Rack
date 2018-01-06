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

static auto audioTimeout = std::chrono::milliseconds(100);


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

	void processStream(const float *input, float *output, int length) override {
		if (numInputs > 0) {
			// TODO Do we need to wait on the input to be consumed here?
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

	AudioInterfaceIO audioIO;

	SampleRateConverter<8> inputSrc;
	SampleRateConverter<8> outputSrc;

	// in rack's sample rate
	DoubleRingBuffer<Frame<MAX_INPUTS>, 16> inputBuffer;
	DoubleRingBuffer<Frame<MAX_OUTPUTS>, 16> outputBuffer;

	AudioInterface() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
	}

	void step() override;
	void stepStream(const float *input, float *output, int numFrames);


	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "driver", json_integer(audioIO.getDriver()));
		std::string deviceName = audioIO.getDeviceName(audioIO.device);
		json_object_set_new(rootJ, "deviceName", json_string(deviceName.c_str()));
		json_object_set_new(rootJ, "sampleRate", json_integer(audioIO.sampleRate));
		json_object_set_new(rootJ, "blockSize", json_integer(audioIO.blockSize));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *driverJ = json_object_get(rootJ, "driver");
		if (driverJ)
			audioIO.setDriver(json_number_value(driverJ));

		json_t *deviceNameJ = json_object_get(rootJ, "deviceName");
		if (deviceNameJ) {
			std::string deviceName = json_string_value(deviceNameJ);
			// Search for device ID with equal name
			for (int device = 0; device < audioIO.getDeviceCount(); device++) {
				if (audioIO.getDeviceName(device) == deviceName) {
					audioIO.device = device;
					break;
				}
			}
		}

		json_t *sampleRateJ = json_object_get(rootJ, "sampleRate");
		if (sampleRateJ)
			audioIO.sampleRate = json_integer_value(sampleRateJ);

		json_t *blockSizeJ = json_object_get(rootJ, "blockSize");
		if (blockSizeJ)
			audioIO.blockSize = json_integer_value(blockSizeJ);

		audioIO.openStream();
	}

	void onReset() override {
		audioIO.closeStream();
	}
};


void AudioInterface::step() {
	Frame<MAX_INPUTS> inputFrame;
	memset(&inputFrame, 0, sizeof(inputFrame));

	if (audioIO.numInputs > 0) {
		if (inputBuffer.empty()) {
			inputSrc.setRates(audioIO.sampleRate, engineGetSampleRate());
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
				outputSrc.setRates(engineGetSampleRate(), audioIO.sampleRate);
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

	// addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	// addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 0)));
	// addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	// addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 365)));

	Vec margin = Vec(5, 2);
	float labelHeight = 15;
	float yPos = margin.y + 100;
	float xPos;

	{
		Label *label = new Label();
		label->box.pos = Vec(margin.x, yPos);
		label->text = "Outputs";
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
		label->text = "Inputs";
		addChild(label);
		yPos += labelHeight + margin.y;
	}

	yPos += 5;
	xPos = 10;
	for (int i = 0; i < 4; i++) {
		addOutput(createOutput<PJ3410Port>(Vec(xPos, yPos), module, AudioInterface::AUDIO_OUTPUT + i));
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
	// Widget *w = construct<DIN_MIDIWidget>();
	// w->box.pos = Vec(100, 0);
	// addChild(w);
}
