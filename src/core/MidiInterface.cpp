#include <assert.h>
#include <list>
#include <algorithm>
#include "rtmidi/RtMidi.h"
#include "core.hpp"
#include "gui.hpp"
#include "../../include/engine.hpp"


using namespace rack;

/**
 * MidiIO implements the shared functionality of all midi modules, namely:
 * + Channel Selection (including helper for storing json)
 * + Interface Selection (including helper for storing json)
 * + rtMidi initialisation (input or output)
 */
struct MidiIO {
	int portId = -1;
	RtMidi *rtMidi = NULL;

	/** Filter MIDI channel
	-1 means all MIDI channels
	*/
	int channel = -1;

	/*
	 * If isOut is set to true, creates a RtMidiOut, RtMidiIn otherwise
	 */
	MidiIO(bool isOut = false) {
		try {
			if (isOut) {
				rtMidi = new RtMidiOut(RtMidi::UNSPECIFIED, "Rack");
			} else {
				rtMidi = new RtMidiIn(RtMidi::UNSPECIFIED, "Rack");
			}
		}
		catch (RtMidiError &error) {
			fprintf(stderr, "Failed to create RtMidiIn: %s\n", error.getMessage().c_str());
		}
	}

	~MidiIO() {}

	int getPortCount();

	std::string getPortName(int portId);

	// -1 will close the port
	void setPortId(int portId);

	void setChannel(int channel) {
		this->channel = channel;
	}

	json_t *addBaseJson(json_t *rootJ) {
		if (portId >= 0) {
			std::string portName = getPortName(portId);
			json_object_set_new(rootJ, "portName", json_string(portName.c_str()));
			json_object_set_new(rootJ, "channel", json_integer(channel));
		}
		return rootJ;
	}

	void baseFromJson(json_t *rootJ) {
		json_t *portNameJ = json_object_get(rootJ, "portName");
		if (portNameJ) {
			std::string portName = json_string_value(portNameJ);
			for (int i = 0; i < getPortCount(); i++) {
				if (portName == getPortName(i)) {
					setPortId(i);
					break;
				}
			}
		}

		json_t *channelJ = json_object_get(rootJ, "channel");
		if (channelJ) {
			setChannel(json_integer_value(channelJ));
		}
	}

};

int MidiIO::getPortCount() {
	return rtMidi->getPortCount();
}

std::string MidiIO::getPortName(int portId) {
	std::string portName;
	try {
		portName = rtMidi->getPortName(portId);
	}
	catch (RtMidiError &error) {
		fprintf(stderr, "Failed to get Port Name: %d, %s\n", portId, error.getMessage().c_str());
	}
	return portName;
}

void MidiIO::setPortId(int portId) {
	// Close port if it was previously opened
	if (rtMidi->isPortOpen()) {
		rtMidi->closePort();
	}
	this->portId = -1;

	// Open new port
	if (portId >= 0) {
		rtMidi->openPort(portId, "Midi Interface");
	}
	this->portId = portId;
}

struct MidiItem : MenuItem {
	MidiIO *midiModule;
	int portId;

	void onAction() {
		midiModule->setPortId(portId);
	}
};

struct MidiChoice : ChoiceButton {
	MidiIO *midiModule;

	void onAction() {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		int portCount = midiModule->getPortCount();
		{
			MidiItem *midiItem = new MidiItem();
			midiItem->midiModule = midiModule;
			midiItem->portId = -1;
			midiItem->text = "No device";
			menu->pushChild(midiItem);
		}
		for (int portId = 0; portId < portCount; portId++) {
			MidiItem *midiItem = new MidiItem();
			midiItem->midiModule = midiModule;
			midiItem->portId = portId;
			midiItem->text = midiModule->getPortName(portId);
			menu->pushChild(midiItem);
		}
	}

	void step() {
		std::string name = midiModule->getPortName(midiModule->portId);
		text = ellipsize(name, 15);
	}
};

struct ChannelItem : MenuItem {
	MidiIO *midiModule;
	int channel;

	void onAction() {
		midiModule->setChannel(channel);
	}
};

struct ChannelChoice : ChoiceButton {
	MidiIO *midiModule;

	void onAction() {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		{
			ChannelItem *channelItem = new ChannelItem();
			channelItem->midiModule = midiModule;
			channelItem->channel = -1;
			channelItem->text = "All";
			menu->pushChild(channelItem);
		}
		for (int channel = 0; channel < 16; channel++) {
			ChannelItem *channelItem = new ChannelItem();
			channelItem->midiModule = midiModule;
			channelItem->channel = channel;
			channelItem->text = stringf("%d", channel + 1);
			menu->pushChild(channelItem);
		}
	}

