#include <assert.h>
#include <mutex>
#include <thread>
#include <portaudio.h>
#include "core.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"


using namespace rack;

static bool initialized = false;

void audioInit() {
	if (initialized)
		return;

	PaError err = Pa_Initialize();
	if (err) {
		fprintf(stderr, "Failed to initialize PortAudio: %s\n", Pa_GetErrorText(err));
		return;
	}
	initialized = true;
}


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

	PaStream *stream = NULL;
	// Stream properties
	int deviceId = -1;
	float sampleRate = 44100.0;
	int blockSize = 256;
	int numOutputs = 0;
	int numInputs = 0;

	// Used because the GUI thread and Rack thread can both interact with this class
	std::mutex bufferMutex;
	bool streamRunning;

	SampleRateConverter<8> inputSrc;
	SampleRateConverter<8> outputSrc;

	// in rack's sample rate
	DoubleRingBuffer<Frame<8>, 16> inputBuffer;
	DoubleRingBuffer<Frame<8>, (1<<15)> outputBuffer;
	// in device's sample rate
	DoubleRingBuffer<Frame<8>, (1<<15)> inputSrcBuffer;

	AudioInterface() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
		audioInit();
	}
	~AudioInterface() {
		closeDevice();
	}

	void step();
	void stepStream(const float *input, float *output, int numFrames);

	int getDeviceCount();
	std::string getDeviceName(int deviceId);

	void openDevice(int deviceId, float sampleRate, int blockSize);
	void closeDevice();

	void setDeviceId(int deviceId) {
		openDevice(deviceId, sampleRate, blockSize);
	}
	void setSampleRate(float sampleRate) {
		openDevice(deviceId, sampleRate, blockSize);
	}
	void setBlockSize(int blockSize) {
		openDevice(deviceId, sampleRate, blockSize);
	}

	json_t *toJson() {
		json_t *rootJ = json_object();
		if (deviceId >= 0) {
			std::string deviceName = getDeviceName(deviceId);
			json_object_set_new(rootJ, "deviceName", json_string(deviceName.c_str()));
			json_object_set_new(rootJ, "sampleRate", json_real(sampleRate));
			json_object_set_new(rootJ, "blockSize", json_integer(blockSize));
		}
		return rootJ;
	}

	void fromJson(json_t *rootJ) {
		json_t *deviceNameJ = json_object_get(rootJ, "deviceName");
		if (deviceNameJ) {
			std::string deviceName = json_string_value(deviceNameJ);
			for (int i = 0; i < getDeviceCount(); i++) {
				if (deviceName == getDeviceName(i)) {
					setDeviceId(i);
					break;
				}
			}
		}

		json_t *sampleRateJ = json_object_get(rootJ, "sampleRate");
		if (sampleRateJ) {
			setSampleRate(json_number_value(sampleRateJ));
		}

		json_t *blockSizeJ = json_object_get(rootJ, "blockSize");
		if (blockSizeJ) {
			setBlockSize(json_integer_value(blockSizeJ));
		}
	}

	void initialize() {
		closeDevice();
	}
};


void AudioInterface::step() {
	if (!stream)
		return;

	// Read/write stream if we have enough input, OR the output buffer is empty if we have no input
	if (numOutputs > 0) {
		while (inputSrcBuffer.size() >= blockSize && streamRunning) {
			std::this_thread::sleep_for(std::chrono::duration<float>(100e-6));
		}
	}
	else if (numInputs > 0) {
		while (outputBuffer.empty() && streamRunning) {
			std::this_thread::sleep_for(std::chrono::duration<float>(100e-6));
		}
	}

	std::lock_guard<std::mutex> lock(bufferMutex);

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
			inputSrc.setRatio(sampleRate / engineGetSampleRate());
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
	if (numOutputs > 0) {
		// Wait for enough input before proceeding
		while (inputSrcBuffer.size() < numFrames) {
			if (!streamRunning)
				return;
			std::this_thread::sleep_for(std::chrono::duration<float>(100e-6));
		}
	}

	std::lock_guard<std::mutex> lock(bufferMutex);
	// input stream -> output buffer
	if (numInputs > 0) {
		Frame<8> inputFrames[numFrames];
		for (int i = 0; i < numFrames; i++) {
			for (int c = 0; c < 8; c++) {
				inputFrames[i].samples[c] = (c < numInputs) ? input[i*numInputs + c] : 0.0;
			}
		}

		// Pass output through sample rate converter
		outputSrc.setRatio(engineGetSampleRate() / sampleRate);
		int inLen = numFrames;
		int outLen = outputBuffer.capacity();
		outputSrc.process(inputFrames, &inLen, outputBuffer.endData(), &outLen);
		outputBuffer.endIncr(outLen);
	}

	// input buffer -> output stream
	if (numOutputs > 0) {
		for (int i = 0; i < numFrames; i++) {
			if (inputSrcBuffer.empty())
				break;
			Frame<8> f = inputSrcBuffer.shift();
			for (int c = 0; c < numOutputs; c++) {
				output[i*numOutputs + c] = (c < 8) ? clampf(f.samples[c], -1.0, 1.0) : 0.0;
			}
		}
	}
}

