#include <assert.h>
#include <mutex>
#include <portaudio.h>
#include "core.hpp"
#include "dsp.hpp"

using namespace rack;


void audioInit() {
	PaError err = Pa_Initialize();
	if (err) {
		fprintf(stderr, "Failed to initialize PortAudio: %s\n", Pa_GetErrorText(err));
		return;
	}
}


struct AudioInterface : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		AUDIO1_INPUT,
		AUDIO2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO1_OUTPUT,
		AUDIO2_OUTPUT,
		NUM_OUTPUTS
	};

	PaStream *stream = NULL;
	// Stream properties
	int deviceId = -1;
	float sampleRate = 44100.0;
	int blockSize = 256;
	int numOutputs = 0;
	int numInputs = 0;

	// Used because the GUI thread and Rack thread can both interact with this class
	std::mutex mutex;

	SampleRateConverter<2> inSrc;
	SampleRateConverter<2> outSrc;

	struct Frame {
		float samples[2];
	};
	// in device's sample rate
	RingBuffer<Frame, (1<<15)> inBuffer;
	// in rack's sample rate
	RingBuffer<Frame, (1<<15)> outBuffer;

	AudioInterface();
	~AudioInterface();
	void step();

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
};


AudioInterface::AudioInterface() {
	params.resize(NUM_PARAMS);
	inputs.resize(NUM_INPUTS);
	outputs.resize(NUM_OUTPUTS);
}

AudioInterface::~AudioInterface() {
	closeDevice();
}

void AudioInterface::step() {
	std::lock_guard<std::mutex> lock(mutex);
	if (!stream)
		return;

	// Get input and pass it through the sample rate converter
	if (numOutputs > 0) {
		Frame f;
		f.samples[0] = getf(inputs[AUDIO1_INPUT]);
		f.samples[1] = getf(inputs[AUDIO2_INPUT]);

		inSrc.setRatio(sampleRate / gRack->sampleRate);
		int inLength = 1;
		int outLength = 16;
		float buf[2*outLength];
		inSrc.process(f.samples, &inLength, buf, &outLength);
		for (int i = 0; i < outLength; i++) {
			if (inBuffer.full())
				break;
			Frame f;
			f.samples[0] = buf[2*i + 0];
			f.samples[1] = buf[2*i + 1];
			inBuffer.push(f);
		}
	}

	// Read/write stream if we have enough input
	// TODO If numOutputs == 0, call this when outBuffer.empty()
	bool streamReady = (numOutputs > 0) ? ((int)inBuffer.size() >= blockSize) : (outBuffer.empty());
	if (streamReady) {
		// printf("%d %d\n", inBuffer.size(), outBuffer.size());
		PaError err;

		// Read output from input stream
		// (for some reason, if you write the output stream before you read the input stream, PortAudio can segfault on Windows.)
		if (numInputs > 0) {
			float *buf = new float[numInputs * blockSize];
			err = Pa_ReadStream(stream, buf, blockSize);
			if (err) {
				// Ignore buffer underflows
				if (err != paInputOverflowed) {
					fprintf(stderr, "Audio input buffer underflow\n");
				}
			}

			// Pass output through sample rate converter
			outSrc.setRatio(gRack->sampleRate / sampleRate);
			int inLength = blockSize;
			int outLength = 8*blockSize;
			float *outBuf = new float[2*outLength];
			outSrc.process(buf, &inLength, outBuf, &outLength);
			// Add to output ring buffer
			for (int i = 0; i < outLength; i++) {
				if (outBuffer.full())
					break;
				Frame f;
				f.samples[0] = outBuf[2*i + 0];
				f.samples[1] = outBuf[2*i + 1];
				outBuffer.push(f);
			}
			delete[] outBuf;
			delete[] buf;
		}


		// Write input to output stream
		if (numOutputs > 0) {
			float *buf = new float[numOutputs * blockSize]();
			for (int i = 0; i < blockSize; i++) {
				assert(!inBuffer.empty());
				Frame f = inBuffer.shift();
				for (int channel = 0; channel < numOutputs; channel++) {
					buf[i * numOutputs + channel] = f.samples[channel];
				}
			}

			err = Pa_WriteStream(stream, buf, blockSize);
			if (err) {
				// Ignore buffer underflows
				if (err != paOutputUnderflowed) {
					fprintf(stderr, "Audio output buffer underflow\n");
				}
			}
			delete[] buf;
		}
	}

	// Set output
	if (!outBuffer.empty()) {
		Frame f = outBuffer.shift();
		setf(outputs[AUDIO1_OUTPUT], f.samples[0]);
		setf(outputs[AUDIO2_OUTPUT], f.samples[1]);
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

void AudioInterface::openDevice(int deviceId, float sampleRate, int blockSize) {
	closeDevice();
	std::lock_guard<std::mutex> lock(mutex);

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

		numOutputs = mini(deviceInfo->maxOutputChannels, 2);
		numInputs = mini(deviceInfo->maxInputChannels, 2);

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
			sampleRate, blockSize, paNoFlag, NULL, NULL);
		if (err) {
			fprintf(stderr, "Failed to open audio stream: %s\n", Pa_GetErrorText(err));
			return;
		}

		err = Pa_StartStream(stream);
		if (err) {
			fprintf(stderr, "Failed to start audio stream: %s\n", Pa_GetErrorText(err));
			return;
		}

		// Correct sample rate
		const PaStreamInfo *streamInfo = Pa_GetStreamInfo(stream);
		this->sampleRate = streamInfo->sampleRate;
	}

	this->deviceId = deviceId;
}

