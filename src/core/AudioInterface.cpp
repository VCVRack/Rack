#include <assert.h>
#include <mutex>
#include <thread>
#include <algorithm>
#include "core.hpp"
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

	RtAudio *stream = NULL;
	// Stream properties
	int device = -1;
	float sampleRate = 44100.0;
	int blockSize = 256;
	int numOutputs = 0;
	int numInputs = 0;

	SampleRateConverter<8> inputSrc;
	SampleRateConverter<8> outputSrc;

	// in rack's sample rate
	DoubleRingBuffer<Frame<8>, 16> inputBuffer;
	DoubleRingBuffer<Frame<8>, (1<<15)> outputBuffer;
	// in device's sample rate
	DoubleRingBuffer<Frame<8>, (1<<15)> inputSrcBuffer;

	AudioInterface() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
		setDriver(RtAudio::UNSPECIFIED);
	}
	~AudioInterface() {
		closeStream();
	}

	void step() override;
	void stepStream(const float *input, float *output, int numFrames);

	int getDeviceCount();
	std::string getDeviceName(int device);

	void openStream();
	void closeStream();

	void setDriver(int driver) {
		closeStream();
		if (stream)
			delete stream;
		stream = new RtAudio((RtAudio::Api) driver);
	}
	int getDriver() {
		if (!stream)
			return RtAudio::UNSPECIFIED;
		return stream->getCurrentApi();
	}
	std::vector<int> getAvailableDrivers() {
		std::vector<RtAudio::Api> apis;
		RtAudio::getCompiledApi(apis);
		std::vector<int> drivers;
		for (RtAudio::Api api : apis)
			drivers.push_back(api);
		return drivers;
	}
	std::string getDriverName(int driver) {
		switch (driver) {
			case RtAudio::UNSPECIFIED: return "Unspecified";
			case RtAudio::LINUX_ALSA: return "ALSA";
			case RtAudio::LINUX_PULSE: return "PulseAudio";
			case RtAudio::LINUX_OSS: return "OSS";
			case RtAudio::UNIX_JACK: return "JACK";
			case RtAudio::MACOSX_CORE: return "Core Audio";
			case RtAudio::WINDOWS_WASAPI: return "WASAPI";
			case RtAudio::WINDOWS_ASIO: return "ASIO";
			case RtAudio::WINDOWS_DS: return "DirectSound";
			case RtAudio::RTAUDIO_DUMMY: return "Dummy";
			default: return "Unknown";
		}
	}

	std::vector<float> getSampleRates();

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "driver", json_integer(getDriver()));
		json_object_set_new(rootJ, "device", json_integer(device));
		json_object_set_new(rootJ, "sampleRate", json_real(sampleRate));
		json_object_set_new(rootJ, "blockSize", json_integer(blockSize));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *driverJ = json_object_get(rootJ, "driver");
		if (driverJ)
			setDriver(json_number_value(driverJ));

		json_t *deviceJ = json_object_get(rootJ, "device");
		if (deviceJ)
			device = json_number_value(deviceJ);

		json_t *sampleRateJ = json_object_get(rootJ, "sampleRate");
		if (sampleRateJ)
			sampleRate = json_number_value(sampleRateJ);

		json_t *blockSizeJ = json_object_get(rootJ, "blockSize");
		if (blockSizeJ)
			blockSize = json_integer_value(blockSizeJ);

		openStream();
	}

	void onReset() override {
		closeStream();
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
	if (numOutputs > 0) {
		TIMED_SLEEP_LOCK(inputSrcBuffer.size() < blockSize, 100e-6, 0.2);
	}
	else if (numInputs > 0) {
		TIMED_SLEEP_LOCK(!outputBuffer.empty(), 100e-6, 0.2);
	}

	// Get input and pass it through the sample rate converter
	if (numOutputs > 0) {
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
			inputSrc.setRates(engineGetSampleRate(), sampleRate);
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
		memset(output, 0, sizeof(float) * numOutputs * numFrames);
		return;
	}

	if (numOutputs > 0) {
		// Wait for enough input before proceeding
		TIMED_SLEEP_LOCK(inputSrcBuffer.size() >= numFrames, 100e-6, 0.2);
	}
	else if (numInputs > 0) {
		TIMED_SLEEP_LOCK(outputBuffer.empty(), 100e-6, 0.2);
	}

	// input stream -> output buffer
	if (numInputs > 0) {
		Frame<8> inputFrames[numFrames];
		for (int i = 0; i < numFrames; i++) {
			for (int c = 0; c < 8; c++) {
				inputFrames[i].samples[c] = (c < numInputs) ? input[i*numInputs + c] : 0.0;
			}
		}

		// Pass output through sample rate converter
		outputSrc.setRates(sampleRate, engineGetSampleRate());
		int inLen = numFrames;
		int outLen = outputBuffer.capacity();
		outputSrc.process(inputFrames, &inLen, outputBuffer.endData(), &outLen);
		outputBuffer.endIncr(outLen);
	}

	// input buffer -> output stream
	if (numOutputs > 0) {
		for (int i = 0; i < numFrames; i++) {
			Frame<8> f;
			if (inputSrcBuffer.empty()) {
				memset(&f, 0, sizeof(f));
			}
			else {
				f = inputSrcBuffer.shift();
			}
			for (int c = 0; c < numOutputs; c++) {
				output[i*numOutputs + c] = clampf(f.samples[c], -1.0, 1.0);
			}
		}
	}
}

