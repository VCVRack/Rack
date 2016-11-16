#include "core.hpp"
#include <assert.h>
#include <mutex>
#include <condition_variable>
#include <rtaudio/RtAudio.h>


#define AUDIO_BUFFER_SIZE 16384

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

	float audio1Buffer[AUDIO_BUFFER_SIZE] = {};
	float audio2Buffer[AUDIO_BUFFER_SIZE] = {};
	// Current frame for step(), called by the rack thread
	long bufferFrame = 0;
	// Current frame for processAudio(), called by audio thread
	long audioFrame = 0;
	long audioFrameNeeded = -1;
	RtAudio audio;
	// The audio thread should wait on the rack thread until the buffer has enough samples
	std::mutex mutex;
	std::condition_variable cv;
	bool running;

	AudioInterface();
	~AudioInterface();
	void step();

	void openDevice(int deviceId);
	void closeDevice();
	// Blocks until the buffer has enough samples
	void processAudio(float *outputBuffer, int frameCount);
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
	int i = bufferFrame % AUDIO_BUFFER_SIZE;
	audio1Buffer[i] = getf(inputs[AUDIO1_INPUT]);
	audio2Buffer[i] = getf(inputs[AUDIO2_INPUT]);
	// std::unique_lock<std::mutex> lock(mutex);
	bufferFrame++;
	if (bufferFrame == audioFrameNeeded) {
	// lock.unlock();
		cv.notify_all();
	}
}

int audioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData) {
	AudioInterface *that = (AudioInterface*) userData;
	that->processAudio((float*) outputBuffer, nBufferFrames);
	return 0;
}

void AudioInterface::openDevice(int deviceId) {
	assert(!audio.isStreamOpen());
	if (deviceId < 0) {
		deviceId = audio.getDefaultOutputDevice();
	}

	RtAudio::StreamParameters streamParams;
	streamParams.deviceId = deviceId;
	streamParams.nChannels = 2;
	streamParams.firstChannel = 0;
	unsigned int sampleRate = SAMPLE_RATE;
	unsigned int bufferFrames = 512;

	audioFrame = -1;
	running = true;

	try {
		audio.openStream(&streamParams, NULL, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &audioCallback, this);
		audio.startStream();
	}
	catch (RtAudioError &e) {
		printf("Could not open audio stream: %s\n", e.what());
	}
}

void AudioInterface::closeDevice() {
	if (!audio.isStreamOpen()) {
		return;
	}
	{
		std::unique_lock<std::mutex> lock(mutex);
		running = false;
	}
	cv.notify_all();

	try {
		// Blocks until stream thread exits
		audio.stopStream();
		audio.closeStream();
	}
	catch (RtAudioError &e) {
		printf("Could not close audio stream: %s\n", e.what());
	}
}

void AudioInterface::processAudio(float *outputBuffer, int frameCount) {
	std::unique_lock<std::mutex> lock(mutex);
	if (audioFrame < 0) {
		// This audio thread is new. Reset the frame positions
		audioFrame = rackGetFrame();
		bufferFrame = audioFrame;
	}
	audioFrameNeeded = audioFrame + frameCount;
	rackRequestFrame(audioFrameNeeded);
	// Wait for needed frames
	while (running && bufferFrame < audioFrameNeeded) {
		cv.wait(lock);
	}
	// Copy values from internal buffer to audio buffer, while holding the mutex just in case our audio buffer wraps around
	for (int frame = 0; frame < frameCount; frame++) {
		int i = audioFrame % AUDIO_BUFFER_SIZE;
		outputBuffer[2*frame + 0] = audio1Buffer[i] / 5.0;
		outputBuffer[2*frame + 1] = audio2Buffer[i] / 5.0;
		audioFrame++;
	}
}


struct AudioItem : MenuItem {
	AudioInterface *audioInterface;
	int deviceId;
	void onAction() {
		audioInterface->closeDevice();
		audioInterface->openDevice(deviceId);
	}
};

struct AudioChoice : ChoiceButton {
	AudioInterface *audioInterface;
	void onAction() {
		MenuOverlay *overlay = new MenuOverlay();
		Menu *menu = new Menu();
		menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));

		int deviceCount = audioInterface->audio.getDeviceCount();
		if (deviceCount == 0) {
			MenuLabel *label = new MenuLabel();
			label->text = "No audio devices";
			menu->pushChild(label);
		}
		for (int deviceId = 0; deviceId < deviceCount; deviceId++) {
			RtAudio::DeviceInfo info = audioInterface->audio.getDeviceInfo(deviceId);
			if (!info.probed)
				continue;

			char text[100];
			snprintf(text, 100, "%s (%d in, %d out)", info.name.c_str(), info.inputChannels, info.outputChannels);

			AudioItem *audioItem = new AudioItem();
			audioItem->audioInterface = audioInterface;
			audioItem->deviceId = deviceId;
			audioItem->text = text;
			menu->pushChild(audioItem);
		}
		overlay->addChild(menu);
		gScene->addChild(overlay);
	}
};


AudioInterfaceWidget::AudioInterfaceWidget() : ModuleWidget(new AudioInterface()) {
	box.size = Vec(15*8, 380);
	inputs.resize(AudioInterface::NUM_INPUTS);

	createInputPort(this, AudioInterface::AUDIO1_INPUT, Vec(15, 100));
	createInputPort(this, AudioInterface::AUDIO2_INPUT, Vec(70, 100));

	AudioChoice *audioChoice = new AudioChoice();
	audioChoice->audioInterface = dynamic_cast<AudioInterface*>(module);
	audioChoice->text = "Audio Interface";
	audioChoice->box.pos = Vec(0, 0);
	audioChoice->box.size.x = box.size.x;
	addChild(audioChoice);
}

void AudioInterfaceWidget::draw(NVGcontext *vg) {
	bndBackground(vg, box.pos.x, box.pos.y, box.size.x, box.size.y);
	ModuleWidget::draw(vg);
}