int AudioInterface::getDeviceCount() {
	return Pa_GetDeviceCount();
}

std::string AudioInterface::getDeviceName(int deviceId) {
	const PaDeviceInfo *info = Pa_GetDeviceInfo(deviceId);
	if (!info)
		return "";
	const PaHostApiInfo *apiInfo = Pa_GetHostApiInfo(info->hostApi);
	return stringf("%s: %s (%d in, %d out)", apiInfo->name, info->name, info->maxInputChannels, info->maxOutputChannels);
}

static int paCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
	AudioInterface *p = (AudioInterface *) userData;
	p->stepStream((const float *) inputBuffer, (float *) outputBuffer, framesPerBuffer);
	return paContinue;
}

void AudioInterface::openDevice(int deviceId, float sampleRate, int blockSize) {
	closeDevice();
	std::lock_guard<std::mutex> lock(bufferMutex);

	this->sampleRate = sampleRate;
	this->blockSize = blockSize;

	// Open new device
	if (deviceId >= 0) {
		PaError err;
		const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(deviceId);
		if (!deviceInfo) {
			fprintf(stderr, "Failed to query audio device\n");
			return;
		}

		numOutputs = mini(deviceInfo->maxOutputChannels, 8);
		numInputs = mini(deviceInfo->maxInputChannels, 8);

		PaStreamParameters outputParameters;
		outputParameters.device = deviceId;
		outputParameters.channelCount = numOutputs;
		outputParameters.sampleFormat = paFloat32;
		outputParameters.suggestedLatency = deviceInfo->defaultLowOutputLatency;
		outputParameters.hostApiSpecificStreamInfo = NULL;

		PaStreamParameters inputParameters;
		inputParameters.device = deviceId;
		inputParameters.channelCount = numInputs;
		inputParameters.sampleFormat = paFloat32;
		inputParameters.suggestedLatency = deviceInfo->defaultLowInputLatency;
		inputParameters.hostApiSpecificStreamInfo = NULL;

		// Don't use stream parameters if 0 input or output channels
		err = Pa_OpenStream(&stream,
			numInputs == 0 ? NULL : &inputParameters,
			numOutputs == 0 ? NULL : &outputParameters,
			sampleRate, blockSize, paNoFlag, paCallback, this);
		if (err) {
			fprintf(stderr, "Failed to open audio stream: %s\n", Pa_GetErrorText(err));
			return;
		}

		err = Pa_StartStream(stream);
		if (err) {
			fprintf(stderr, "Failed to start audio stream: %s\n", Pa_GetErrorText(err));
			return;
		}
		// This should go after Pa_StartStream because sometimes it will call the callback once synchronously, and that time it should return early
		streamRunning = true;

		// Correct sample rate
		const PaStreamInfo *streamInfo = Pa_GetStreamInfo(stream);
		this->sampleRate = streamInfo->sampleRate;
		this->deviceId = deviceId;
	}
}

void AudioInterface::closeDevice() {
	std::lock_guard<std::mutex> lock(bufferMutex);

	if (stream) {
		PaError err;
		streamRunning = false;
		err = Pa_AbortStream(stream);
		// err = Pa_StopStream(stream);
		if (err) {
			fprintf(stderr, "Failed to stop audio stream: %s\n", Pa_GetErrorText(err));
		}

		err = Pa_CloseStream(stream);
		if (err) {
			fprintf(stderr, "Failed to close audio stream: %s\n", Pa_GetErrorText(err));
		}
	}

	// Reset stream settings
	stream = NULL;
	deviceId = -1;
	numOutputs = 0;
	numInputs = 0;

	// Clear buffers
	inputBuffer.clear();
	outputBuffer.clear();
	inputSrcBuffer.clear();
	inputSrc.reset();
	outputSrc.reset();
}


struct AudioItem : MenuItem {
	AudioInterface *audioInterface;
	int deviceId;
	void onAction() {
		audioInterface->setDeviceId(deviceId);
	}
};

struct AudioChoice : ChoiceButton {
	AudioInterface *audioInterface;
	void onAction() {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		int deviceCount = audioInterface->getDeviceCount();
		{
			AudioItem *audioItem = new AudioItem();
			audioItem->audioInterface = audioInterface;
			audioItem->deviceId = -1;
			audioItem->text = "No device";
			menu->pushChild(audioItem);
		}
		for (int deviceId = 0; deviceId < deviceCount; deviceId++) {
			AudioItem *audioItem = new AudioItem();
			audioItem->audioInterface = audioInterface;
			audioItem->deviceId = deviceId;
			audioItem->text = audioInterface->getDeviceName(deviceId);
			menu->pushChild(audioItem);
		}
	}
	void step() {
		std::string name = audioInterface->getDeviceName(audioInterface->deviceId);
		text = ellipsize(name, 24);
	}
};


