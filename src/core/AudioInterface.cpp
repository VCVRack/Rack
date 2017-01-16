#include <assert.h>
#include <mutex>
#include <portaudio.h>
#include "core.hpp"

using namespace rack;


void audioInit() {
	PaError err = Pa_Initialize();
	if (err) {
		printf("Failed to initialize PortAudio: %s\n", Pa_GetErrorText(err));
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
		AUDIO3_INPUT,
		AUDIO4_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO1_OUTPUT,
		AUDIO2_OUTPUT,
		AUDIO3_OUTPUT,
		AUDIO4_OUTPUT,
		NUM_OUTPUTS
	};

	PaStream *stream = NULL;
	int numOutputs;
	int numInputs;
	int bufferFrame;
	float outputBuffer[1<<14] = {};
	float inputBuffer[1<<14] = {};
	// Used because the GUI thread and Rack thread can both interact with this class
	std::mutex mutex;

	// Set these and call openDevice()
	int deviceId = -1;
	float sampleRate = 44100.0;
	int blockSize = 256;

	AudioInterface();
	~AudioInterface();
	void step();

	int getDeviceCount();
	std::string getDeviceName(int deviceId);
	void openDevice();
	void closeDevice();
};


AudioInterface::AudioInterface() {
	params.resize(NUM_PARAMS);
	inputs.resize(NUM_INPUTS);
	outputs.resize(NUM_OUTPUTS);
	closeDevice();
}

AudioInterface::~AudioInterface() {
	closeDevice();
}

void AudioInterface::step() {
	std::lock_guard<std::mutex> lock(mutex);

	if (!stream) {
		setf(inputs[AUDIO1_OUTPUT], 0.0);
		setf(inputs[AUDIO2_OUTPUT], 0.0);
		setf(inputs[AUDIO3_OUTPUT], 0.0);
		setf(inputs[AUDIO4_OUTPUT], 0.0);
		return;
	}

	// Input ports -> Output buffer
	for (int i = 0; i < numOutputs; i++) {
		outputBuffer[numOutputs * bufferFrame + i] = getf(inputs[i]) / 5.0;
	}
	// Input buffer -> Output ports
	for (int i = 0; i < numInputs; i++) {
		setf(outputs[i], inputBuffer[numOutputs * bufferFrame + i] * 5.0);
	}

	if (++bufferFrame >= blockSize) {
		bufferFrame = 0;
		PaError err;

		// Input
		// (for some reason, if you write the output stream before you read the input stream, PortAudio can segfault on Windows.)
		if (numInputs > 0) {
			err = Pa_ReadStream(stream, inputBuffer, blockSize);
			if (err) {
				// Ignore buffer underflows
				if (err != paInputOverflowed) {
					printf("Audio input buffer underflow\n");
				}
			}
		}

		// Output
		if (numOutputs > 0) {
			err = Pa_WriteStream(stream, outputBuffer, blockSize);
			if (err) {
				// Ignore buffer underflows
				if (err != paOutputUnderflowed) {
					printf("Audio output buffer underflow\n");
				}
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

void AudioInterface::openDevice() {
	closeDevice();
	std::lock_guard<std::mutex> lock(mutex);

	// Open new device
	if (deviceId >= 0) {
		PaError err;
		const PaDeviceInfo *info = Pa_GetDeviceInfo(deviceId);
		if (!info) {
			printf("Failed to query audio device\n");
			return;
		}

		numOutputs = mini(info->maxOutputChannels, 4);
		numInputs = mini(info->maxInputChannels, 4);

		PaStreamParameters outputParameters;
		outputParameters.device = deviceId;
		outputParameters.channelCount = numOutputs;
		outputParameters.sampleFormat = paFloat32;
		outputParameters.suggestedLatency = info->defaultLowOutputLatency;
		outputParameters.hostApiSpecificStreamInfo = NULL;

		PaStreamParameters inputParameters;
		inputParameters.device = deviceId;
		inputParameters.channelCount = numInputs;
		inputParameters.sampleFormat = paFloat32;
		inputParameters.suggestedLatency = info->defaultLowInputLatency;
		inputParameters.hostApiSpecificStreamInfo = NULL;

		// Don't use stream parameters if 0 input or output channels
		err = Pa_OpenStream(&stream,
			numInputs == 0 ? NULL : &outputParameters,
			numOutputs == 0 ? NULL : &inputParameters,
			sampleRate, blockSize, paNoFlag, NULL, NULL);
		if (err) {
			printf("Failed to open audio stream: %s\n", Pa_GetErrorText(err));
			return;
		}

		err = Pa_StartStream(stream);
		if (err) {
			printf("Failed to start audio stream: %s\n", Pa_GetErrorText(err));
			return;
		}
	}
}

void AudioInterface::closeDevice() {
	std::lock_guard<std::mutex> lock(mutex);

	if (stream) {
		PaError err;
		err = Pa_CloseStream(stream);
		if (err) {
			// This shouldn't happen:
			printf("Failed to close audio stream: %s\n", Pa_GetErrorText(err));
		}
		stream = NULL;
	}
	numOutputs = 0;
	numInputs = 0;
	bufferFrame = 0;
}


struct AudioItem : MenuItem {
	AudioInterface *audioInterface;
	int deviceId;
	void onAction() {
		audioInterface->deviceId = deviceId;
		audioInterface->openDevice();
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
		if (name.empty())
			text = "(no device)";
		else
			text = name;
	}
};


struct SampleRateItem : MenuItem {
	AudioInterface *audioInterface;
	float sampleRate;
	void onAction() {
		audioInterface->sampleRate = sampleRate;
		audioInterface->openDevice();
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
			item->text = stringf("%.0f", sampleRates[i]);
			menu->pushChild(item);
		}

		overlay->addChild(menu);
		gScene->setOverlay(overlay);
	}
	void step() {
		this->text = stringf("%.0f", audioInterface->sampleRate);
	}
};


struct BlockSizeItem : MenuItem {
	AudioInterface *audioInterface;
	int blockSize;
	void onAction() {
		audioInterface->blockSize = blockSize;
		audioInterface->openDevice();
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

	addInput(createInput(Vec(25, yPos), module, AudioInterface::AUDIO3_INPUT));
	addInput(createInput(Vec(75, yPos), module, AudioInterface::AUDIO4_INPUT));
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

	addOutput(createOutput(Vec(25, yPos), module, AudioInterface::AUDIO3_OUTPUT));
	addOutput(createOutput(Vec(75, yPos), module, AudioInterface::AUDIO4_OUTPUT));
	yPos += 35 + margin;
}

void AudioInterfaceWidget::draw(NVGcontext *vg) {
	bndBackground(vg, box.pos.x, box.pos.y, box.size.x, box.size.y);
	ModuleWidget::draw(vg);
}