void AudioInterface::closeDevice() {
	std::lock_guard<std::mutex> lock(mutex);

	if (stream) {
		PaError err;
		err = Pa_CloseStream(stream);
		if (err) {
			// This shouldn't happen:
			fprintf(stderr, "Failed to close audio stream: %s\n", Pa_GetErrorText(err));
		}
	}

	// Reset stream settings
	stream = NULL;
	deviceId = -1;
	numOutputs = 0;
	numInputs = 0;

	// Clear buffers
	inBuffer.clear();
	outBuffer.clear();
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
		MenuOverlay *overlay = new MenuOverlay();
		Menu *menu = new Menu();
		menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));

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
		overlay->addChild(menu);
		gScene->setOverlay(overlay);
	}
	void step() {
		std::string name = audioInterface->getDeviceName(audioInterface->deviceId);
		text = name.empty() ? "(no device)" : ellipsize(name, 14);
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
		MenuOverlay *overlay = new MenuOverlay();
		Menu *menu = new Menu();
		menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));

		const float sampleRates[6] = {44100, 48000, 88200, 96000, 176400, 192000};
		for (int i = 0; i < 6; i++) {
			SampleRateItem *item = new SampleRateItem();
			item->audioInterface = audioInterface;
			item->sampleRate = sampleRates[i];
			item->text = stringf("%.0f Hz", sampleRates[i]);
			menu->pushChild(item);
		}

		overlay->addChild(menu);
		gScene->setOverlay(overlay);
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
		MenuOverlay *overlay = new MenuOverlay();
		Menu *menu = new Menu();
		menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));

		const int blockSizes[6] = {128, 256, 512, 1024, 2048, 4096};
		for (int i = 0; i < 6; i++) {
			BlockSizeItem *item = new BlockSizeItem();
			item->audioInterface = audioInterface;
			item->blockSize = blockSizes[i];
			item->text = stringf("%d", blockSizes[i]);
			menu->pushChild(item);
		}

		overlay->addChild(menu);
		gScene->setOverlay(overlay);
	}
	void step() {
		this->text = stringf("%d", audioInterface->blockSize);
	}
};


AudioInterfaceWidget::AudioInterfaceWidget() : ModuleWidget(new AudioInterface()) {
	box.size = Vec(15*8, 380);

	float margin = 5;
	float yPos = margin;

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Audio Interface";
		addChild(label);
		yPos += label->box.size.y + margin;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Audio device";
		addChild(label);
		yPos += label->box.size.y + margin;

		AudioChoice *choice = new AudioChoice();
		choice->audioInterface = dynamic_cast<AudioInterface*>(module);
		choice->box.pos = Vec(margin, yPos);
		choice->box.size.x = box.size.x - 10;
		addChild(choice);
		yPos += choice->box.size.y + 2*margin;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Sample rate";
		addChild(label);
		yPos += label->box.size.y + margin;

		SampleRateChoice *choice = new SampleRateChoice();
		choice->audioInterface = dynamic_cast<AudioInterface*>(module);
		choice->box.pos = Vec(margin, yPos);
		choice->box.size.x = box.size.x - 10;
		addChild(choice);
		yPos += choice->box.size.y + 2*margin;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Block size";
		addChild(label);
		yPos += label->box.size.y + margin;

		BlockSizeChoice *choice = new BlockSizeChoice();
		choice->audioInterface = dynamic_cast<AudioInterface*>(module);
		choice->box.pos = Vec(margin, yPos);
		choice->box.size.x = box.size.x - 10;
		addChild(choice);
		yPos += choice->box.size.y + 2*margin;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Outputs";
		addChild(label);
		yPos += label->box.size.y + margin;
	}

	yPos += 5;
	addInput(createInput(Vec(25, yPos), module, AudioInterface::AUDIO1_INPUT));
	addInput(createInput(Vec(75, yPos), module, AudioInterface::AUDIO2_INPUT));
	yPos += 35 + margin;

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Inputs";
		addChild(label);
		yPos += label->box.size.y + margin;
	}

	yPos += 5;
	addOutput(createOutput(Vec(25, yPos), module, AudioInterface::AUDIO1_OUTPUT));
	addOutput(createOutput(Vec(75, yPos), module, AudioInterface::AUDIO2_OUTPUT));
	yPos += 35 + margin;
}

void AudioInterfaceWidget::draw(NVGcontext *vg) {
	bndBackground(vg, box.pos.x, box.pos.y, box.size.x, box.size.y);
	ModuleWidget::draw(vg);
}
