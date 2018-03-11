#include "Core.hpp"
#include "midi.hpp"
#include "dsp/digital.hpp"
#include "dsp/filter.hpp"

#include <algorithm>


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
	PulseGenerator startPulse;
	PulseGenerator stopPulse;
	PulseGenerator continuePulse;
	PulseGenerator clockPulse;

	struct NoteData {
		uint8_t velocity = 0;
		uint8_t aftertouch = 0;
	};

	NoteData noteData[128];
	std::vector<uint8_t> heldNotes;
	uint8_t lastNote;
	bool pedal;
	bool gate;

	MIDIToCVInterface() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS), heldNotes(128) {
		onReset();
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "midi", midiInput.toJson());
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiInput.fromJson(midiJ);
	}

	void onReset() override {
		heldNotes.clear();
		lastNote = 60;
		pedal = false;
		gate = false;
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
		auto it = std::find(heldNotes.begin(), heldNotes.end(), note);
		if (it != heldNotes.end())
			heldNotes.erase(it);
		// Hold note if pedal is pressed
		if (pedal)
			return;
		// Set last note
		if (!heldNotes.empty()) {
			lastNote = heldNotes[heldNotes.size() - 1];
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
		float deltaTime = engineGetSampleTime();

		outputs[CV_OUTPUT].value = (lastNote - 60) / 12.f;
		outputs[GATE_OUTPUT].value = gate ? 10.f : 0.f;
		outputs[VELOCITY_OUTPUT].value = rescale(noteData[lastNote].velocity, 0, 127, 0.f, 10.f);
		modFilter.lambda = 100.f * deltaTime;
		outputs[MOD_OUTPUT].value = modFilter.process(rescale(mod, 0, 127, 0.f, 10.f));

		pitchFilter.lambda = 100.f * deltaTime;
		outputs[PITCH_OUTPUT].value = pitchFilter.process(rescale(pitch, 0, 16384, -5.f, 5.f));

		outputs[AFTERTOUCH_OUTPUT].value = rescale(noteData[lastNote].aftertouch, 0, 127, 0.f, 10.f);

		outputs[START_OUTPUT].value = startPulse.process(deltaTime) ? 10.f : 0.f;
		outputs[STOP_OUTPUT].value = stopPulse.process(deltaTime) ? 10.f : 0.f;
		outputs[CONTINUE_OUTPUT].value = continuePulse.process(deltaTime) ? 10.f : 0.f;

		outputs[CLOCK_OUTPUT].value = clockPulse.process(deltaTime) ? 10.f : 0.f;
		// TODO
		outputs[CLOCK_2_OUTPUT].value = 0.f;
		outputs[CLOCK_HALF_OUTPUT].value = 0.f;
	}

	void processMessage(MidiMessage msg) {
		// debug("MIDI: %01x %01x %02x %02x", msg.status(), msg.channel(), msg.note(), msg.value());

		switch (msg.status()) {
			// note off
			case 0x8: {
				releaseNote(msg.note());
			} break;
			// note on
			case 0x9: {
				if (msg.value() > 0) {
					noteData[msg.note()].velocity = msg.value();
					pressNote(msg.note());
				}
				else {
					// For some reason, some keyboards send a "note on" event with a velocity of 0 to signal that the key has been released.
					releaseNote(msg.note());
				}
			} break;
			// channel aftertouch
			case 0xa: {
				uint8_t note = msg.note();
				noteData[note].aftertouch = msg.value();
			} break;
			// cc
			case 0xb: {
				processCC(msg);
			} break;
			// pitch wheel
			case 0xe: {
				pitch = msg.value() * 128 + msg.note();
			} break;
			case 0xf: {
				processSystem(msg);
			} break;
			default: break;
		}
	}

	void processCC(MidiMessage msg) {
		switch (msg.note()) {
			// mod
			case 0x01: {
				mod = msg.value();
			} break;
			// sustain
			case 0x40: {
				if (msg.value() >= 64)
					pressPedal();
				else
					releasePedal();
			} break;
			default: break;
		}
	}

	void processSystem(MidiMessage msg) {
		switch (msg.channel()) {
			// Timing
			case 0x8: {
				clockPulse.trigger(1e-3);
			} break;
			// Start
			case 0xa: {
				startPulse.trigger(1e-3);
			} break;
			// Continue
			case 0xb: {
				continuePulse.trigger(1e-3);
			} break;
			// Stop
			case 0xc: {
				stopPulse.trigger(1e-3);
			} break;
			default: break;
		}
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
