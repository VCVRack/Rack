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
		for (int driver : audioWidget->audioIO->listDrivers()) {
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
	void onAction(EventAction &e) override {
		audioIO->device = device;
		audioIO->openStream();
	}
};

struct AudioDeviceChoice : LedDisplayChoice {
	AudioWidget *audioWidget;
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
			AudioDeviceItem *item = new AudioDeviceItem();
			item->audioIO = audioWidget->audioIO;
			item->device = device;
			item->text = audioWidget->audioIO->getDeviceDetail(device);
			item->rightText = CHECKMARK(item->device == audioWidget->audioIO->device);
			menu->addChild(item);
		}
	}
	void step() override {
		text = audioWidget->audioIO->getDeviceDetail(audioWidget->audioIO->device);
		if (text.empty()) {
			text = "(No device)";
			color.a = 0.5f;
		}
		else {
			color.a = 1.f;
		}
		text = ellipsize(text, 18);
	}
};


struct AudioSampleRateItem : MenuItem {
	AudioIO *audioIO;
	int sampleRate;
	void onAction(EventAction &e) override {
		audioIO->sampleRate = sampleRate;
		audioIO->openStream();
	}
};

struct AudioSampleRateChoice : LedDisplayChoice {
	AudioWidget *audioWidget;
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Sample rate"));
		for (int sampleRate : audioWidget->audioIO->listSampleRates()) {
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
		audioIO->blockSize = blockSize;
		audioIO->openStream();
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


struct AudioWidget::Internal {
	LedDisplayChoice *driverChoice;
	LedDisplaySeparator *driverSeparator;
	LedDisplayChoice *deviceChoice;
	LedDisplaySeparator *deviceSeparator;
	LedDisplayChoice *sampleRateChoice;
	LedDisplaySeparator *sampleRateSeparator;
	LedDisplayChoice *bufferSizeChoice;
};

AudioWidget::AudioWidget() {
	internal = new Internal();
	box.size = mm2px(Vec(44, 28));

	Vec pos = Vec();

	AudioDriverChoice *driverChoice = Widget::create<AudioDriverChoice>(pos);
	driverChoice->audioWidget = this;
	addChild(driverChoice);
	pos = driverChoice->box.getBottomLeft();
	internal->driverChoice = driverChoice;

	internal->driverSeparator = Widget::create<LedDisplaySeparator>(pos);
	addChild(internal->driverSeparator);

	AudioDeviceChoice *deviceChoice = Widget::create<AudioDeviceChoice>(pos);
	deviceChoice->audioWidget = this;
	addChild(deviceChoice);
	pos = deviceChoice->box.getBottomLeft();
	internal->deviceChoice = deviceChoice;

	internal->deviceSeparator = Widget::create<LedDisplaySeparator>(pos);
	addChild(internal->deviceSeparator);

	AudioSampleRateChoice *sampleRateChoice = Widget::create<AudioSampleRateChoice>(pos);
	sampleRateChoice->audioWidget = this;
	addChild(sampleRateChoice);
	internal->sampleRateChoice = sampleRateChoice;

	internal->sampleRateSeparator = Widget::create<LedDisplaySeparator>(pos);
	internal->sampleRateSeparator->box.size.y = internal->sampleRateChoice->box.size.y;
	addChild(internal->sampleRateSeparator);

	AudioBlockSizeChoice *bufferSizeChoice = Widget::create<AudioBlockSizeChoice>(pos);
	bufferSizeChoice->audioWidget = this;
	addChild(bufferSizeChoice);
	internal->bufferSizeChoice = bufferSizeChoice;
}

AudioWidget::~AudioWidget() {
	delete internal;
}

void AudioWidget::step() {
	internal->driverChoice->box.size.x = box.size.x;
	internal->driverSeparator->box.size.x = box.size.x;
	internal->deviceChoice->box.size.x = box.size.x;
	internal->deviceSeparator->box.size.x = box.size.x;
	internal->sampleRateChoice->box.size.x = box.size.x / 2;
	internal->sampleRateSeparator->box.pos.x = box.size.x / 2;
	internal->bufferSizeChoice->box.pos.x = box.size.x / 2;
	internal->bufferSizeChoice->box.size.x = box.size.x / 2;
	LedDisplay::step();
}


} // namespace rack
