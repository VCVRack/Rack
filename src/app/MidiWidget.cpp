#include "app.hpp"
#include "midi.hpp"


namespace rack {


struct MidiDriverItem : MenuItem {
	MidiIO *midiIO;
	int driverId;
	void onAction(EventAction &e) override {
		midiIO->setDriverId(driverId);
	}
};

struct MidiDriverChoice : LedDisplayChoice {
	MidiWidget *midiWidget;
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "MIDI driver"));
		for (int driverId : midiWidget->midiIO->getDriverIds()) {
			MidiDriverItem *item = new MidiDriverItem();
			item->midiIO = midiWidget->midiIO;
			item->driverId = driverId;
			item->text = midiWidget->midiIO->getDriverName(driverId);
			item->rightText = CHECKMARK(item->driverId == midiWidget->midiIO->driverId);
			menu->addChild(item);
		}
	}
	void step() override {
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

struct MidiDeviceItem : MenuItem {
	MidiIO *midiIO;
	int deviceId;
	void onAction(EventAction &e) override {
		midiIO->setDeviceId(deviceId);
	}
};

struct MidiDeviceChoice : LedDisplayChoice {
	MidiWidget *midiWidget;
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "MIDI device"));
		{
			MidiDeviceItem *item = new MidiDeviceItem();
			item->midiIO = midiWidget->midiIO;
			item->deviceId = -1;
			item->text = "(No device)";
			item->rightText = CHECKMARK(item->deviceId == midiWidget->midiIO->deviceId);
			menu->addChild(item);
		}
		for (int deviceId : midiWidget->midiIO->getDeviceIds()) {
			MidiDeviceItem *item = new MidiDeviceItem();
			item->midiIO = midiWidget->midiIO;
			item->deviceId = deviceId;
			item->text = midiWidget->midiIO->getDeviceName(deviceId);
			item->rightText = CHECKMARK(item->deviceId == midiWidget->midiIO->deviceId);
			menu->addChild(item);
		}
	}
	void step() override {
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

struct MidiChannelItem : MenuItem {
	MidiIO *midiIO;
	int channel;
	void onAction(EventAction &e) override {
		midiIO->channel = channel;
	}
};

struct MidiChannelChoice : LedDisplayChoice {
	MidiWidget *midiWidget;
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "MIDI channel"));
		for (int channel = -1; channel < 16; channel++) {
			MidiChannelItem *item = new MidiChannelItem();
			item->midiIO = midiWidget->midiIO;
			item->channel = channel;
			item->text = midiWidget->midiIO->getChannelName(channel);
			item->rightText = CHECKMARK(item->channel == midiWidget->midiIO->channel);
			menu->addChild(item);
		}
	}
	void step() override {
		text = midiWidget->midiIO->getChannelName(midiWidget->midiIO->channel);
	}
};


MidiWidget::MidiWidget() {
	box.size = mm2px(Vec(44, 28));

	Vec pos = Vec();

	MidiDriverChoice *driverChoice = Widget::create<MidiDriverChoice>(pos);
	driverChoice->midiWidget = this;
	addChild(driverChoice);
	pos = driverChoice->box.getBottomLeft();
	this->driverChoice = driverChoice;

	this->driverSeparator = Widget::create<LedDisplaySeparator>(pos);
	addChild(this->driverSeparator);

	MidiDeviceChoice *deviceChoice = Widget::create<MidiDeviceChoice>(pos);
	deviceChoice->midiWidget = this;
	addChild(deviceChoice);
	pos = deviceChoice->box.getBottomLeft();
	this->deviceChoice = deviceChoice;

	this->deviceSeparator = Widget::create<LedDisplaySeparator>(pos);
	addChild(this->deviceSeparator);

	MidiChannelChoice *channelChoice = Widget::create<MidiChannelChoice>(pos);
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


} // namespace rack
