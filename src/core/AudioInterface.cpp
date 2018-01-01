#include <assert.h>
#include <mutex>
#include <thread>
#include "core.hpp"
#include "audio.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#include <RtAudio.h>
#pragma GCC diagnostic pop



using namespace rack;


struct AudioInterface : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		AUDIO1_INPUT,
		NUM_INPUTS = AUDIO1_INPUT + 8
	};
	enum OutputIds {
		AUDIO1_OUTPUT,
		NUM_OUTPUTS = AUDIO1_OUTPUT + 8
	};

	AudioIO audioIO;

	SampleRateConverter<8> inputSrc;
	SampleRateConverter<8> outputSrc;

	// in rack's sample rate
	DoubleRingBuffer<Frame<8>, 16> inputBuffer;
	DoubleRingBuffer<Frame<8>, (1<<15)> outputBuffer;
	// in device's sample rate
	DoubleRingBuffer<Frame<8>, (1<<15)> inputSrcBuffer;

	AudioInterface() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
	}

	void step() override;
	void stepStream(const float *input, float *output, int numFrames);


	json_t *toJson() override {
		json_t *rootJ = json_object();
		// json_object_set_new(rootJ, "driver", json_integer(getDriver()));
		// json_object_set_new(rootJ, "device", json_integer(device));
		// json_object_set_new(rootJ, "audioIO.sampleRate", json_real(audioIO.sampleRate));
		// json_object_set_new(rootJ, "audioIO.blockSize", json_integer(audioIO.blockSize));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// json_t *driverJ = json_object_get(rootJ, "driver");
		// if (driverJ)
		// 	setDriver(json_number_value(driverJ));

		// json_t *deviceJ = json_object_get(rootJ, "device");
		// if (deviceJ)
		// 	device = json_number_value(deviceJ);

		// json_t *sampleRateJ = json_object_get(rootJ, "audioIO.sampleRate");
		// if (sampleRateJ)
		// 	audioIO.sampleRate = json_number_value(sampleRateJ);

		// json_t *blockSizeJ = json_object_get(rootJ, "audioIO.blockSize");
		// if (blockSizeJ)
		// 	audioIO.blockSize = json_integer_value(blockSizeJ);

		// openStream();
	}

	void onReset() override {
		audioIO.closeStream();
	}
};


#define TIMED_SLEEP_LOCK(_cond, _spinTime, _totalTime) { \
	auto startTime = std::chrono::high_resolution_clock::now(); \
	while (!(_cond)) { \
		std::this_thread::sleep_for(std::chrono::duration<float>(_spinTime)); \
		auto currTime = std::chrono::high_resolution_clock::now(); \
		float totalTime = std::chrono::duration<float>(currTime - startTime).count(); \
		if (totalTime > (_totalTime)) \
			break; \
	} \
}


void AudioInterface::step() {
	// debug("inputBuffer %d inputSrcBuffer %d outputBuffer %d", inputBuffer.size(), inputSrcBuffer.size(), outputBuffer.size());
	// Read/write stream if we have enough input, OR the output buffer is empty if we have no input
	if (audioIO.numOutputs > 0) {
		TIMED_SLEEP_LOCK(inputSrcBuffer.size() < audioIO.blockSize, 100e-6, 0.2);
	}
	else if (audioIO.numInputs > 0) {
		TIMED_SLEEP_LOCK(!outputBuffer.empty(), 100e-6, 0.2);
	}

	// Get input and pass it through the sample rate converter
	if (audioIO.numOutputs > 0) {
		if (!inputBuffer.full()) {
			Frame<8> f;
			for (int i = 0; i < 8; i++) {
				f.samples[i] = inputs[AUDIO1_INPUT + i].value / 10.0;
			}
			inputBuffer.push(f);
		}

		// Once full, sample rate convert the input
		// inputBuffer -> SRC -> inputSrcBuffer
		if (inputBuffer.full()) {
			inputSrc.setRates(engineGetSampleRate(), audioIO.sampleRate);
			int inLen = inputBuffer.size();
			int outLen = inputSrcBuffer.capacity();
			inputSrc.process(inputBuffer.startData(), &inLen, inputSrcBuffer.endData(), &outLen);
			inputBuffer.startIncr(inLen);
			inputSrcBuffer.endIncr(outLen);
		}
	}

	// Set output
	if (!outputBuffer.empty()) {
		Frame<8> f = outputBuffer.shift();
		for (int i = 0; i < 8; i++) {
			outputs[AUDIO1_OUTPUT + i].value = 10.0 * f.samples[i];
		}
	}
}

void AudioInterface::stepStream(const float *input, float *output, int numFrames) {
	if (gPaused) {
		memset(output, 0, sizeof(float) * audioIO.numOutputs * numFrames);
		return;
	}

	if (audioIO.numOutputs > 0) {
		// Wait for enough input before proceeding
		TIMED_SLEEP_LOCK(inputSrcBuffer.size() >= numFrames, 100e-6, 0.2);
	}
	else if (audioIO.numInputs > 0) {
		TIMED_SLEEP_LOCK(outputBuffer.empty(), 100e-6, 0.2);
	}

	// input stream -> output buffer
	if (audioIO.numInputs > 0) {
		Frame<8> inputFrames[numFrames];
		for (int i = 0; i < numFrames; i++) {
			for (int c = 0; c < 8; c++) {
				inputFrames[i].samples[c] = (c < audioIO.numInputs) ? input[i*audioIO.numInputs + c] : 0.0;
			}
		}

		// Pass output through sample rate converter
		outputSrc.setRates(audioIO.sampleRate, engineGetSampleRate());
		int inLen = numFrames;
		int outLen = outputBuffer.capacity();
		outputSrc.process(inputFrames, &inLen, outputBuffer.endData(), &outLen);
		outputBuffer.endIncr(outLen);
	}

	// input buffer -> output stream
	if (audioIO.numOutputs > 0) {
		for (int i = 0; i < numFrames; i++) {
			Frame<8> f;
			if (inputSrcBuffer.empty()) {
				memset(&f, 0, sizeof(f));
			}
			else {
				f = inputSrcBuffer.shift();
			}
			for (int c = 0; c < audioIO.numOutputs; c++) {
				output[i*audioIO.numOutputs + c] = clampf(f.samples[c], -1.0, 1.0);
			}
		}
	}
}


// void AudioInterface::closeStream() {
// 	// Clear buffers
// 	inputBuffer.clear();
// 	outputBuffer.clear();
// 	inputSrcBuffer.clear();
// 	inputSrc.reset();
// 	outputSrc.reset();
// }



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
		addInput(createInput<PJ3410Port>(Vec(xPos, yPos), module, AudioInterface::AUDIO1_INPUT + i));
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
		addInput(createInput<PJ3410Port>(Vec(xPos, yPos), module, AudioInterface::AUDIO1_INPUT + i));
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
		addOutput(createOutput<PJ3410Port>(Vec(xPos, yPos), module, AudioInterface::AUDIO1_OUTPUT + i));
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
		addOutput(createOutput<PJ3410Port>(Vec(xPos, yPos), module, AudioInterface::AUDIO1_OUTPUT + i));
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