	void step() {
		text = (midiModule->channel >= 0) ? stringf("%d", midiModule->channel + 1) : "All";
	}
};

/*
 * MIDIToCVInterface converts midi note on/off events, velocity , channel aftertouch, pitch wheel and mod wheel to
 * CV
 */
struct MIDIToCVInterface : MidiIO, Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		PITCH_OUTPUT = 0,
		GATE_OUTPUT,
		VELOCITY_OUTPUT,
		MOD_OUTPUT,
		PITCHWHEEL_OUTPUT,
		CHANNEL_AFTERTOUCH_OUTPUT,
		NUM_OUTPUTS
	};

	std::list<int> notes;
	bool pedal = false;
	int note = 60; // C4, most modules should use 261.626 Hz
	int mod = 0;
	int vel = 0;
	int afterTouch = 0;
	int pitchWheel = 64;
	bool retrigger = false;
	bool retriggered = false;
	float lights[NUM_OUTPUTS];

	MIDIToCVInterface() : MidiIO(), Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {

	}

	~MIDIToCVInterface() {
		setPortId(-1);
	};

	void step();

	void pressNote(int note);

	void releaseNote(int note);

	void processMidi(std::vector<unsigned char> msg);

	virtual json_t *toJson() {
		json_t *rootJ = json_object();
		addBaseJson(rootJ);
		return rootJ;
	}

	virtual void fromJson(json_t *rootJ) {
		baseFromJson(rootJ);
	}

	virtual void initialize() {
		setPortId(-1);
	}

};


void MIDIToCVInterface::step() {
	if (rtMidi->isPortOpen()) {
		std::vector<unsigned char> message;

		// midiIn->getMessage returns empty vector if there are no messages in the queue

		dynamic_cast<RtMidiIn *>(rtMidi)->getMessage(&message);
		while (message.size() > 0) {
			processMidi(message);
			dynamic_cast<RtMidiIn *>(rtMidi)->getMessage(&message);
		}
	}

	outputs[PITCH_OUTPUT].value = ((note - 60)) / 12.0;

	bool gate = pedal || !notes.empty();
	if (retrigger && retriggered) {
		gate = false;
		retriggered = false;
	}
	outputs[GATE_OUTPUT].value = gate ? 10.0 : 0.0;
	lights[GATE_OUTPUT] = gate ? 1.0 : 0.0;

	outputs[MOD_OUTPUT].value = mod / 127.0 * 10.0;
	lights[MOD_OUTPUT] = mod / 127.0;

	outputs[PITCHWHEEL_OUTPUT].value = (pitchWheel - 64) / 64.0 * 10.0;
	lights[MOD_OUTPUT] = pitchWheel / 127.0;

	outputs[CHANNEL_AFTERTOUCH_OUTPUT].value = afterTouch / 127.0 * 10.0;
	lights[CHANNEL_AFTERTOUCH_OUTPUT] = afterTouch / 127.0;

	outputs[VELOCITY_OUTPUT].value = vel / 127.0 * 10.0;
	lights[VELOCITY_OUTPUT] = vel / 127.0;
}


void MIDIToCVInterface::pressNote(int note) {
	// Remove existing similar note
	auto it = std::find(notes.begin(), notes.end(), note);
	if (it != notes.end())
		notes.erase(it);
	// Push note
	notes.push_back(note);
	this->note = note;
	retriggered = true;
}

void MIDIToCVInterface::releaseNote(int note) {
	// Remove the note
	auto it = std::find(notes.begin(), notes.end(), note);
	if (it != notes.end())
		notes.erase(it);

	if (pedal) {
		// Don't release if pedal is held
	} else if (!notes.empty()) {
		// Play previous note
		auto it2 = notes.end();
		it2--;
		this->note = *it2;
		retriggered = true;
	}
}

void MIDIToCVInterface::processMidi(std::vector<unsigned char> msg) {
	int channel = msg[0] & 0xf;
	int status = (msg[0] >> 4) & 0xf;
	int data1 = msg[1];
	int data2 = msg[2];

	//fprintf(stderr, "channel %d status %d data1 %d data2 %d\n", channel, status, data1,data2);

	// Filter channels
	if (this->channel >= 0 && this->channel != channel)
		return;

	switch (status) {
		// note off
		case 0x8: {
			releaseNote(data1);
		}
			break;
		case 0x9: // note on
			if (data2 > 0) {
				pressNote(data1);
				this->vel = data2;
			} else {
				// For some reason, some keyboards send a "note on" event with a velocity of 0 to signal that the key has been released.
				releaseNote(data1);
			}
			break;
		case 0xb: // cc
			switch (data1) {
				case 0x01: // mod
					this->mod = data2;
					break;
				case 0x40: // sustain
					pedal = (data2 >= 64);
					releaseNote(-1);
					break;
			}
			break;
		case 0xe: // pitch wheel
			this->pitchWheel = data2;
			break;
		case 0xd: // channel aftertouch
			this->afterTouch = data1;
			break;
	}
}