int AudioInterface::getDeviceCount() {
	if (!stream)
		return 0;
	return stream->getDeviceCount();
}

std::string AudioInterface::getDeviceName(int device) {
	if (!stream || device < 0)
		return "";

	try {
		RtAudio::DeviceInfo deviceInfo = stream->getDeviceInfo(device);
		return stringf("%s (%d in, %d out)", deviceInfo.name.c_str(), deviceInfo.inputChannels, deviceInfo.outputChannels);
	}
	catch (RtAudioError &e) {
		warn("Failed to query audio device: %s", e.what());
		return "";
	}
}

static int rtCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void *userData) {
	AudioInterface *audioInterface = (AudioInterface *) userData;
	assert(audioInterface);
	audioInterface->stepStream((const float *) inputBuffer, (float *) outputBuffer, nFrames);
	return 0;
}

void AudioInterface::openStream() {
	int device = this->device;
	closeStream();
	if (!stream)
		return;

	// Open new device
	if (device >= 0) {
		RtAudio::DeviceInfo deviceInfo;
		try {
			deviceInfo = stream->getDeviceInfo(device);
		}
		catch (RtAudioError &e) {
			warn("Failed to query audio device: %s", e.what());
			return;
		}

		numOutputs = mini(deviceInfo.outputChannels, 8);
		numInputs = mini(deviceInfo.inputChannels, 8);

		if (numOutputs == 0 && numInputs == 0) {
			warn("Audio device %d has 0 inputs and 0 outputs");
			return;
		}

		RtAudio::StreamParameters outParameters;
		outParameters.deviceId = device;
		outParameters.nChannels = numOutputs;

		RtAudio::StreamParameters inParameters;
		inParameters.deviceId = device;
		inParameters.nChannels = numInputs;

		RtAudio::StreamOptions options;
		// options.flags |= RTAUDIO_SCHEDULE_REALTIME;

		// Find closest sample rate
		unsigned int closestSampleRate = 0;
		for (unsigned int sr : deviceInfo.sampleRates) {
			if (fabsf(sr - sampleRate) < fabsf(closestSampleRate - sampleRate)) {
				closestSampleRate = sr;
			}
		}

		try {
			// Don't use stream parameters if 0 input or output channels
			debug("Opening audio stream %d", device);
			stream->openStream(
				numOutputs == 0 ? NULL : &outParameters,
				numInputs == 0 ? NULL : &inParameters,
				RTAUDIO_FLOAT32, closestSampleRate, (unsigned int*) &blockSize, &rtCallback, this, &options, NULL);
		}
		catch (RtAudioError &e) {
			warn("Failed to open audio stream: %s", e.what());
			return;
		}

		try {
			debug("Starting audio stream %d", device);
			stream->startStream();
		}
		catch (RtAudioError &e) {
			warn("Failed to start audio stream: %s", e.what());
			return;
		}

		// Update sample rate because this may have changed
		this->sampleRate = stream->getStreamSampleRate();
		this->device = device;
	}
}

