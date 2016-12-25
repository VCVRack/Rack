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
	int numFrames;
	float outputBuffer[1<<14] = {};
	float inputBuffer[1<<14] = {};
	int bufferFrame;
	// Used because the GUI thread and Rack thread can both interact with this class
	std::mutex mutex;

	AudioInterface();
	~AudioInterface();
	void step();

	int getDeviceCount();
	std::string getDeviceName(int deviceId);
	// Use -1 as the deviceId to close the current device
	void openDevice(int deviceId);
};


AudioInterface::AudioInterface() {
	params.resize(NUM_PARAMS);
	inputs.resize(NUM_INPUTS);
	outputs.resize(NUM_OUTPUTS);
}

AudioInterface::~AudioInterface() {
	openDevice(-1);
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

	if (++bufferFrame >= numFrames) {
		bufferFrame = 0;
		PaError err;

		// Input
		// (for some reason, if you write the output stream before you read the input stream, PortAudio can segfault on Windows.)
		err = Pa_ReadStream(stream, inputBuffer, numFrames);
		if (err) {
			// Ignore buffer underflows
			if (err != paInputOverflowed) {
				printf("Audio input buffer underflow\n");
			}
		}

		// Output
		err = Pa_WriteStream(stream, outputBuffer, numFrames);
		if (err) {
			// Ignore buffer underflows
			if (err != paOutputUnderflowed) {
				printf("Audio output buffer underflow\n");
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
	char name[1024];
	snprintf(name, sizeof(name), "%s: %s (%d in, %d out)", apiInfo->name, info->name, info->maxInputChannels, info->maxOutputChannels);
	return name;
}

void AudioInterface::openDevice(int deviceId) {
	std::lock_guard<std::mutex> lock(mutex);

	// Close existing device
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

	numFrames = 256;
	bufferFrame = 0;
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

		err = Pa_OpenStream(&stream, &inputParameters, &outputParameters, SAMPLE_RATE, numFrames, paNoFlag, NULL, NULL);
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


struct AudioItem : MenuItem {
	AudioInterface *audioInterface;
	int deviceId;
	void onAction() {
		audioInterface->openDevice(deviceId);
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
		AudioChoice *audioChoice = new AudioChoice();
		audioChoice->audioInterface = dynamic_cast<AudioInterface*>(module);
		audioChoice->text = "Audio device";
		audioChoice->box.pos = Vec(margin, yPos);
		audioChoice->box.size.x = box.size.x - 10;
		addChild(audioChoice);
		yPos += audioChoice->box.size.y + 2*margin;
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
