#include "app/MidiWidget.hpp"
#include "midi.hpp"
#include "helpers.hpp"


namespace rack {
namespace app {


struct MidiDriverItem : ui::MenuItem {
	midi::IO *midiIO;
	int driverId;
	void onAction(const event::Action &e) override {
		midiIO->setDriverId(driverId);
	}
};

struct MidiDriverChoice : LedDisplayChoice {
	MidiWidget *midiWidget;
	void onAction(const event::Action &e) override {
		if (!midiWidget->midiIO)
			return;

		ui::Menu *menu = createMenu();
		menu->addChild(createMenuLabel("MIDI driver"));
		for (int driverId : midiWidget->midiIO->getDriverIds()) {
			MidiDriverItem *item = new MidiDriverItem;
			item->midiIO = midiWidget->midiIO;
			item->driverId = driverId;
			item->text = midiWidget->midiIO->getDriverName(driverId);
			item->rightText = CHECKMARK(item->driverId == midiWidget->midiIO->driverId);
			menu->addChild(item);
		}
	}
	void step() override {
		if (!midiWidget->midiIO) {
			text = "";
			return;
		}
		text = midiWidget->midiIO->getDriverName(midiWidget->midiIO->driverId);
		if (text.empty()) {
			text = "(No driver)";
			color.a = 0.5f;
		}
		else {
			color.a = 1.f;
		}
	}
};

struct MidiDeviceItem : ui::MenuItem {
	midi::IO *midiIO;
	int deviceId;
	void onAction(const event::Action &e) override {
		midiIO->setDeviceId(deviceId);
	}
};

struct MidiDeviceChoice : LedDisplayChoice {
	MidiWidget *midiWidget;
	void onAction(const event::Action &e) override {
		if (!midiWidget->midiIO)
			return;

		ui::Menu *menu = createMenu();
		menu->addChild(createMenuLabel("MIDI device"));
		{
			MidiDeviceItem *item = new MidiDeviceItem;
			item->midiIO = midiWidget->midiIO;
			item->deviceId = -1;
			item->text = "(No device)";
			item->rightText = CHECKMARK(item->deviceId == midiWidget->midiIO->deviceId);
			menu->addChild(item);
		}
		for (int deviceId : midiWidget->midiIO->getDeviceIds()) {
			MidiDeviceItem *item = new MidiDeviceItem;
			item->midiIO = midiWidget->midiIO;
			item->deviceId = deviceId;
			item->text = midiWidget->midiIO->getDeviceName(deviceId);
			item->rightText = CHECKMARK(item->deviceId == midiWidget->midiIO->deviceId);
			menu->addChild(item);
		}
	}
	void step() override {
		if (!midiWidget->midiIO) {
			text = "";
			return;
		}
		text = midiWidget->midiIO->getDeviceName(midiWidget->midiIO->deviceId);
		if (text.empty()) {
			text = "(No device)";
			color.a = 0.5f;
		}
		else {
			color.a = 1.f;
		}
	}
};

struct MidiChannelItem : ui::MenuItem {
	midi::IO *midiIO;
	int channel;
	void onAction(const event::Action &e) override {
		midiIO->channel = channel;
	}
};

struct MidiChannelChoice : LedDisplayChoice {
	MidiWidget *midiWidget;
	void onAction(const event::Action &e) override {
		if (!midiWidget->midiIO)
			return;

		ui::Menu *menu = createMenu();
		menu->addChild(createMenuLabel("MIDI channel"));
		for (int channel : midiWidget->midiIO->getChannels()) {
			MidiChannelItem *item = new MidiChannelItem;
			item->midiIO = midiWidget->midiIO;
			item->channel = channel;
			item->text = midiWidget->midiIO->getChannelName(channel);
			item->rightText = CHECKMARK(item->channel == midiWidget->midiIO->channel);
			menu->addChild(item);
		}
	}
	void step() override {
		if (midiWidget->midiIO)
			text = midiWidget->midiIO->getChannelName(midiWidget->midiIO->channel);
		else
			text = "";
	}
};


MidiWidget::MidiWidget() {
	box.size = mm2px(math::Vec(44, 28));

	math::Vec pos = math::Vec();

	MidiDriverChoice *driverChoice = createWidget<MidiDriverChoice>(pos);
	driverChoice->midiWidget = this;
	addChild(driverChoice);
	pos = driverChoice->box.getBottomLeft();
	this->driverChoice = driverChoice;

	this->driverSeparator = createWidget<LedDisplaySeparator>(pos);
	addChild(this->driverSeparator);

	MidiDeviceChoice *deviceChoice = createWidget<MidiDeviceChoice>(pos);
	deviceChoice->midiWidget = this;
	addChild(deviceChoice);
	pos = deviceChoice->box.getBottomLeft();
	this->deviceChoice = deviceChoice;

	this->deviceSeparator = createWidget<LedDisplaySeparator>(pos);
	addChild(this->deviceSeparator);

	MidiChannelChoice *channelChoice = createWidget<MidiChannelChoice>(pos);
	channelChoice->midiWidget = this;
	addChild(channelChoice);
	this->channelChoice = channelChoice;
}

void MidiWidget::step() {
	this->driverChoice->box.size.x = box.size.x;
	this->driverSeparator->box.size.x = box.size.x;
	this->deviceChoice->box.size.x = box.size.x;
	this->deviceSeparator->box.size.x = box.size.x;
	this->channelChoice->box.size.x = box.size.x;
	LedDisplay::step();
}


} // namespace app
} // namespace rack
