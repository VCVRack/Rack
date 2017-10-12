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

	virtual void resetMidi()=0; // called when midi port is set
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
		midiModule->resetMidi(); // reset Midi values
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
		if (midiModule->portId < 0) {
			text = "No Device";
			return;
		}
		std::string name = midiModule->getPortName(midiModule->portId);
		text = ellipsize(name, 15);
	}
};

struct ChannelItem : MenuItem {
	MidiIO *midiModule;
	int channel;

	void onAction() {
		midiModule->resetMidi(); // reset Midi values
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
		RESET_PARAM,
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

	SchmittTrigger resetTrigger;
	float resetLight = 0.0;

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

	virtual void resetMidi();

};

void MIDIToCVInterface::resetMidi() {
	mod = 0;
	pitchWheel = 64;
	afterTouch = 0;
	vel = 0;
	resetLight = 1.0;
	outputs[GATE_OUTPUT].value = 0.0;
	notes.clear();
}

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
	if (resetTrigger.process(params[RESET_PARAM].value)) {
		resetMidi();
		return;
	}

	if (resetLight > 0) {
		resetLight -= resetLight / 0.55 / gSampleRate; // fade out light
	}


	outputs[GATE_OUTPUT].value = gate ? 10.0 : 0.0;
	outputs[MOD_OUTPUT].value = mod / 127.0 * 10.0;
	outputs[PITCHWHEEL_OUTPUT].value = (pitchWheel - 64) / 64.0 * 10.0;
	outputs[CHANNEL_AFTERTOUCH_OUTPUT].value = afterTouch / 127.0 * 10.0;
	outputs[VELOCITY_OUTPUT].value = vel / 127.0 * 10.0;

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

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 365)));

	{
		Label *label = new Label();
		label->box.pos = Vec(box.size.x - margin - 7 * 15, margin);
		label->text = "MIDI to CV";
		addChild(label);
		yPos = labelHeight * 2;
	}

	addParam(createParam<LEDButton>(Vec(7 * 15, labelHeight), module, MIDIToCVInterface::RESET_PARAM, 0.0, 1.0, 0.0));
	addChild(createValueLight<SmallLight<RedValueLight>>(Vec(7 * 15 + 5, labelHeight + 5), &module->resetLight));
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

	std::string labels[MIDIToCVInterface::NUM_OUTPUTS] = {"1V/oct", "Gate", "Velocity", "Mod Wheel", "Pitch Wheel",
														  "Aftertouch"};

	for (int i = 0; i < MIDIToCVInterface::NUM_OUTPUTS; i++) {
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = labels[i];
		addChild(label);

		addOutput(createOutput<PJ3410Port>(Vec(15 * 6, yPos - 5), module, i));

		yPos += yGap + margin;
	}
}

void MidiToCVWidget::step() {

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

	virtual void resetMidi();

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
	}
}

void MIDICCToCVInterface::resetMidi() {
	for (int i = 0; i < NUM_OUTPUTS; i++) {
		cc[i] = 0;
	}
};

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
	void onTextChange();

	void draw(NVGcontext *vg);

	int *ccNum;
	bool *inited;
};

void CCTextField::draw(NVGcontext *vg) {
	/* This is necessary, since the save
	 * file is loaded after constructing the widget*/
	if (*inited) {
		*inited = false;
		text = std::to_string(*ccNum);
	}

	TextField::draw(vg);
}

