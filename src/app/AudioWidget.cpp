#include <app/AudioWidget.hpp>
#include <helpers.hpp>


namespace rack {
namespace app {


struct AudioDriverItem : ui::MenuItem {
	audio::Port* port;
	int driverId;
	void onAction(const event::Action& e) override {
		port->setDriverId(driverId);
	}
};

struct AudioDriverChoice : LedDisplayChoice {
	audio::Port* port;
	void onAction(const event::Action& e) override {
		if (!port)
			return;

		ui::Menu* menu = createMenu();
		menu->addChild(createMenuLabel("Audio driver"));
		for (int driverId : audio::getDriverIds()) {
			AudioDriverItem* item = new AudioDriverItem;
			item->port = port;
			item->driverId = driverId;
			item->text = audio::getDriver(driverId)->getName();
			item->rightText = CHECKMARK(item->driverId == port->getDriverId());
			menu->addChild(item);
		}
	}
	void step() override {
		text = "";
		if (box.size.x >= 200.0)
			text += "Driver: ";
		std::string driverName = (port && port->driver) ? port->getDriver()->getName() : "";
		if (driverName != "") {
			text += driverName;
			color.a = 1.0;
		}
		else {
			text += "(No driver)";
			color.a = 0.5;
		}
	}
};


struct AudioDeviceItem : ui::MenuItem {
	audio::Port* port;
	int deviceId;
	int offset;
	void onAction(const event::Action& e) override {
		port->setDeviceId(deviceId);
		port->offset = offset;
	}
};

struct AudioDeviceChoice : LedDisplayChoice {
	audio::Port* port;

