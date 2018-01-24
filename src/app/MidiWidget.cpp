#include "app.hpp"
#include "midi.hpp"


namespace rack {


struct MidiDeviceItem : MenuItem {
	MidiIO *midiIO;
	int device;
	void onAction(EventAction &e) override {
		midiIO->openDevice(device);
	}
};


struct MidiChannelItem : MenuItem {
	MidiIO *midiIO;
	int channel;
	void onAction(EventAction &e) override {
		midiIO->channel = channel;
	}
};


void MidiWidget::onMouseDown(EventMouseDown &e) {
	OpaqueWidget::onMouseDown(e);

	if (!midiIO)
		return;

	Menu *menu = gScene->createMenu();

	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "MIDI device"));
	{
		MidiDeviceItem *item = new MidiDeviceItem();
		item->midiIO = midiIO;
		item->device = -1;
		item->text = "No device";
		item->rightText = CHECKMARK(item->device == midiIO->device);
		menu->addChild(item);
	}
	for (int device = 0; device < midiIO->getDeviceCount(); device++) {
		MidiDeviceItem *item = new MidiDeviceItem();
		item->midiIO = midiIO;
		item->device = device;
		item->text = midiIO->getDeviceName(device);
		item->rightText = CHECKMARK(item->device == midiIO->device);
		menu->addChild(item);
	}
	menu->addChild(construct<MenuEntry>());

	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "MIDI channel"));
	MidiInput *midiInput = dynamic_cast<MidiInput*>(midiIO);
	if (midiInput) {
		MidiChannelItem *item = new MidiChannelItem();
		item->midiIO = midiIO;
		item->channel = -1;
		item->text = "All";
		item->rightText = CHECKMARK(item->channel == midiIO->channel);
		menu->addChild(item);
	}
	for (int channel = 0; channel < 16; channel++) {
		MidiChannelItem *item = new MidiChannelItem();
		item->midiIO = midiIO;
		item->channel = channel;
		item->text = stringf("%d", channel + 1);
		item->rightText = CHECKMARK(item->channel == midiIO->channel);
		menu->addChild(item);
	}
}


} // namespace rack
