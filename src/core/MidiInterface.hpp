#include <unordered_map>
#include "rack.hpp"
#include "rtmidi/RtMidi.h"


using namespace rack;


/**
 * This class allows to use one instance of rtMidiIn with
 * multiple modules. A MidiIn port will be opened only once while multiple
 * instances can use it simultaniously, each receiving all its incoming messages.
 */

struct MidiInWrapper : RtMidiIn {
	std::unordered_map<int, std::list<std::vector<unsigned char>>> idMessagesMap;
	std::unordered_map<int, std::list<double>> idStampsMap;
	int uid_c = 0;
	int subscribers = 0;

	MidiInWrapper() : RtMidiIn() {
		idMessagesMap = {};
		idStampsMap = {};
	};

	int add() {
		int id = ++uid_c;
		subscribers++;
		idMessagesMap[id] = {};
		idStampsMap[id] = {};
		return id;
	}

	void erase(int id) {
		subscribers--;
		idMessagesMap.erase(id);
		idStampsMap.erase(id);
	}
};

struct MidiIO {
private:
	static std::unordered_map<std::string, MidiInWrapper *> midiInMap;
	/* TODO: add for midi out*/
	int id = -1;
	std::string deviceName = "";
	bool isOut = false;

public:
	bool ignore_midiSysex=true;
	bool ignore_midiTime=true;
	bool ignore_midiSense=true;
	int channel;


	MidiIO(bool isOut = false);

	~MidiIO() {
		close();
	}

	std::vector<std::string> getDevices();

	void openDevice(std::string deviceName);

	std::string getDeviceName();

	void setChannel(int channel);

	double getMessage(std::vector<unsigned char> *msg);

	json_t *addBaseJson(json_t *rootJ);

	void baseFromJson(json_t *rootJ);

	bool isPortOpen();

	void close();

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

