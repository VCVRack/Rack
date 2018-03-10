#include "Core.hpp"
#include "midi.hpp"
#include "dsp/filter.hpp"

#include <algorithm>


struct MidiNoteData {
	uint8_t velocity = 0;
	uint8_t aftertouch = 0;
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

	MidiInputQueue midiInput;

	uint8_t mod = 0;
	ExponentialFilter modFilter;
	uint16_t pitch = 0;
	ExponentialFilter pitchFilter;

	MidiNoteData noteData[128];
	std::list<uint8_t> heldNotes;
	uint8_t lastNote;
	bool pedal;
	bool gate;

	MIDIToCVInterface() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void onReset() override {
		heldNotes.clear();
		pedal = false;
		lastNote = 60;
	}

	void pressNote(uint8_t note) {
		// Remove existing similar note
		auto it = std::find(heldNotes.begin(), heldNotes.end(), note);
		if (it != heldNotes.end())
			heldNotes.erase(it);
		// Push note
		heldNotes.push_back(note);
		lastNote = note;
		gate = true;
	}

	void releaseNote(uint8_t note) {
		// Remove the note
		if (note < 128) {
			auto it = std::find(heldNotes.begin(), heldNotes.end(), note);
			if (it != heldNotes.end())
				heldNotes.erase(it);
		}
		// Hold note if pedal is pressed
		if (pedal)
			return;
		// Set last note
		if (!heldNotes.empty()) {
			auto it2 = heldNotes.end();
			it2--;
			lastNote = *it2;
			gate = true;
		}
		else {
			gate = false;
		}
	}

	void pressPedal() {
		pedal = true;
	}

	void releasePedal() {
		pedal = false;
		releaseNote(255);
	}

	void step() override {
		MidiMessage msg;
		while (midiInput.shift(&msg)) {
			processMessage(msg);
		}

		outputs[CV_OUTPUT].value = (lastNote - 60) / 12.f;
		outputs[GATE_OUTPUT].value = gate ? 10.f : 0.f;
		outputs[VELOCITY_OUTPUT].value = rescale(noteData[lastNote].velocity, 0, 127, 0.f, 10.f);
		modFilter.lambda = 100.f * engineGetSampleTime();
		outputs[MOD_OUTPUT].value = modFilter.process(rescale(mod, 0, 127, 0.f, 10.f));

		pitchFilter.lambda = 100.f * engineGetSampleTime();
		outputs[PITCH_OUTPUT].value = pitchFilter.process(rescale(pitch, 0, 16384, -5.f, 5.f));

		outputs[AFTERTOUCH_OUTPUT].value = rescale(noteData[lastNote].aftertouch, 0, 127, 0.f, 10.f);
	}

	void processMessage(MidiMessage msg) {
		switch (msg.status()) {
			// note off
			case 0x8: {
				releaseNote(msg.data1);
			} break;
			// note on
			case 0x9: {
				if (msg.data2 > 0) {
					uint8_t note = msg.data1 & 0x7f;
					noteData[note].velocity = msg.data2;
					pressNote(msg.data1);
				}
				else {
					// For some reason, some keyboards send a "note on" event with a velocity of 0 to signal that the key has been released.
					releaseNote(msg.data1);
				}
			} break;
			// channel aftertouch
			case 0xa: {
				uint8_t note = msg.data1 & 0x7f;
				noteData[note].aftertouch = msg.data2;
			} break;
			// cc
			case 0xb: {
				processCC(msg);
			} break;
			// pitch wheel
			case 0xe: {
				pitch = msg.data2 * 128 + msg.data1;
			} break;
			case 0xf: {
				processSystem(msg);
			} break;
			default: break;
		}
	}

	void processCC(MidiMessage msg) {
		switch (msg.data1) {
			// mod
			case 0x01: {
				mod = msg.data2;
			} break;
			// sustain
			case 0x40: {
				if (msg.data2 >= 64)
					pressPedal();
				else
					releasePedal();
			} break;
			default: break;
		}
	}

	void processSystem(MidiMessage msg) {
		switch (msg.channel()) {
			case 0x8: {
				debug("timing clock");
			} break;
			case 0xa: {
				debug("start");
			} break;
			case 0xb: {
				debug("continue");
			} break;
			case 0xc: {
				debug("stop");
			} break;
			default: break;
		}
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "midi", midiInput.toJson());
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *midiJ = json_object_get(rootJ, "midi");
		midiInput.fromJson(midiJ);
	}
};


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
		midiWidget->midiIO = &module->midiInput;
		addChild(midiWidget);
	}
};


Model *modelMIDIToCVInterface = Model::create<MIDIToCVInterface, MIDIToCVInterfaceWidget>("Core", "MIDIToCVInterface", "MIDI-1", MIDI_TAG, EXTERNAL_TAG);
