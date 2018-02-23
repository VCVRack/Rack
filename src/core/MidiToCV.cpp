#include <list>
#include <algorithm>
#include "core.hpp"
#include "midi.hpp"
#include "dsp/digital.hpp"


/*
 * MIDIToCVInterface converts midi note on/off events, velocity , channel aftertouch, pitch wheel and mod wheel to
 * CV
 */
struct MidiValue {
	int val = 0; // Controller value
	// TransitionSmoother tSmooth;
	bool changed = false; // Value has been changed by midi message (only if it is in sync!)
};


struct MIDIToCVInterface : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		CV_OUTPUT,
		GATE_OUTPUT,
		VELOCITY_OUTPUT,
		MOD_OUTPUT,
		PITCH_OUTPUT,
		AFTERTOUCH_OUTPUT,
		START_OUTPUT,
		STOP_OUTPUT,
		CONTINUE_OUTPUT,
		CLOCK_OUTPUT,
		CLOCK_2_OUTPUT,
		CLOCK_HALF_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	MidiInput midiIO;

	MidiInputQueue midiInput;
	std::list<int> notes;
	bool pedal = false;
	int note = 60; // C4, most modules should use 261.626 Hz
	int vel = 0;
	MidiValue mod;
	MidiValue afterTouch;
	MidiValue pitchWheel;
	bool gate = false;

	SchmittTrigger resetTrigger;

	MIDIToCVInterface() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		pitchWheel.val = 64;
		// pitchWheel.tSmooth.set(0, 0);
	}

	~MIDIToCVInterface() {
	};

	void step() override;

	void pressNote(int note);

	void releaseNote(int note);

	void processMidi(std::vector<unsigned char> msg);

	json_t *toJson() override {
		json_t *rootJ = json_object();
		// addBaseJson(rootJ);
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// baseFromJson(rootJ);
	}

	void onReset() override {
		// resetMidi();
	}

	// void resetMidi() override;
};

/*
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
*/

void MIDIToCVInterface::step() {
	/*
	if (isPortOpen()) {
		std::vector<unsigned char> message;

		// midiIn->getMessage returns empty vector if there are no messages in the queue
		getMessage(&message);
		if (message.size() > 0) {
			processMidi(message);
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

	outputs[CHANNEL_AFTERTOUCH_OUTPUT].value = afterTouch.val / 127.0 * 10.0;
	*/
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
	/*
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
	*/
}


struct MIDIToCVInterfaceWidget : ModuleWidget {
	MIDIToCVInterfaceWidget(MIDIToCVInterface *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetGlobal("res/Core/MIDIToCVInterface.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addOutput(Port::create<PJ301MPort>(mm2px(Vec(4.61505, 60.1445)), Port::OUTPUT, module, MIDIToCVInterface::CV_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(16.214, 60.1445)), Port::OUTPUT, module, MIDIToCVInterface::GATE_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.8143, 60.1445)), Port::OUTPUT, module, MIDIToCVInterface::VELOCITY_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(4.61505, 76.1449)), Port::OUTPUT, module, MIDIToCVInterface::MOD_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(16.214, 76.1449)), Port::OUTPUT, module, MIDIToCVInterface::PITCH_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.8143, 76.1449)), Port::OUTPUT, module, MIDIToCVInterface::AFTERTOUCH_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(4.61505, 92.1439)), Port::OUTPUT, module, MIDIToCVInterface::START_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(16.214, 92.1439)), Port::OUTPUT, module, MIDIToCVInterface::STOP_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.8143, 92.1439)), Port::OUTPUT, module, MIDIToCVInterface::CONTINUE_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(4.61505, 108.144)), Port::OUTPUT, module, MIDIToCVInterface::CLOCK_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(16.214, 108.144)), Port::OUTPUT, module, MIDIToCVInterface::CLOCK_2_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.8143, 108.144)), Port::OUTPUT, module, MIDIToCVInterface::CLOCK_HALF_OUTPUT));

		MidiWidget *midiWidget = Widget::create<MidiWidget>(mm2px(Vec(3.41891, 14.8373)));
		midiWidget->box.size = mm2px(Vec(33.840, 28));
		midiWidget->midiIO = &module->midiIO;
		addChild(midiWidget);
	}
};


Model *modelMidiToCvInterface = Model::create<MIDIToCVInterface, MIDIToCVInterfaceWidget>("Core", "MIDIToCVInterface", "MIDI-1", MIDI_TAG, EXTERNAL_TAG);