void CCTextField::onTextChange() {
	if (text.size() > 0) {
		try {
			*ccNum = std::stoi(text);
			// Only allow valid cc numbers
			if (*ccNum < 0 || *ccNum > 127 || text.size() > 3) {
				text = "";
				begin = end = 0;
				*ccNum = -1;
			}
		} catch (...) {
			text = "";
			begin = end = 0;
			*ccNum = -1;
		}
	};
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

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 365)));
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
		yPos += channelChoice->box.size.y + margin * 3;
	}

	for (int i = 0; i < MIDICCToCVInterface::NUM_OUTPUTS; i++) {
		CCTextField *ccNumChoice = new CCTextField();
		ccNumChoice->ccNum = &module->ccNum[i];
		ccNumChoice->inited = &module->ccNumInited[i];
		ccNumChoice->text = std::to_string(module->ccNum[i]);
		ccNumChoice->box.pos = Vec(11 + (i % 4) * (63), yPos);
		ccNumChoice->box.size.x = 29;

		addChild(ccNumChoice);

		yPos += labelHeight + margin;
		addOutput(createOutput<PJ3410Port>(Vec((i % 4) * (63) + 10, yPos + 5), module, i));

		if ((i + 1) % 4 == 0) {
			yPos += 47 + margin;
		} else {
			yPos -= labelHeight + margin;
		}
	}
}

void MIDICCToCVWidget::step() {

	ModuleWidget::step();
}

struct MIDIClockToCVInterface : MidiIO, Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		CLOCK1_PULSE,
		CLOCK2_PULSE,
		CLOCK_START_PULSE,
		CLOCK_STOP_PULSE,
		NUM_OUTPUTS
	};

	int clock1ratio = 0;
	int clock2ratio = 0;

	PulseGenerator clock1Pulse;
	PulseGenerator clock2Pulse;
	PulseGenerator clockStartPulse;
	PulseGenerator clockStopPulse;
	bool tick = false;
	bool running = false;
	bool start = false;
	bool stop = false;

	MIDIClockToCVInterface() : MidiIO(), Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
		(dynamic_cast<RtMidiIn *>(rtMidi))->ignoreTypes(true, false);
	}

	~MIDIClockToCVInterface() {
		setPortId(-1);
	}

	void step();

	void processMidi(std::vector<unsigned char> msg);

	virtual void resetMidi();

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

void MIDIClockToCVInterface::step() {
	static int c1_16th = 0;
	static int c2_16th = 0;

	/* Note this is in relation to the Midi clock's Tick (6x per 16th note).
	 * Therefore, e.g. the 2:3 is calculated:
	 *
	 * 24 (Ticks per quarter note) * 2 / 3 = 16
	 *
	 * Implying that every 16 midi clock ticks we need to send a pulse
	 * */
	static int ratios[] = {6, 8, 12, 16, 24, 32, 48, 96, 192};

	if (rtMidi->isPortOpen()) {
		std::vector<unsigned char> message;

		// midiIn->getMessage returns empty vector if there are no messages in the queue

		dynamic_cast<RtMidiIn *>(rtMidi)->getMessage(&message);
		while (message.size() > 0) {
			processMidi(message);
			dynamic_cast<RtMidiIn *>(rtMidi)->getMessage(&message);
		}

	}

	if(start) {
		clockStartPulse.trigger(0.1);
		start=false;
		c1_16th = 0;
		c2_16th = 0;
	}

	if(stop) {
		clockStopPulse.trigger(0.1);
		start=false;
	}

	if (running && tick) {
		tick = false;
		c1_16th++;
		c2_16th++;

		if (c1_16th % ratios[clock1ratio] == 0) {
			c1_16th = 0;
			clock1Pulse.trigger(0.1);
		}

		if (c2_16th % ratios[clock2ratio] == 0) {
			c2_16th = 0;
			clock2Pulse.trigger(0.1);
		}
	}


	bool pulse = clock1Pulse.process(1.0 / gSampleRate);
	outputs[CLOCK1_PULSE].value = pulse ? 10.0 : 0.0;

	pulse = clock2Pulse.process(1.0 / gSampleRate);
	outputs[CLOCK2_PULSE].value = pulse ? 10.0 : 0.0;

	pulse = clockStartPulse.process(1.0 / gSampleRate);
	outputs[CLOCK_START_PULSE].value = pulse ? 10.0 : 0.0;

	pulse = clockStopPulse.process(1.0 / gSampleRate);
	outputs[CLOCK_STOP_PULSE].value = pulse ? 10.0 : 0.0;

}

void MIDIClockToCVInterface::resetMidi() {
	outputs[CLOCK1_PULSE].value = 0.0;
}