MidiToCVWidget::MidiToCVWidget() {
	MIDIToCVInterface *module = new MIDIToCVInterface();
	setModule(module);
	box.size = Vec(15 * 9, 380);

	{
		Panel *panel = new LightPanel();
		panel->box.size = box.size;
		addChild(panel);
	}

	float margin = 5;
	float labelHeight = 15;
	float yPos = margin;
	float yGap = 35;

	addChild(createScrew<ScrewSilver>(Vec(margin, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 15 - margin, 0)));
	addChild(createScrew<ScrewSilver>(Vec(margin, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 15 - margin, 365)));

	{
		Label *label = new Label();
		label->box.pos = Vec(box.size.x - margin - 7 * 15, margin);
		label->text = "MIDI to CV";
		addChild(label);
		yPos = labelHeight * 2;

	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "MIDI Interface";
		addChild(label);
		yPos += labelHeight + margin;

		MidiChoice *midiChoice = new MidiChoice();
		midiChoice->midiModule = dynamic_cast<MidiIO *>(module);
		midiChoice->box.pos = Vec(margin, yPos);
		midiChoice->box.size.x = box.size.x - 10;
		addChild(midiChoice);
		yPos += midiChoice->box.size.y + margin;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Channel";
		addChild(label);
		yPos += labelHeight + margin;

		ChannelChoice *channelChoice = new ChannelChoice();
		channelChoice->midiModule = dynamic_cast<MidiIO *>(module);
		channelChoice->box.pos = Vec(margin, yPos);
		channelChoice->box.size.x = box.size.x - 10;
		addChild(channelChoice);
		yPos += channelChoice->box.size.y + margin + 15;
	}

	std::string labels[MIDIToCVInterface::NUM_OUTPUTS] = {"1V/oct", "Gate", "Velocity", "Mod Wheel",
														  "Pitch Wheel", "Aftertouch"};

	for (int i = 0; i < MIDIToCVInterface::NUM_OUTPUTS; i++) {
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = labels[i];
		addChild(label);

		addOutput(createOutput<PJ3410Port>(Vec(15 * 6, yPos - 5), module, i));
		if (i != MIDIToCVInterface::PITCH_OUTPUT) {
			addChild(createValueLight<SmallLight<GreenValueLight>>(Vec(15 * 7.5, yPos - 5), &module->lights[i]));

		}
		yPos += yGap + margin;
	}


}

void MidiToCVWidget::step() {
	// Assume QWERTY
#define MIDI_KEY(key, midi) if (glfwGetKey(gWindow, key)) printf("%d\n", midi);

	// MIDI_KEY(GLFW_KEY_Z, 48);

	ModuleWidget::step();
}


/*
 * MIDIToCVInterface converts midi note on/off events, velocity , channel aftertouch, pitch wheel and mod weel to
 * CV
 */
struct MIDICCToCVInterface : MidiIO, Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS = 16
	};

	int cc[NUM_OUTPUTS];
	int ccNum[NUM_OUTPUTS];
	bool ccNumInited[NUM_OUTPUTS];
	float lights[NUM_OUTPUTS];


	MIDICCToCVInterface() : MidiIO(), Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			cc[i] = 0;
			ccNum[i] = i;
		}
	}

	~MIDICCToCVInterface() {
		setPortId(-1);
	}

	void step();

	void processMidi(std::vector<unsigned char> msg);

	virtual json_t *toJson() {
		json_t *rootJ = json_object();
		addBaseJson(rootJ);
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			json_object_set_new(rootJ, std::to_string(i).c_str(), json_integer(ccNum[i]));
		}
		return rootJ;
	}

	virtual void fromJson(json_t *rootJ) {
		baseFromJson(rootJ);
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			json_t *ccNumJ = json_object_get(rootJ, std::to_string(i).c_str());
			if (ccNumJ) {
				ccNum[i] = json_integer_value(ccNumJ);
				ccNumInited[i] = true;
			}

		}
	}

	virtual void initialize() {
		setPortId(-1);
	}

};


void MIDICCToCVInterface::step() {
	if (rtMidi->isPortOpen()) {
		std::vector<unsigned char> message;

		// midiIn->getMessage returns empty vector if there are no messages in the queue

		dynamic_cast<RtMidiIn *>(rtMidi)->getMessage(&message);
		while (message.size() > 0) {
			processMidi(message);
			dynamic_cast<RtMidiIn *>(rtMidi)->getMessage(&message);
		}
	}

	for (int i = 0; i < NUM_OUTPUTS; i++) {
		outputs[i].value = cc[i] / 127.0 * 10.0;
		lights[i] = 2.0 * outputs[i].value / 10.0 - 1.0;
	}
}