void AudioInterface::closeStream() {
	if (stream) {
		if (stream->isStreamRunning()) {
			debug("Aborting audio stream %d", device);
			try {
				stream->abortStream();
			}
			catch (RtAudioError &e) {
				warn("Failed to abort stream %s", e.what());
			}
		}
		if (stream->isStreamOpen()) {
			debug("Closing audio stream %d", device);
			try {
				stream->closeStream();
			}
			catch (RtAudioError &e) {
				warn("Failed to close stream %s", e.what());
			}
		}
	}

	// Reset stream settings
	device = -1;
	numOutputs = 0;
	numInputs = 0;

	// Clear buffers
	inputBuffer.clear();
	outputBuffer.clear();
	inputSrcBuffer.clear();
	inputSrc.reset();
	outputSrc.reset();
}

std::vector<float> AudioInterface::getSampleRates() {
	std::vector<float> allowedSampleRates = {44100, 48000, 88200, 96000, 176400, 192000};
	if (!stream || device < 0)
		return allowedSampleRates;

	try {
		std::vector<float> sampleRates;
		RtAudio::DeviceInfo deviceInfo = stream->getDeviceInfo(device);
		for (int sr : deviceInfo.sampleRates) {
			float sampleRate = sr;
			auto allowedIt = std::find(allowedSampleRates.begin(), allowedSampleRates.end(), sampleRate);
			if (allowedIt != allowedSampleRates.end()) {
				sampleRates.push_back(sampleRate);
			}
		}
		return sampleRates;
	}
	catch (RtAudioError &e) {
		warn("Failed to query audio device: %s", e.what());
		return {};
	}
}



struct AudioDriverItem : MenuItem {
	AudioInterface *audioInterface;
	int driver;
	void onAction(EventAction &e) override {
		audioInterface->setDriver(driver);
	}
};

struct AudioDriverChoice : ChoiceButton {
	AudioInterface *audioInterface;
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y)).round();
		menu->box.size.x = box.size.x;

		for (int driver : audioInterface->getAvailableDrivers()) {
			AudioDriverItem *audioItem = new AudioDriverItem();
			audioItem->audioInterface = audioInterface;
			audioItem->driver = driver;
			audioItem->text = audioInterface->getDriverName(driver);
			menu->addChild(audioItem);
		}
	}
	void step() override {
		text = audioInterface->getDriverName(audioInterface->getDriver());
	}
};


struct AudioDeviceItem : MenuItem {
	AudioInterface *audioInterface;
	int device;
	void onAction(EventAction &e) override {
		audioInterface->device = device;
		audioInterface->openStream();
	}
};

struct AudioDeviceChoice : ChoiceButton {
	int lastDeviceId = -1;
	AudioInterface *audioInterface;
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y)).round();
		menu->box.size.x = box.size.x;

		int deviceCount = audioInterface->getDeviceCount();
		{
			AudioDeviceItem *audioItem = new AudioDeviceItem();
			audioItem->audioInterface = audioInterface;
			audioItem->device = -1;
			audioItem->text = "No device";
			menu->addChild(audioItem);
		}
		for (int device = 0; device < deviceCount; device++) {
			AudioDeviceItem *audioItem = new AudioDeviceItem();
			audioItem->audioInterface = audioInterface;
			audioItem->device = device;
			audioItem->text = audioInterface->getDeviceName(device);
			menu->addChild(audioItem);
		}
	}
	void step() override {
		if (lastDeviceId != audioInterface->device) {
			std::string name = audioInterface->getDeviceName(audioInterface->device);
			text = ellipsize(name, 24);
			lastDeviceId = audioInterface->device;
		}
	}
};