void MIDIClockToCVInterface::processMidi(std::vector<unsigned char> msg) {

	switch (msg[0]) {
		case 0xfa:
			start = true;
			running = true;
			break;
		case 0xfc:
			stop = true;
			running = false;
			break;
		case 0xf8:
			tick = true;
			break;
	}


}

struct ClockRatioItem : MenuItem {
	int ratio;
	int *clockRatio;

	void onAction() {
		*clockRatio = ratio;
	}
};

struct ClockRatioChoice : ChoiceButton {
	int *clockRatio;
	const std::vector<std::string> ratioNames = {"Sixteenth note (1:4 ratio)", "Eighth note triplet (1:3 ratio)",
												 "Eighth note (1:2 ratio)", "Quarter note triplet (2:3 ratio)",
												 "Quarter note (tap speed)", "Half note triplet (4:3 ratio)",
												 "Half note (2:1 ratio)", "Whole note (4:1 ratio)",
												 "Two whole notes (8:1 ratio)"};

	const std::vector<std::string> ratioNames_short = {"1:4 ratio", "1:3 ratio", "1:2 ratio", "2:3 ratio", "1:1 ratio",
													   "4:3", "2:1 ratio", "4:1 ratio", "8:1 ratio"};

	void onAction() {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		for (int ratio = 0; ratio < ratioNames.size(); ratio++) {
			ClockRatioItem *clockRatioItem = new ClockRatioItem();
			clockRatioItem->ratio = ratio;
			clockRatioItem->clockRatio = clockRatio;
			clockRatioItem->text = ratioNames[ratio];
			menu->pushChild(clockRatioItem);
		}
	}

	void step() {
		text = ratioNames_short[*clockRatio];
	}
};

MIDIClockToCVWidget::MIDIClockToCVWidget() {
	MIDIClockToCVInterface *module = new MIDIClockToCVInterface();
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

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 365)));

	{
		Label *label = new Label();
		label->box.pos = Vec(box.size.x - margin - 7 * 15, margin);
		label->text = "MIDI Clock to CV";
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
		yPos += midiChoice->box.size.y + margin * 6;
	}



	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Clock 1 Ratio";
		addChild(label);
		yPos += labelHeight + margin;

		ClockRatioChoice *ratioChoice = new ClockRatioChoice();
		ratioChoice->clockRatio = &module->clock1ratio;
		ratioChoice->box.pos = Vec(margin, yPos);
		ratioChoice->box.size.x = box.size.x - 10;
		addChild(ratioChoice);
		yPos += ratioChoice->box.size.y + margin+5;

	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Clock 1 Pulse";
		addChild(label);

		addOutput(createOutput<PJ3410Port>(Vec(15 * 6, yPos - 5), module, MIDIClockToCVInterface::CLOCK1_PULSE));
		yPos += labelHeight + margin*4;
	}


	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Clock 2 Ratio";
		addChild(label);
		yPos += labelHeight + margin;

		ClockRatioChoice *ratioChoice = new ClockRatioChoice();
		ratioChoice->clockRatio = &module->clock2ratio;
		ratioChoice->box.pos = Vec(margin, yPos);
		ratioChoice->box.size.x = box.size.x - 10;
		addChild(ratioChoice);
		yPos += ratioChoice->box.size.y + margin+5;

	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Clock 2 Pulse";
		addChild(label);

		addOutput(createOutput<PJ3410Port>(Vec(15 * 6, yPos - 5), module, MIDIClockToCVInterface::CLOCK2_PULSE));
		yPos += labelHeight + margin*7;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Clock Start";
		addChild(label);
		addOutput(createOutput<PJ3410Port>(Vec(15*6, yPos - 5), module, MIDIClockToCVInterface::CLOCK_START_PULSE));
		yPos += 40;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Clock Stop";
		addChild(label);

		addOutput(createOutput<PJ3410Port>(Vec(15*6, yPos - 5), module, MIDIClockToCVInterface::CLOCK_STOP_PULSE));
	}
}

void MIDIClockToCVWidget::step() {

	ModuleWidget::step();
}