	void onAction(const event::Action& e) override {
		if (!port || !port->driver)
			return;

		ui::Menu* menu = createMenu();
		menu->addChild(createMenuLabel("Audio device"));
		{
			AudioDeviceItem* item = new AudioDeviceItem;
			item->port = port;
			item->deviceId = -1;
			item->text = "(No device)";
			item->rightText = CHECKMARK(port->getDeviceId() == -1);
			menu->addChild(item);
		}
		for (int deviceId : port->driver->getDeviceIds()) {
			int channels = std::max(port->driver->getDeviceNumInputs(deviceId), port->driver->getDeviceNumOutputs(deviceId));
			// Prevents devices with a ridiculous number of channels from being displayed
			const int maxTotalChannels = 128;
			channels = std::min(maxTotalChannels, channels);

			for (int offset = 0; offset < channels; offset += port->maxChannels) {
				AudioDeviceItem* item = new AudioDeviceItem;
				item->port = port;
				item->deviceId = deviceId;
				item->offset = offset;
				item->text = port->driver->getDeviceDetail(deviceId, offset, port->maxChannels);
				item->rightText = CHECKMARK(item->deviceId == port->getDeviceId() && item->offset == port->offset);
				menu->addChild(item);
			}
		}
	}
	void step() override {
		text = "";
		if (box.size.x >= 200.0)
			text += "Device: ";
		std::string detail = (port && port->device) ? port->device->getDetail(port->offset, port->maxChannels) : "";
		if (detail != "") {
			text += detail;
			color.a = 1.0;
		}
		else {
			text += "(No device)";
			color.a = 0.5;
		}
	}
};


struct AudioSampleRateItem : ui::MenuItem {
	audio::Port* port;
	int sampleRate;
	void onAction(const event::Action& e) override {
		port->setSampleRate(sampleRate);
	}
};

struct AudioSampleRateChoice : LedDisplayChoice {
	audio::Port* port;
	void onAction(const event::Action& e) override {
		if (!port)
			return;

		ui::Menu* menu = createMenu();
		menu->addChild(createMenuLabel("Sample rate"));
		std::vector<int> sampleRates = port->getSampleRates();
		if (sampleRates.empty()) {
			menu->addChild(createMenuLabel("(Locked by device)"));
		}
		for (int sampleRate : sampleRates) {
			AudioSampleRateItem* item = new AudioSampleRateItem;
			item->port = port;
			item->sampleRate = sampleRate;
			item->text = string::f("%g kHz", sampleRate / 1000.0);
			item->rightText = CHECKMARK(item->sampleRate == port->getSampleRate());
			menu->addChild(item);
		}
	}
	void step() override {
		text = "";
		if (box.size.x >= 100.0)
			text += "Rate: ";
		int sampleRate = port ? port->getSampleRate() : 0;
		if (sampleRate > 0) {
			text += string::f("%g", sampleRate / 1000.0);
			color.a = 1.0;
		}
		else {
			text += "---";
			color.a = 0.5;
		}
		text += " kHz";
	}
};


struct AudioBlockSizeItem : ui::MenuItem {
	audio::Port* port;
	int blockSize;
	void onAction(const event::Action& e) override {
		port->setBlockSize(blockSize);
	}
};

struct AudioBlockSizeChoice : LedDisplayChoice {
	audio::Port* port;
	void onAction(const event::Action& e) override {
		if (!port)
			return;

		ui::Menu* menu = createMenu();
		menu->addChild(createMenuLabel("Block size"));
		std::vector<int> blockSizes = port->getBlockSizes();
		if (blockSizes.empty()) {
			menu->addChild(createMenuLabel("(Locked by device)"));
		}
		for (int blockSize : blockSizes) {
			AudioBlockSizeItem* item = new AudioBlockSizeItem;
			item->port = port;
			item->blockSize = blockSize;
			float latency = (float) blockSize / port->getSampleRate() * 1000.0;
			item->text = string::f("%d (%.1f ms)", blockSize, latency);
			item->rightText = CHECKMARK(item->blockSize == port->getBlockSize());
			menu->addChild(item);
		}
	}
	void step() override {
		text = "";
		if (box.size.x >= 100.0)
			text += "Block size: ";
		int blockSize = port ? port->getBlockSize() : 0;
		if (blockSize > 0) {
			text += string::f("%d", blockSize);
			color.a = 1.0;
		}
		else {
			text += "---";
			color.a = 0.5;
		}
	}
};


void AudioWidget::setAudioPort(audio::Port* port) {
	clearChildren();

	math::Vec pos;

	AudioDriverChoice* driverChoice = createWidget<AudioDriverChoice>(pos);
	driverChoice->box.size.x = box.size.x;
	driverChoice->port = port;
	addChild(driverChoice);
	pos = driverChoice->box.getBottomLeft();
	this->driverChoice = driverChoice;

	this->driverSeparator = createWidget<LedDisplaySeparator>(pos);
	this->driverSeparator->box.size.x = box.size.x;
	addChild(this->driverSeparator);

	AudioDeviceChoice* deviceChoice = createWidget<AudioDeviceChoice>(pos);
	deviceChoice->box.size.x = box.size.x;
	deviceChoice->port = port;
	addChild(deviceChoice);
	pos = deviceChoice->box.getBottomLeft();
	this->deviceChoice = deviceChoice;

	this->deviceSeparator = createWidget<LedDisplaySeparator>(pos);
	this->deviceSeparator->box.size.x = box.size.x;
	addChild(this->deviceSeparator);

	AudioSampleRateChoice* sampleRateChoice = createWidget<AudioSampleRateChoice>(pos);
	sampleRateChoice->box.size.x = box.size.x / 2;
	sampleRateChoice->port = port;
	addChild(sampleRateChoice);
	this->sampleRateChoice = sampleRateChoice;

	this->sampleRateSeparator = createWidget<LedDisplaySeparator>(pos);
	this->sampleRateSeparator->box.pos.x = box.size.x / 2;
	this->sampleRateSeparator->box.size.y = this->sampleRateChoice->box.size.y;
	addChild(this->sampleRateSeparator);

	AudioBlockSizeChoice* bufferSizeChoice = createWidget<AudioBlockSizeChoice>(pos);
	bufferSizeChoice->box.pos.x = box.size.x / 2;
	bufferSizeChoice->box.size.x = box.size.x / 2;
	bufferSizeChoice->port = port;
	addChild(bufferSizeChoice);
	this->bufferSizeChoice = bufferSizeChoice;
}


} // namespace app
} // namespace rack
