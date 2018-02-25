#include "app.hpp"
#include "audio.hpp"


namespace rack {


struct AudioDriverItem : MenuItem {
	AudioIO *audioIO;
	int driver;
	void onAction(EventAction &e) override {
		audioIO->setDriver(driver);
	}
};

struct AudioDriverChoice : LedDisplayChoice {
	AudioWidget *audioWidget;
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Audio driver"));
		for (int driver : audioWidget->audioIO->getDrivers()) {
			AudioDriverItem *item = new AudioDriverItem();
			item->audioIO = audioWidget->audioIO;
			item->driver = driver;
			item->text = audioWidget->audioIO->getDriverName(driver);
			item->rightText = CHECKMARK(item->driver == audioWidget->audioIO->driver);
			menu->addChild(item);
		}
	}
	void step() override {
		text = audioWidget->audioIO->getDriverName(audioWidget->audioIO->driver);
	}
};


struct AudioDeviceItem : MenuItem {
	AudioIO *audioIO;
	int device;
	int offset;
	void onAction(EventAction &e) override {
		audioIO->setDevice(device, offset);
	}
};

struct AudioDeviceChoice : LedDisplayChoice {
	AudioWidget *audioWidget;
	/** Prevents devices with a ridiculous number of channels from being displayed */
	int maxTotalChannels = 128;

	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Audio device"));
		int deviceCount = audioWidget->audioIO->getDeviceCount();
		{
			AudioDeviceItem *item = new AudioDeviceItem();
			item->audioIO = audioWidget->audioIO;
			item->device = -1;
			item->text = "(No device)";
			item->rightText = CHECKMARK(item->device == audioWidget->audioIO->device);
			menu->addChild(item);
		}
		for (int device = 0; device < deviceCount; device++) {
			int channels = min(maxTotalChannels, audioWidget->audioIO->getDeviceChannels(device));
			for (int offset = 0; offset < channels; offset += audioWidget->audioIO->maxChannels) {
				AudioDeviceItem *item = new AudioDeviceItem();
				item->audioIO = audioWidget->audioIO;
				item->device = device;
				item->offset = offset;
				item->text = audioWidget->audioIO->getDeviceDetail(device, offset);
				item->rightText = CHECKMARK(item->device == audioWidget->audioIO->device && item->offset == audioWidget->audioIO->offset);
				menu->addChild(item);
			}
		}
	}
	void step() override {
		text = audioWidget->audioIO->getDeviceDetail(audioWidget->audioIO->device, audioWidget->audioIO->offset);
		if (text.empty()) {
			text = "(No device)";
			color.a = 0.5f;
		}
		else {
			color.a = 1.f;
		}
	}
};


struct AudioSampleRateItem : MenuItem {
	AudioIO *audioIO;
	int sampleRate;
	void onAction(EventAction &e) override {
		audioIO->setSampleRate(sampleRate);
	}
};

struct AudioSampleRateChoice : LedDisplayChoice {
	AudioWidget *audioWidget;
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Sample rate"));
		for (int sampleRate : audioWidget->audioIO->getSampleRates()) {
			AudioSampleRateItem *item = new AudioSampleRateItem();
			item->audioIO = audioWidget->audioIO;
			item->sampleRate = sampleRate;
			item->text = stringf("%d Hz", sampleRate);
			item->rightText = CHECKMARK(item->sampleRate == audioWidget->audioIO->sampleRate);
			menu->addChild(item);
		}
	}
	void step() override {
		text = stringf("%g kHz", audioWidget->audioIO->sampleRate / 1000.f);
	}
};


struct AudioBlockSizeItem : MenuItem {
	AudioIO *audioIO;
	int blockSize;
	void onAction(EventAction &e) override {
		audioIO->setBlockSize(blockSize);
	}
};

struct AudioBlockSizeChoice : LedDisplayChoice {
	AudioWidget *audioWidget;
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Block size"));
		std::vector<int> blockSizes = {64, 128, 256, 512, 1024, 2048, 4096};
		for (int blockSize : blockSizes) {
			AudioBlockSizeItem *item = new AudioBlockSizeItem();
			item->audioIO = audioWidget->audioIO;
			item->blockSize = blockSize;
			float latency = (float) blockSize / audioWidget->audioIO->sampleRate * 1000.0;
			item->text = stringf("%d (%.1f ms)", blockSize, latency);
			item->rightText = CHECKMARK(item->blockSize == audioWidget->audioIO->blockSize);
			menu->addChild(item);
		}
	}
	void step() override {
		text = stringf("%d", audioWidget->audioIO->blockSize);
	}
};


AudioWidget::AudioWidget() {
	box.size = mm2px(Vec(44, 28));

	Vec pos = Vec();

	AudioDriverChoice *driverChoice = Widget::create<AudioDriverChoice>(pos);
	driverChoice->audioWidget = this;
	addChild(driverChoice);
	pos = driverChoice->box.getBottomLeft();
	this->driverChoice = driverChoice;

	this->driverSeparator = Widget::create<LedDisplaySeparator>(pos);
	addChild(this->driverSeparator);

	AudioDeviceChoice *deviceChoice = Widget::create<AudioDeviceChoice>(pos);
	deviceChoice->audioWidget = this;
	addChild(deviceChoice);
	pos = deviceChoice->box.getBottomLeft();
	this->deviceChoice = deviceChoice;

	this->deviceSeparator = Widget::create<LedDisplaySeparator>(pos);
	addChild(this->deviceSeparator);

	AudioSampleRateChoice *sampleRateChoice = Widget::create<AudioSampleRateChoice>(pos);
	sampleRateChoice->audioWidget = this;
	addChild(sampleRateChoice);
	this->sampleRateChoice = sampleRateChoice;

	this->sampleRateSeparator = Widget::create<LedDisplaySeparator>(pos);
	this->sampleRateSeparator->box.size.y = this->sampleRateChoice->box.size.y;
	addChild(this->sampleRateSeparator);

	AudioBlockSizeChoice *bufferSizeChoice = Widget::create<AudioBlockSizeChoice>(pos);
	bufferSizeChoice->audioWidget = this;
	addChild(bufferSizeChoice);
	this->bufferSizeChoice = bufferSizeChoice;
}

void AudioWidget::step() {
	this->driverChoice->box.size.x = box.size.x;
	this->driverSeparator->box.size.x = box.size.x;
	this->deviceChoice->box.size.x = box.size.x;
	this->deviceSeparator->box.size.x = box.size.x;
	this->sampleRateChoice->box.size.x = box.size.x / 2;
	this->sampleRateSeparator->box.pos.x = box.size.x / 2;
	this->bufferSizeChoice->box.pos.x = box.size.x / 2;
	this->bufferSizeChoice->box.size.x = box.size.x / 2;
	LedDisplay::step();
}


} // namespace rack
