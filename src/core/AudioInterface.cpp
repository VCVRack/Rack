#include <assert.h>
#include <mutex>
#include <portaudio.h>
#include "core.hpp"


using namespace rack;

static bool audioInitialized = false;


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
		NUM_OUTPUTS
	};

	PaStream *stream = NULL;
	float *buffer;
	int bufferFrames;
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

	buffer = new float[1<<14];

	// Lazy initialize PulseAudio
	if (!audioInitialized) {
		PaError err = Pa_Initialize();
		if (err) {
			printf("Failed to initialize PortAudio: %s\n", Pa_GetErrorText(err));
			return;
		}
		audioInitialized = true;
	}
}

AudioInterface::~AudioInterface() {
	openDevice(-1);
	delete[] buffer;
}

void AudioInterface::step() {
	std::lock_guard<std::mutex> lock(mutex);

	if (!stream)
		return;

	buffer[2*bufferFrame + 0] = getf(inputs[AUDIO1_INPUT]) / 5.0;
	buffer[2*bufferFrame + 1] = getf(inputs[AUDIO2_INPUT]) / 5.0;

	if (++bufferFrame >= bufferFrames) {
		bufferFrame = 0;
		PaError err = Pa_WriteStream(stream, buffer, bufferFrames);
		if (err) {
			// Ignore buffer underflows
			if (err != paOutputUnderflowed) {
				printf("Failed to write buffer to audio stream: %s\n", Pa_GetErrorText(err));
				return;
			}
		}
	}
}

int AudioInterface::getDeviceCount() {
	return Pa_GetDeviceCount();
}

std::string AudioInterface::getDeviceName(int deviceId) {
	const PaDeviceInfo *info = Pa_GetDeviceInfo(deviceId);
	return info ? std::string(info->name) : "";
}

void AudioInterface::openDevice(int deviceId) {
	PaError err;
	std::lock_guard<std::mutex> lock(mutex);

	// Close existing device
	if (stream) {
		err = Pa_CloseStream(stream);
		if (err) {
			printf("Failed to close audio stream: %s\n", Pa_GetErrorText(err));
		}
		stream = NULL;
	}

	// Open new device
	bufferFrames = 256;
	bufferFrame = 0;
	if (deviceId >= 0) {
		const PaDeviceInfo *info = Pa_GetDeviceInfo(deviceId);
		if (!info) {
			printf("Failed to query audio device\n");
			return;
		}

		PaStreamParameters outputParameters;
		outputParameters.device = deviceId;
		outputParameters.channelCount = 2;
		outputParameters.sampleFormat = paFloat32;
		outputParameters.suggestedLatency = info->defaultLowOutputLatency;
		outputParameters.hostApiSpecificStreamInfo = NULL;

		err = Pa_OpenStream(&stream, NULL, &outputParameters, SAMPLE_RATE, bufferFrames, paNoFlag, NULL, NULL);
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
		if (deviceCount == 0) {
			MenuLabel *label = new MenuLabel();
			label->text = "No audio devices";
			menu->pushChild(label);
		}
		for (int deviceId = 0; deviceId < deviceCount; deviceId++) {
			AudioItem *audioItem = new AudioItem();
			audioItem->audioInterface = audioInterface;
			audioItem->deviceId = deviceId;
			std::string text = audioInterface->getDeviceName(deviceId);
			audioItem->text = text;
			menu->pushChild(audioItem);
		}
		overlay->addChild(menu);
		gScene->setOverlay(overlay);
	}
};


AudioInterfaceWidget::AudioInterfaceWidget() : ModuleWidget(new AudioInterface()) {
	box.size = Vec(15*8, 380);

	addInput(createInput(Vec(15, 100), module, AudioInterface::AUDIO1_INPUT));
	addInput(createInput(Vec(70, 100), module, AudioInterface::AUDIO2_INPUT));

	{
		AudioChoice *audioChoice = new AudioChoice();
		audioChoice->audioInterface = dynamic_cast<AudioInterface*>(module);
		audioChoice->text = "Audio Interface";
		audioChoice->box.pos = Vec(0, 0);
		audioChoice->box.size.x = box.size.x;
		addChild(audioChoice);
	}
}

void AudioInterfaceWidget::draw(NVGcontext *vg) {
	bndBackground(vg, box.pos.x, box.pos.y, box.size.x, box.size.y);
	ModuleWidget::draw(vg);
}
