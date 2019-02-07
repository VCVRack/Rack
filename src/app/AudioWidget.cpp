#include "app/AudioWidget.hpp"
#include "audio.hpp"
#include "helpers.hpp"


namespace rack {
namespace app {


struct AudioDriverItem : ui::MenuItem {
	audio::IO *audioIO;
	int driver;
	void onAction(const event::Action &e) override {
		audioIO->setDriver(driver);
	}
};

struct AudioDriverChoice : LedDisplayChoice {
	audio::IO *audioIO;
	void onAction(const event::Action &e) override {
		if (!audioIO)
			return;

		ui::Menu *menu = createMenu();
		menu->addChild(createMenuLabel("Audio driver"));
		for (int driver : audioIO->getDrivers()) {
			AudioDriverItem *item = new AudioDriverItem;
			item->audioIO = audioIO;
			item->driver = driver;
			item->text = audioIO->getDriverName(driver);
			item->rightText = CHECKMARK(item->driver == audioIO->driver);
			menu->addChild(item);
		}
	}
	void step() override {
		if (audioIO)
			text = audioIO->getDriverName(audioIO->driver);
		else
			text = "";
	}
};


struct AudioDeviceItem : ui::MenuItem {
	audio::IO *audioIO;
	int device;
	int offset;
	void onAction(const event::Action &e) override {
		audioIO->setDevice(device, offset);
	}
};

struct AudioDeviceChoice : LedDisplayChoice {
	audio::IO *audioIO;
	/** Prevents devices with a ridiculous number of channels from being displayed */
	int maxTotalChannels = 128;

	void onAction(const event::Action &e) override {
		if (!audioIO)
			return;

		ui::Menu *menu = createMenu();
		menu->addChild(createMenuLabel("Audio device"));
		int deviceCount = audioIO->getDeviceCount();
		{
			AudioDeviceItem *item = new AudioDeviceItem;
			item->audioIO = audioIO;
			item->device = -1;
			item->text = "(No device)";
			item->rightText = CHECKMARK(item->device == audioIO->device);
			menu->addChild(item);
		}
		for (int device = 0; device < deviceCount; device++) {
			int channels = std::min(maxTotalChannels, audioIO->getDeviceChannels(device));
			for (int offset = 0; offset < channels; offset += audioIO->maxChannels) {
				AudioDeviceItem *item = new AudioDeviceItem;
				item->audioIO = audioIO;
				item->device = device;
				item->offset = offset;
				item->text = audioIO->getDeviceDetail(device, offset);
				item->rightText = CHECKMARK(item->device == audioIO->device && item->offset == audioIO->offset);
				menu->addChild(item);
			}
		}
	}
	void step() override {
		if (!audioIO) {
			text = "";
			return;
		}
		text = audioIO->getDeviceDetail(audioIO->device, audioIO->offset);
		if (text.empty()) {
			text = "(No device)";
			color.a = 0.5f;
		}
		else {
			color.a = 1.f;
		}
	}
};


struct AudioSampleRateItem : ui::MenuItem {
	audio::IO *audioIO;
	int sampleRate;
	void onAction(const event::Action &e) override {
		audioIO->setSampleRate(sampleRate);
	}
};

struct AudioSampleRateChoice : LedDisplayChoice {
	audio::IO *audioIO;
	void onAction(const event::Action &e) override {
		if (!audioIO)
			return;

		ui::Menu *menu = createMenu();
		menu->addChild(createMenuLabel("Sample rate"));
		std::vector<int> sampleRates = audioIO->getSampleRates();
		if (sampleRates.empty()) {
			menu->addChild(createMenuLabel("(Locked by device)"));
		}
		for (int sampleRate : sampleRates) {
			AudioSampleRateItem *item = new AudioSampleRateItem;
			item->audioIO = audioIO;
			item->sampleRate = sampleRate;
			item->text = string::f("%d Hz", sampleRate);
			item->rightText = CHECKMARK(item->sampleRate == audioIO->sampleRate);
			menu->addChild(item);
		}
	}
	void step() override {
		if (audioIO)
			text = string::f("%g kHz", audioIO->sampleRate / 1000.f);
		else
			text = "";
	}
};


struct AudioBlockSizeItem : ui::MenuItem {
	audio::IO *audioIO;
	int blockSize;
	void onAction(const event::Action &e) override {
		audioIO->setBlockSize(blockSize);
	}
};

struct AudioBlockSizeChoice : LedDisplayChoice {
	audio::IO *audioIO;
	void onAction(const event::Action &e) override {
		if (!audioIO)
			return;

		ui::Menu *menu = createMenu();
		menu->addChild(createMenuLabel("Block size"));
		std::vector<int> blockSizes = audioIO->getBlockSizes();
		if (blockSizes.empty()) {
			menu->addChild(createMenuLabel("(Locked by device)"));
		}
		for (int blockSize : blockSizes) {
			AudioBlockSizeItem *item = new AudioBlockSizeItem;
			item->audioIO = audioIO;
			item->blockSize = blockSize;
			float latency = (float) blockSize / audioIO->sampleRate * 1000.0;
			item->text = string::f("%d (%.1f ms)", blockSize, latency);
			item->rightText = CHECKMARK(item->blockSize == audioIO->blockSize);
			menu->addChild(item);
		}
	}
	void step() override {
		if (audioIO)
			text = string::f("%d", audioIO->blockSize);
		else
			text = "";
	}
};


void AudioWidget::setAudioIO(audio::IO *audioIO) {
	clearChildren();

	math::Vec pos;

	AudioDriverChoice *driverChoice = createWidget<AudioDriverChoice>(pos);
	driverChoice->box.size.x = box.size.x;
	driverChoice->audioIO = audioIO;
	addChild(driverChoice);
	pos = driverChoice->box.getBottomLeft();
	this->driverChoice = driverChoice;

	this->driverSeparator = createWidget<LedDisplaySeparator>(pos);
	this->driverSeparator->box.size.x = box.size.x;
	addChild(this->driverSeparator);

	AudioDeviceChoice *deviceChoice = createWidget<AudioDeviceChoice>(pos);
	deviceChoice->box.size.x = box.size.x;
	deviceChoice->audioIO = audioIO;
	addChild(deviceChoice);
	pos = deviceChoice->box.getBottomLeft();
	this->deviceChoice = deviceChoice;

	this->deviceSeparator = createWidget<LedDisplaySeparator>(pos);
	this->deviceSeparator->box.size.x = box.size.x;
	addChild(this->deviceSeparator);

	AudioSampleRateChoice *sampleRateChoice = createWidget<AudioSampleRateChoice>(pos);
	sampleRateChoice->box.size.x = box.size.x / 2;
	sampleRateChoice->audioIO = audioIO;
	addChild(sampleRateChoice);
	this->sampleRateChoice = sampleRateChoice;

	this->sampleRateSeparator = createWidget<LedDisplaySeparator>(pos);
	this->sampleRateSeparator->box.pos.x = box.size.x / 2;
	this->sampleRateSeparator->box.size.y = this->sampleRateChoice->box.size.y;
	addChild(this->sampleRateSeparator);

	AudioBlockSizeChoice *bufferSizeChoice = createWidget<AudioBlockSizeChoice>(pos);
	bufferSizeChoice->box.pos.x = box.size.x / 2;
	bufferSizeChoice->box.size.x = box.size.x / 2;
	bufferSizeChoice->audioIO = audioIO;
	addChild(bufferSizeChoice);
	this->bufferSizeChoice = bufferSizeChoice;
}


} // namespace app
} // namespace rack
