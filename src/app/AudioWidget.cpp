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


struct AudioDeviceItem : MenuItem {
	AudioIO *audioIO;
	int device;
	void onAction(EventAction &e) override {
		audioIO->device = device;
		audioIO->openStream();
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


struct AudioBlockSizeItem : MenuItem {
	AudioIO *audioIO;
	int blockSize;
	void onAction(EventAction &e) override {
		audioIO->blockSize = blockSize;
		audioIO->openStream();
	}
};


void AudioWidget::onMouseDown(EventMouseDown &e) {
	OpaqueWidget::onMouseDown(e);

	if (!audioIO)
		return;

	Menu *menu = gScene->createMenu();

	// Audio driver
	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Audio driver"));
	for (int driver : audioIO->listDrivers()) {
		AudioDriverItem *item = new AudioDriverItem();
		item->audioIO = audioIO;
		item->driver = driver;
		item->text = audioIO->getDriverName(driver);
		item->rightText = CHECKMARK(item->driver == audioIO->getDriver());
		menu->addChild(item);
	}
	menu->addChild(construct<MenuEntry>());

	// Audio device
	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Audio device"));
	int deviceCount = audioIO->getDeviceCount();
	{
		AudioDeviceItem *item = new AudioDeviceItem();
		item->audioIO = audioIO;
		item->device = -1;
		item->text = "No device";
		item->rightText = CHECKMARK(item->device == audioIO->device);
		menu->addChild(item);
	}
	for (int device = 0; device < deviceCount; device++) {
		AudioDeviceItem *item = new AudioDeviceItem();
		item->audioIO = audioIO;
		item->device = device;
		item->text = audioIO->getDeviceDetail(device);
		item->rightText = CHECKMARK(item->device == audioIO->device);
		menu->addChild(item);
	}
	menu->addChild(construct<MenuEntry>());

	// Sample rate
	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Sample rate"));
	for (int sampleRate : audioIO->listSampleRates()) {
		AudioSampleRateItem *item = new AudioSampleRateItem();
		item->audioIO = audioIO;
		item->sampleRate = sampleRate;
		item->text = stringf("%d Hz", sampleRate);
		item->rightText = CHECKMARK(item->sampleRate == audioIO->sampleRate);
		menu->addChild(item);
	}
	menu->addChild(construct<MenuEntry>());

	// Block size
	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Block size"));
	std::vector<int> blockSizes = {64, 128, 256, 512, 1024, 2048, 4096};
	for (int blockSize : blockSizes) {
		AudioBlockSizeItem *item = new AudioBlockSizeItem();
		item->audioIO = audioIO;
		item->blockSize = blockSize;
		float latency = (float) blockSize / audioIO->sampleRate * 1000.0;
		item->text = stringf("%d (%.1f ms)", blockSize, latency);
		item->rightText = CHECKMARK(item->blockSize == audioIO->blockSize);
		menu->addChild(item);
	}
}


} // namespace rack
