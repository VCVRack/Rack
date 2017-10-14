#include <list>
#include <algorithm>
#include "rtmidi/RtMidi.h"
#include "core.hpp"
#include "MidiInterface.hpp"
#include "dsp/digital.hpp"

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
	if (isPortOpen()) {
		std::vector<unsigned char> message;

		// midiIn->getMessage returns empty vector if there are no messages in the queue
		message = getMessage();
		while (message.size() > 0) {
			processMidi(message);
			message = getMessage();
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
