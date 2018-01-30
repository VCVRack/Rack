#include "app.hpp"
#include "midi.hpp"


namespace rack {


struct MidiPortItem : MenuItem {
	MidiIO *midiIO;
	int port;
	void onAction(EventAction &e) override {
		midiIO->openPort(port);
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

	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "MIDI port"));
	for (int port = 0; port < midiIO->getPortCount(); port++) {
		MidiPortItem *item = new MidiPortItem();
		item->midiIO = midiIO;
		item->port = port;
		item->text = midiIO->getPortName(port);
		item->rightText = CHECKMARK(item->port == midiIO->port);
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