void MIDICCToCVInterface::processMidi(std::vector<unsigned char> msg) {
	int channel = msg[0] & 0xf;
	int status = (msg[0] >> 4) & 0xf;
	int data1 = msg[1];
	int data2 = msg[2];

	//fprintf(stderr, "channel %d status %d data1 %d data2 %d\n", channel, status, data1,data2);

	// Filter channels
	if (this->channel >= 0 && this->channel != channel)
		return;

	if (status == 0xb) {
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			if (data1 == ccNum[i]) {
				this->cc[i] = data2;
			}

		}
	}
}


struct CCTextField : TextField {
	void draw(NVGcontext *vg);

	int *ccNum;
	bool *inited;
};


void CCTextField::draw(NVGcontext *vg) {
	// Note: this might not be the best way to do this.
	// Text field should have a virtual "onTextChange" function or something.
	// draw() is triggered way more frequently
	if (text.size() > 0) {
		if (*inited) {
			*inited = false;
			text = std::to_string(*ccNum);
		}
		try {
			*ccNum = std::stoi(text, NULL, 10);
			// Only allow valid cc numbers
			if (*ccNum < 0 || *ccNum > 127) {
				text = "";
				begin = 0;
				end = text.size();
			}
		} catch (...) {
			text = "";
			begin = 0;
			end = text.size();
		}
	};
	TextField::draw(vg);
}

MIDICCToCVWidget::MIDICCToCVWidget() {
	MIDICCToCVInterface *module = new MIDICCToCVInterface();
	setModule(module);
	box.size = Vec(16 * 15, 380);

	{
		Panel *panel = new LightPanel();
		panel->box.size = box.size;
		addChild(panel);
	}

	float margin = 5;
	float labelHeight = 15;
	float yPos = margin;

	addChild(createScrew<ScrewSilver>(Vec(margin, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 15 - margin, 0)));
	addChild(createScrew<ScrewSilver>(Vec(margin, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 15 - margin, 365)));
	{
		Label *label = new Label();
		label->box.pos = Vec(box.size.x - margin - 11 * 15, margin);
		label->text = "MIDI CC to CV";
		addChild(label);
		yPos = labelHeight * 2;

	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "MIDI Interface";
		addChild(label);

		MidiChoice *midiChoice = new MidiChoice();
		midiChoice->midiModule = dynamic_cast<MidiIO *>(module);
		midiChoice->box.pos = Vec((box.size.x - 10) / 2 + margin, yPos);
		midiChoice->box.size.x = (box.size.x / 2.0) - margin;
		addChild(midiChoice);
		yPos += midiChoice->box.size.y + margin;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Channel";
		addChild(label);

		ChannelChoice *channelChoice = new ChannelChoice();
		channelChoice->midiModule = dynamic_cast<MidiIO *>(module);
		channelChoice->box.pos = Vec((box.size.x - 10) / 2 + margin, yPos);
		channelChoice->box.size.x = (box.size.x / 2.0) - margin;
		addChild(channelChoice);
		yPos += channelChoice->box.size.y + margin * 2;
	}

	for (int i = 0; i < MIDICCToCVInterface::NUM_OUTPUTS; i++) {
		CCTextField *ccNumChoice = new CCTextField();
		ccNumChoice->ccNum = &module->ccNum[i];
		ccNumChoice->inited = &module->ccNumInited[i];
		ccNumChoice->text = std::to_string(module->ccNum[i]);
		ccNumChoice->box.pos = Vec(10 + (i % 4) * (63), yPos);
		ccNumChoice->box.size.x = 15 * 2;

		addChild(ccNumChoice);

		yPos += labelHeight + margin;
		addOutput(createOutput<PJ3410Port>(Vec((i % 4) * (63) + 10, yPos + 5), module, i));
		addChild(createValueLight<SmallLight<GreenValueLight>>(Vec((i % 4) * (63) + 32, yPos + 5), &module->lights[i]));

		if ((i + 1) % 4 == 0) {
			yPos += 50 + margin;
		} else {
			yPos -= labelHeight + margin;
		}
	}


}

void MIDICCToCVWidget::step() {
	// Assume QWERTY
#define MIDI_KEY(key, midi) if (glfwGetKey(gWindow, key)) printf("%d\n", midi);

	// MIDI_KEY(GLFW_KEY_Z, 48);

	ModuleWidget::step();
}
