#if 0
#include <unordered_map>
#include "rack.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#include "rtmidi/RtMidi.h"
#pragma GCC diagnostic pop


using namespace rack;


struct IgnoreFlags {
	bool midiSysex = true;
	bool midiTime = true;
	bool midiSense = true;
};

struct MidiMessage {
	std::vector<unsigned char> bytes;
	double timeStamp;

	MidiMessage() : bytes(0), timeStamp(0.0) {};
};

/**
 * This class allows to use one instance of rtMidiIn with
 * multiple modules. A MidiIn port will be opened only once while multiple
 * instances can use it simultaniously, each receiving all its incoming messages.
 */
struct MidiInWrapper : RtMidiIn {

	std::unordered_map<int, std::list<MidiMessage>> idMessagesMap;
	std::unordered_map<int, IgnoreFlags> ignoresMap;

	int uid_c = 0;

	MidiInWrapper() : RtMidiIn() {
		idMessagesMap = {};
	};

	int add() {
		int id = ++uid_c;
		idMessagesMap[id] = {};
		ignoresMap[id] = IgnoreFlags();
		return id;
	}

	void erase(int id) {
		idMessagesMap.erase(id);
		ignoresMap.erase(id);
	}
};

/**
 * Note: MidiIO is not thread safe which might become
 * important in the future
 */
struct MidiIO {
private:
	static std::unordered_map<std::string, MidiInWrapper *> midiInMap;
	/* TODO: add for midi out*/
	int id = -1;
	std::string deviceName = "";
	bool isOut = false;

public:
	int channel;


	MidiIO(bool isOut = false);

	~MidiIO() {
		close();
	}

	std::vector<std::string> getDevices();

	void openDevice(std::string deviceName);

	void setIgnores(bool ignoreSysex = true, bool ignoreTime = true, bool ignoreSense = true);

	std::string getDeviceName();

	void setChannel(int channel);

	double getMessage(std::vector<unsigned char> *msg);

	json_t *addBaseJson(json_t *rootJ);

	void baseFromJson(json_t *rootJ);

	bool isPortOpen();

	void close();

	/* called when midi port is set */
	virtual void resetMidi() {}

	/* called if a user switches or sets the device (and after this device is initialised)*/
	virtual void onDeviceChange() {}
};


struct TransitionSmoother {
	enum TransitionFunction {
		SMOOTHSTEP,
		EXP,
		LIN,
	};

	enum TransitionMode {
		DELTA,
		CONST,
	};

	float start;
	float end;
	float x;
	float delta;
	float step;
	TransitionFunction t;


	void set(float start, float end, int l = 1500, TransitionFunction t = LIN,  TransitionMode m = DELTA, bool reset = true) {
		this->start = start;
		this->end = end;
		this->delta = end - start;
		this->t = t;

		if (reset || x >= 1) {
			this->x = 0;
		}

		switch (m) {
		case DELTA:
			/* If the change is smaller, the transition phase is longer */
			this->step = delta > 0 ?  delta / l :  -delta / l;
			break;
		case CONST:
			this->step = 1.0 / l;
			break;
		}

	}

	float next() {
		float next = start;

		x += step;
		if (x >= 1)
			return end;

		switch (t) {
		case SMOOTHSTEP:
			next += delta * x * x * (3 - 2 * x);
			break;
		case EXP:
			next += delta * x * x;
			break;
		case LIN:
			next += delta * x;
			break;
		}

		if ((delta > 0 && next > end) || (delta <= 0 && next < end))
			return end;

		return next;;
	}
};


//////////////////////
// MIDI module widgets
//////////////////////

struct MidiItem : MenuItem {
	MidiIO *midiModule;

	void onAction(EventAction &e) override;
};

struct MidiChoice : ChoiceButton {
	MidiIO *midiModule;

	void step() override;
	void onAction(EventAction &e) override;
};

struct ChannelItem : MenuItem {
	MidiIO *midiModule;
	int channel;

	void onAction(EventAction &e) override;
};

struct ChannelChoice : ChoiceButton {
	MidiIO *midiModule;

	void step() override;
	void onAction(EventAction &e) override;
};

#endif