#include "app/AudioWidget.hpp"
#include "audio.hpp"
#include "helpers.hpp"


namespace rack {
namespace app {


struct AudioDriverItem : ui::MenuItem {
	audio::Port *port;
	int driver;
	void onAction(const event::Action &e) override {
		port->setDriver(driver);
	}
};

struct AudioDriverChoice : LedDisplayChoice {
	audio::Port *port;
	void onAction(const event::Action &e) override {
		if (!port)
			return;

		ui::Menu *menu = createMenu();
		menu->addChild(createMenuLabel("Audio driver"));
		for (int driver : port->getDrivers()) {
			AudioDriverItem *item = new AudioDriverItem;
			item->port = port;
			item->driver = driver;
			item->text = port->getDriverName(driver);
			item->rightText = CHECKMARK(item->driver == port->driver);
			menu->addChild(item);
		}
	}
	void step() override {
		if (port)
			text = port->getDriverName(port->driver);
		else
			text = "";
	}
};


struct AudioDeviceItem : ui::MenuItem {
	audio::Port *port;
	int device;
	int offset;
	void onAction(const event::Action &e) override {
		port->setDevice(device, offset);
	}
};

struct AudioDeviceChoice : LedDisplayChoice {
	audio::Port *port;
	/** Prevents devices with a ridiculous number of channels from being displayed */
	int maxTotalChannels = 128;

	void onAction(const event::Action &e) override {
		if (!port)
			return;

		ui::Menu *menu = createMenu();
		menu->addChild(createMenuLabel("Audio device"));
		int deviceCount = port->getDeviceCount();
		{
			AudioDeviceItem *item = new AudioDeviceItem;
			item->port = port;
			item->device = -1;
			item->text = "(No device)";
			item->rightText = CHECKMARK(item->device == port->device);
			menu->addChild(item);
		}
		for (int device = 0; device < deviceCount; device++) {
			int channels = std::min(maxTotalChannels, port->getDeviceChannels(device));
			for (int offset = 0; offset < channels; offset += port->maxChannels) {
				AudioDeviceItem *item = new AudioDeviceItem;
				item->port = port;
				item->device = device;
				item->offset = offset;
				item->text = port->getDeviceDetail(device, offset);
				item->rightText = CHECKMARK(item->device == port->device && item->offset == port->offset);
				menu->addChild(item);
			}
		}
	}
	void step() override {
		if (!port) {
			text = "";
			return;
		}
		text = port->getDeviceDetail(port->device, port->offset);
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
	audio::Port *port;
	int sampleRate;
	void onAction(const event::Action &e) override {
		port->setSampleRate(sampleRate);
	}
};

struct AudioSampleRateChoice : LedDisplayChoice {
	audio::Port *port;
	void onAction(const event::Action &e) override {
		if (!port)
			return;

		ui::Menu *menu = createMenu();
		menu->addChild(createMenuLabel("Sample rate"));
		std::vector<int> sampleRates = port->getSampleRates();
		if (sampleRates.empty()) {
			menu->addChild(createMenuLabel("(Locked by device)"));
		}
		for (int sampleRate : sampleRates) {
			AudioSampleRateItem *item = new AudioSampleRateItem;
			item->port = port;
			item->sampleRate = sampleRate;
			item->text = string::f("%d Hz", sampleRate);
			item->rightText = CHECKMARK(item->sampleRate == port->sampleRate);
			menu->addChild(item);
		}
	}
	void step() override {
		if (port)
			text = string::f("%g kHz", port->sampleRate / 1000.f);
		else
			text = "";
	}
};


struct AudioBlockSizeItem : ui::MenuItem {
	audio::Port *port;
	int blockSize;
	void onAction(const event::Action &e) override {
		port->setBlockSize(blockSize);
	}
};

struct AudioBlockSizeChoice : LedDisplayChoice {
	audio::Port *port;
	void onAction(const event::Action &e) override {
		if (!port)
			return;

		ui::Menu *menu = createMenu();
		menu->addChild(createMenuLabel("Block size"));
		std::vector<int> blockSizes = port->getBlockSizes();
		if (blockSizes.empty()) {
			menu->addChild(createMenuLabel("(Locked by device)"));
		}
		for (int blockSize : blockSizes) {
			AudioBlockSizeItem *item = new AudioBlockSizeItem;
			item->port = port;
			item->blockSize = blockSize;
			float latency = (float) blockSize / port->sampleRate * 1000.0;
			item->text = string::f("%d (%.1f ms)", blockSize, latency);
			item->rightText = CHECKMARK(item->blockSize == port->blockSize);
			menu->addChild(item);
		}
	}
	void step() override {
		if (port)
			text = string::f("%d", port->blockSize);
		else
			text = "";
	}
};


void AudioWidget::setAudioPort(audio::Port *port) {
	clearChildren();

	math::Vec pos;

	AudioDriverChoice *driverChoice = createWidget<AudioDriverChoice>(pos);
	driverChoice->box.size.x = box.size.x;
	driverChoice->port = port;
	addChild(driverChoice);
	pos = driverChoice->box.getBottomLeft();
	this->driverChoice = driverChoice;

	this->driverSeparator = createWidget<LedDisplaySeparator>(pos);
	this->driverSeparator->box.size.x = box.size.x;
	addChild(this->driverSeparator);

	AudioDeviceChoice *deviceChoice = createWidget<AudioDeviceChoice>(pos);
	deviceChoice->box.size.x = box.size.x;
	deviceChoice->port = port;
	addChild(deviceChoice);
	pos = deviceChoice->box.getBottomLeft();
	this->deviceChoice = deviceChoice;

	this->deviceSeparator = createWidget<LedDisplaySeparator>(pos);
	this->deviceSeparator->box.size.x = box.size.x;
	addChild(this->deviceSeparator);

	AudioSampleRateChoice *sampleRateChoice = createWidget<AudioSampleRateChoice>(pos);
	sampleRateChoice->box.size.x = box.size.x / 2;
	sampleRateChoice->port = port;
	addChild(sampleRateChoice);
	this->sampleRateChoice = sampleRateChoice;

	this->sampleRateSeparator = createWidget<LedDisplaySeparator>(pos);
	this->sampleRateSeparator->box.pos.x = box.size.x / 2;
	this->sampleRateSeparator->box.size.y = this->sampleRateChoice->box.size.y;
	addChild(this->sampleRateSeparator);

	AudioBlockSizeChoice *bufferSizeChoice = createWidget<AudioBlockSizeChoice>(pos);
	bufferSizeChoice->box.pos.x = box.size.x / 2;
	bufferSizeChoice->box.size.x = box.size.x / 2;
	bufferSizeChoice->port = port;
	addChild(bufferSizeChoice);
	this->bufferSizeChoice = bufferSizeChoice;
}


} // namespace app
} // namespace rack
