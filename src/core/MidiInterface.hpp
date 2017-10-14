#include <unordered_map>
#include "rack.hpp"
#include "rtmidi/RtMidi.h"


using namespace rack;


/**
 * This class allows to use one instance of rtMidiIn with
 * multiple modules. A MidiIn port will be opened only once while multiple
 * instances can use it simultaniously, each receiving all its incoming messages.
 */
struct RtMidiInSplitter {
private:
	std::unordered_map<std::string, RtMidiIn*> midiInMap;
	std::unordered_map<std::string, std::unordered_map<int, std::list<std::vector<unsigned char>>>> deviceIdMessagesMap;
public:
	RtMidiInSplitter();

	/* Returns an Id which uniquely identifies the caller in combination with the interface name */
	int openDevice(std::string interfaceName);

	/* Returns the next message in queue for given device & id*/
	std::vector<unsigned char> getMessage(std::string deviceName, int id);

	/* Returns Device names as string*/
	std::vector<std::string> getDevices();

};

//struct RtMidiOutSplitter {
//private:
//	std::unordered_map<std::string, RtMidiOut> midiOuts;
//public:
//	RtMidiOutSplitter();
//};

struct MidiIO {
private:
	static RtMidiInSplitter midiInSplitter;
	int id = -1;
	std::string deviceName = "";
public:
	void setDeviceName(const std::string &deviceName);

//	static RtMidiOutSplitter MidiOutSlpitter = RtMidiOutSplitter();

public:
	int channel;
	bool isOut = false;


	MidiIO(bool isOut = false);

	std::vector<std::string> getDevices();
	void openDevice(std::string deviceName);

	std::string getDeviceName();

	void setChannel(int channel);

	std::vector<unsigned char> getMessage();

	bool isPortOpen();

	json_t *addBaseJson(json_t *rootJ);

	void baseFromJson(json_t *rootJ);

	/* called when midi port is set */
	virtual void resetMidi()=0;
};

//////////////////////
// MIDI module widgets
//////////////////////

struct MidiItem : MenuItem {
	MidiIO *midiModule;

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


struct MidiToCVWidget : ModuleWidget {
	MidiToCVWidget();

	void step();
};

struct MIDICCToCVWidget : ModuleWidget {
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

