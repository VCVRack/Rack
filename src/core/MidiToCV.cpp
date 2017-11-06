#include <list>
#include <algorithm>
#include "rtmidi/RtMidi.h"
#include "core.hpp"
#include "MidiIO.hpp"
#include "dsp/digital.hpp"

/*
 * MIDIToCVInterface converts midi note on/off events, velocity , channel aftertouch, pitch wheel and mod wheel to
 * CV
 */
struct MidiValue {
	int val = 0; // Controller value
	TransitionSmoother tSmooth;
	bool changed = false; // Value has been changed by midi message (only if it is in sync!)
};

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
	enum LightIds {
		RESET_LIGHT,
		NUM_LIGHTS
	};

	std::list<int> notes;
	bool pedal = false;
	int note = 60; // C4, most modules should use 261.626 Hz
	int vel = 0;
	MidiValue mod;
	MidiValue afterTouch;
	MidiValue pitchWheel;
	bool gate = false;

	SchmittTrigger resetTrigger;

	MIDIToCVInterface() : MidiIO(), Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		pitchWheel.val = 64;
		pitchWheel.tSmooth.set(0, 0);
	}

	~MIDIToCVInterface() {
	};

	void step() override;

	void pressNote(int note);

	void releaseNote(int note);

	void processMidi(std::vector<unsigned char> msg);

	json_t *toJson() override {
		json_t *rootJ = json_object();
		addBaseJson(rootJ);
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		baseFromJson(rootJ);
	}

	void reset() override {
		resetMidi();
	}

	void resetMidi() override;

};

void MIDIToCVInterface::resetMidi() {
	mod.val = 0;
	mod.tSmooth.set(0, 0);
	pitchWheel.val = 64;
	pitchWheel.tSmooth.set(0, 0);
	afterTouch.val = 0;
	afterTouch.tSmooth.set(0, 0);
	vel = 0;
	gate = false;
	notes.clear();
}

void MIDIToCVInterface::step() {
	if (isPortOpen()) {
		std::vector<unsigned char> message;

		// midiIn->getMessage returns empty vector if there are no messages in the queue
		getMessage(&message);
		while (message.size() > 0) {
			processMidi(message);
			getMessage(&message);
		}
	}

	outputs[PITCH_OUTPUT].value = ((note - 60)) / 12.0;

	if (resetTrigger.process(params[RESET_PARAM].value)) {
		resetMidi();
		return;
	}

	lights[RESET_LIGHT].value -= lights[RESET_LIGHT].value / 0.55 / engineGetSampleRate(); // fade out light

	outputs[GATE_OUTPUT].value = gate ? 10.0 : 0.0;
	outputs[VELOCITY_OUTPUT].value = vel / 127.0 * 10.0;

	int steps = int(engineGetSampleRate() / 32);

	if (mod.changed) {
		mod.tSmooth.set(outputs[MOD_OUTPUT].value, (mod.val / 127.0 * 10.0), steps);
		mod.changed = false;
	}
	outputs[MOD_OUTPUT].value = mod.tSmooth.next();

	if (pitchWheel.changed) {
		pitchWheel.tSmooth.set(outputs[PITCHWHEEL_OUTPUT].value, (pitchWheel.val - 64) / 64.0 * 10.0, steps);
		pitchWheel.changed = false;
	}
	outputs[PITCHWHEEL_OUTPUT].value = pitchWheel.tSmooth.next();


	/* NOTE: I'll leave out value smoothing for after touch for now. I currently don't
	 * have an after touch capable device around and I assume it would require different
	 * smoothing*/
	outputs[CHANNEL_AFTERTOUCH_OUTPUT].value = afterTouch.val / 127.0 * 10.0;
}

void MIDIToCVInterface::pressNote(int note) {
	// Remove existing similar note
	auto it = std::find(notes.begin(), notes.end(), note);
	if (it != notes.end())
		notes.erase(it);
	// Push note
	notes.push_back(note);
	this->note = note;
	gate = true;
}

void MIDIToCVInterface::releaseNote(int note) {
	// Remove the note
	auto it = std::find(notes.begin(), notes.end(), note);
	if (it != notes.end())
		notes.erase(it);

	if (pedal) {
		// Don't release if pedal is held
		gate = true;
	} else if (!notes.empty()) {
		// Play previous note
		auto it2 = notes.end();
		it2--;
		this->note = *it2;
		gate = true;
	} else {
		gate = false;
	}
}

void MIDIToCVInterface::processMidi(std::vector<unsigned char> msg) {
	int channel = msg[0] & 0xf;
	int status = (msg[0] >> 4) & 0xf;
	int data1 = msg[1];
	int data2 = msg[2];

	// fprintf(stderr, "channel %d status %d data1 %d data2 %d\n", channel, status, data1, data2);

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
					mod.val = data2;
					mod.changed = true;
					break;
				case 0x40: // sustain
					pedal = (data2 >= 64);
					if (!pedal) {
						releaseNote(-1);
					}
					break;
			}
			break;
		case 0xe: // pitch wheel
			pitchWheel.val = data2;
			pitchWheel.changed = true;
			break;
		case 0xd: // channel aftertouch
			afterTouch.val = data1;
			afterTouch.changed = true;
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
	addChild(createLight<SmallLight<RedLight>>(Vec(7 * 15 + 5, labelHeight + 5), module,
											   MIDIToCVInterface::RESET_LIGHT));
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