struct SampleRateItem : MenuItem {
	AudioInterface *audioInterface;
	float sampleRate;
	void onAction(EventAction &e) override {
		audioInterface->sampleRate = sampleRate;
		audioInterface->openStream();
	}
};

struct SampleRateChoice : ChoiceButton {
	AudioInterface *audioInterface;
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y)).round();
		menu->box.size.x = box.size.x;

		for (float sampleRate : audioInterface->getSampleRates()) {
			SampleRateItem *item = new SampleRateItem();
			item->audioInterface = audioInterface;
			item->sampleRate = sampleRate;
			item->text = stringf("%.0f Hz", sampleRate);
			menu->addChild(item);
		}
	}
	void step() override {
		this->text = stringf("%.0f Hz", audioInterface->sampleRate);
	}
};


struct BlockSizeItem : MenuItem {
	AudioInterface *audioInterface;
	int blockSize;
	void onAction(EventAction &e) override {
		audioInterface->blockSize = blockSize;
		audioInterface->openStream();
	}
};

struct BlockSizeChoice : ChoiceButton {
	AudioInterface *audioInterface;
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y)).round();
		menu->box.size.x = box.size.x;

		const int blockSizes[] = {64, 128, 256, 512, 1024, 2048, 4096};
		int blockSizesLen = sizeof(blockSizes) / sizeof(blockSizes[0]);
		for (int i = 0; i < blockSizesLen; i++) {
			BlockSizeItem *item = new BlockSizeItem();
			item->audioInterface = audioInterface;
			item->blockSize = blockSizes[i];
			item->text = stringf("%d", blockSizes[i]);
			menu->addChild(item);
		}
	}
	void step() override {
		this->text = stringf("%d", audioInterface->blockSize);
	}
};


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
	float yPos = margin.y;
	float xPos;

	{
		Label *label = new Label();
		label->box.pos = Vec(margin.x, yPos);
		label->text = "Audio driver";
		addChild(label);
		yPos += labelHeight + margin.y;

		AudioDriverChoice *choice = new AudioDriverChoice();
		choice->audioInterface = module;
		choice->box.pos = Vec(margin.x, yPos);
		choice->box.size.x = box.size.x - 2*margin.x;
		addChild(choice);
		yPos += choice->box.size.y + margin.y;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin.x, yPos);
		label->text = "Audio device";
		addChild(label);
		yPos += labelHeight + margin.y;

		AudioDeviceChoice *choice = new AudioDeviceChoice();
		choice->audioInterface = module;
		choice->box.pos = Vec(margin.x, yPos);
		choice->box.size.x = box.size.x - 2*margin.x;
		addChild(choice);
		yPos += choice->box.size.y + margin.y;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin.x, yPos);
		label->text = "Sample rate";
		addChild(label);
		yPos += labelHeight + margin.y;

		SampleRateChoice *choice = new SampleRateChoice();
		choice->audioInterface = module;
		choice->box.pos = Vec(margin.x, yPos);
		choice->box.size.x = box.size.x - 2*margin.x;
		addChild(choice);
		yPos += choice->box.size.y + margin.y;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin.x, yPos);
		label->text = "Block size";
		addChild(label);
		yPos += labelHeight + margin.y;

		BlockSizeChoice *choice = new BlockSizeChoice();
		choice->audioInterface = module;
		choice->box.pos = Vec(margin.x, yPos);
		choice->box.size.x = box.size.x - 2*margin.x;
		addChild(choice);
		yPos += choice->box.size.y + margin.y;
	}

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
}
