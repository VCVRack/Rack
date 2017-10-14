#include "rack.hpp"
#include "rtmidi/RtMidi.h"


using namespace rack;


//////////////////////
// MIDI module widgets
//////////////////////

struct MidiIO {
	int portId;
	int channel;
	RtMidi *rtMidi;

	MidiIO(bool isOut=false);

	int getPortCount();

	std::string getPortName(int portId);

	void setPortId(int portId);

	void setChannel(int channel);

	json_t *addBaseJson(json_t *rootJ);

	void baseFromJson(json_t *rootJ);

	/* called when midi port is set */
	virtual void resetMidi()=0;
};

struct MidiItem : MenuItem {
	MidiIO *midiModule;
	int portId;

	void onAction();
};

struct MidiChoice : ChoiceButton {
	MidiIO *midiModule;

	void onAction();

	void step();
};

struct ChannelItem : MenuItem {
	MidiIO *midiModule;
	int channel;

	void onAction();
};

struct ChannelChoice : ChoiceButton {
	MidiIO *midiModule;

	void onAction();

	void step();
};


struct MidiToCVWidget : ModuleWidget{
	MidiToCVWidget();

	void step();
};

struct MIDICCToCVWidget : ModuleWidget{
	MIDICCToCVWidget();

	void step();
};

struct MIDIClockToCVWidget : ModuleWidget {
	MIDIClockToCVWidget();

	void step();
};

struct MIDITriggerToCVWidget : ModuleWidget {
	MIDITriggerToCVWidget();

	void step();
};