struct SampleRateItem : MenuItem {
	AudioInterface *audioInterface;
	float sampleRate;
	void onAction() {
		audioInterface->setSampleRate(sampleRate);
	}
};

struct SampleRateChoice : ChoiceButton {
	AudioInterface *audioInterface;
	void onAction() {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		const float sampleRates[6] = {44100, 48000, 88200, 96000, 176400, 192000};
		int sampleRatesLen = sizeof(sampleRates) / sizeof(sampleRates[0]);
		for (int i = 0; i < sampleRatesLen; i++) {
			SampleRateItem *item = new SampleRateItem();
			item->audioInterface = audioInterface;
			item->sampleRate = sampleRates[i];
			item->text = stringf("%.0f Hz", sampleRates[i]);
			menu->pushChild(item);
		}
	}
	void step() {
		this->text = stringf("%.0f Hz", audioInterface->sampleRate);
	}
};


struct BlockSizeItem : MenuItem {
	AudioInterface *audioInterface;
	int blockSize;
	void onAction() {
		audioInterface->setBlockSize(blockSize);
	}
};

struct BlockSizeChoice : ChoiceButton {
	AudioInterface *audioInterface;
	void onAction() {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		const int blockSizes[] = {64, 128, 256, 512, 1024, 2048, 4096};
		int blockSizesLen = sizeof(blockSizes) / sizeof(blockSizes[0]);
		for (int i = 0; i < blockSizesLen; i++) {
			BlockSizeItem *item = new BlockSizeItem();
			item->audioInterface = audioInterface;
			item->blockSize = blockSizes[i];
			item->text = stringf("%d", blockSizes[i]);
			menu->pushChild(item);
		}
	}
	void step() {
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

	float margin = 5;
	float labelHeight = 15;
	float yPos = margin;
	float xPos;

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Audio device";
		addChild(label);
		yPos += labelHeight + margin;

		AudioChoice *choice = new AudioChoice();
		choice->audioInterface = dynamic_cast<AudioInterface*>(module);
		choice->box.pos = Vec(margin, yPos);
		choice->box.size.x = box.size.x - 2*margin;
		addChild(choice);
		yPos += choice->box.size.y + margin;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Sample rate";
		addChild(label);
		yPos += labelHeight + margin;

		SampleRateChoice *choice = new SampleRateChoice();
		choice->audioInterface = dynamic_cast<AudioInterface*>(module);
		choice->box.pos = Vec(margin, yPos);
		choice->box.size.x = box.size.x - 2*margin;
		addChild(choice);
		yPos += choice->box.size.y + margin;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Block size";
		addChild(label);
		yPos += labelHeight + margin;

		BlockSizeChoice *choice = new BlockSizeChoice();
		choice->audioInterface = dynamic_cast<AudioInterface*>(module);
		choice->box.pos = Vec(margin, yPos);
		choice->box.size.x = box.size.x - 2*margin;
		addChild(choice);
		yPos += choice->box.size.y + margin;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Outputs";
		addChild(label);
		yPos += labelHeight + margin;
	}

	yPos += 5;
	xPos = 10;
	for (int i = 0; i < 4; i++) {
		addInput(createInput<PJ3410Port>(Vec(xPos, yPos), module, AudioInterface::AUDIO1_INPUT + i));
		Label *label = new Label();
		label->box.pos = Vec(xPos + 4, yPos + 28);
		label->text = stringf("%d", i + 1);
		addChild(label);

		xPos += 37 + margin;
	}
	yPos += 35 + margin;

	yPos += 5;
	xPos = 10;
	for (int i = 4; i < 8; i++) {
		addInput(createInput<PJ3410Port>(Vec(xPos, yPos), module, AudioInterface::AUDIO1_INPUT + i));
		Label *label = new Label();
		label->box.pos = Vec(xPos + 4, yPos + 28);
		label->text = stringf("%d", i + 1);
		addChild(label);

		xPos += 37 + margin;
	}
	yPos += 35 + margin;

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Inputs";
		addChild(label);
		yPos += labelHeight + margin;
	}

	yPos += 5;
	xPos = 10;
	for (int i = 0; i < 4; i++) {
		addOutput(createOutput<PJ3410Port>(Vec(xPos, yPos), module, AudioInterface::AUDIO1_OUTPUT + i));
		Label *label = new Label();
		label->box.pos = Vec(xPos + 4, yPos + 28);
		label->text = stringf("%d", i + 1);
		addChild(label);

		xPos += 37 + margin;
	}
	yPos += 35 + margin;

	yPos += 5;
	xPos = 10;
	for (int i = 4; i < 8; i++) {
		addOutput(createOutput<PJ3410Port>(Vec(xPos, yPos), module, AudioInterface::AUDIO1_OUTPUT + i));
		Label *label = new Label();
		label->box.pos = Vec(xPos + 4, yPos + 28);
		label->text = stringf("%d", i + 1);
		addChild(label);

		xPos += 37 + margin;
	}
	yPos += 35 + margin;
}
