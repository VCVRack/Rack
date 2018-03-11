#include "Core.hpp"
#include "midi.hpp"

#include <algorithm>


struct QuadMIDIToCVInterface : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(CV_OUTPUT, 4),
		ENUMS(GATE_OUTPUT, 4),
		ENUMS(VELOCITY_OUTPUT, 4),
		ENUMS(AFTERTOUCH_OUTPUT, 4),
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	MidiInputQueue midiInput;

	enum PolyMode {
		ROTATE_MODE,
		RESET_MODE,
		REASSIGN_MODE,
		UNISON_MODE,
		NUM_MODES
	};
	PolyMode polyMode = ROTATE_MODE;

	struct NoteData {
		uint8_t velocity = 0;
		uint8_t aftertouch = 0;
	};

	NoteData noteData[128];
	std::vector<uint8_t> heldNotes;
	uint8_t notes[4];
	bool gates[4];
	bool pedal;

	QuadMIDIToCVInterface() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS), heldNotes(128) {
		onReset();
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "midi", midiInput.toJson());
		json_object_set_new(rootJ, "polyMode", json_integer(polyMode));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiInput.fromJson(midiJ);

		json_t *polyModeJ = json_object_get(rootJ, "polyMode");
		if (polyModeJ)
			polyMode = (PolyMode) json_integer_value(polyModeJ);
	}

	void onReset() override {
		for (int i = 0; i < 4; i++) {
			notes[i] = 60;
			gates[i] = false;
		}
		pedal = false;
	}

	void pressNote(uint8_t note) {
		// Remove existing similar note
		auto it = std::find(heldNotes.begin(), heldNotes.end(), note);
		if (it != heldNotes.end())
			heldNotes.erase(it);
		// Push note
		heldNotes.push_back(note);

		// Set notes and gates
		switch (polyMode) {
			case ROTATE_MODE: {

			} break;
			case RESET_MODE: {

			} break;
			case REASSIGN_MODE: {

			} break;
			case UNISON_MODE: {

			} break;
			default: break;
		}
	}

	void releaseNote(uint8_t note) {
		// Remove the note
		auto it = std::find(heldNotes.begin(), heldNotes.end(), note);
		if (it != heldNotes.end())
			heldNotes.erase(it);
		// Hold note if pedal is pressed
		// if (pedal)
		// 	return;
		// // Set last note
		// if (!heldNotes.empty()) {
		// 	auto it2 = heldNotes.end();
		// 	it2--;
		// 	lastNote = *it2;
		// 	gate = true;
		// }
		// else {
		// 	gate = false;
		// }
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

		for (int i = 0; i < 4; i++) {
			uint8_t lastNote = notes[i];
			outputs[CV_OUTPUT + i].value = (lastNote - 60) / 12.f;
			outputs[GATE_OUTPUT + i].value = gates[i] ? 10.f : 0.f;
			outputs[VELOCITY_OUTPUT + i].value = rescale(noteData[lastNote].velocity, 0, 127, 0.f, 10.f);
			outputs[VELOCITY_OUTPUT + i].value = rescale(noteData[lastNote].aftertouch, 0, 127, 0.f, 10.f);
		}
	}

	void processMessage(MidiMessage msg) {
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
					releaseNote(msg.note());
				}
			} break;
			// channel aftertouch
			case 0xa: {
				noteData[msg.note()].aftertouch = msg.value();
			} break;
			// cc
			case 0xb: {
				processCC(msg);
			} break;
			default: break;
		}
	}

	void processCC(MidiMessage msg) {
		switch (msg.note()) {
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
};


struct QuadMIDIToCVInterfaceWidget : ModuleWidget {
	QuadMIDIToCVInterfaceWidget(QuadMIDIToCVInterface *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetGlobal("res/Core/QuadMIDIToCVInterface.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.894335, 60.144478)), Port::OUTPUT, module, QuadMIDIToCVInterface::CV_OUTPUT + 0));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.494659, 60.144478)), Port::OUTPUT, module, QuadMIDIToCVInterface::GATE_OUTPUT + 0));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.094986, 60.144478)), Port::OUTPUT, module, QuadMIDIToCVInterface::VELOCITY_OUTPUT + 0));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.693935, 60.144478)), Port::OUTPUT, module, QuadMIDIToCVInterface::AFTERTOUCH_OUTPUT + 0));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.894335, 76.144882)), Port::OUTPUT, module, QuadMIDIToCVInterface::CV_OUTPUT + 1));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.494659, 76.144882)), Port::OUTPUT, module, QuadMIDIToCVInterface::GATE_OUTPUT + 1));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.094986, 76.144882)), Port::OUTPUT, module, QuadMIDIToCVInterface::VELOCITY_OUTPUT + 1));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.693935, 76.144882)), Port::OUTPUT, module, QuadMIDIToCVInterface::AFTERTOUCH_OUTPUT + 1));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.894335, 92.143906)), Port::OUTPUT, module, QuadMIDIToCVInterface::CV_OUTPUT + 2));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.494659, 92.143906)), Port::OUTPUT, module, QuadMIDIToCVInterface::GATE_OUTPUT + 2));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.094986, 92.143906)), Port::OUTPUT, module, QuadMIDIToCVInterface::VELOCITY_OUTPUT + 2));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.693935, 92.143906)), Port::OUTPUT, module, QuadMIDIToCVInterface::AFTERTOUCH_OUTPUT + 2));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.894335, 108.1443)), Port::OUTPUT, module, QuadMIDIToCVInterface::CV_OUTPUT + 3));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.494659, 108.1443)), Port::OUTPUT, module, QuadMIDIToCVInterface::GATE_OUTPUT + 3));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.094986, 108.1443)), Port::OUTPUT, module, QuadMIDIToCVInterface::VELOCITY_OUTPUT + 3));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.693935, 108.1443)), Port::OUTPUT, module, QuadMIDIToCVInterface::AFTERTOUCH_OUTPUT + 3));

		MidiWidget *midiWidget = Widget::create<MidiWidget>(mm2px(Vec(3.4009969, 14.837336)));
		midiWidget->box.size = mm2px(Vec(44, 28));
		midiWidget->midiIO = &module->midiInput;
		addChild(midiWidget);
	}
};


Model *modelQuadMIDIToCVInterface = Model::create<QuadMIDIToCVInterface, QuadMIDIToCVInterfaceWidget>("Core", "QuadMIDIToCVInterface", "MIDI-4", MIDI_TAG, EXTERNAL_TAG, QUAD_TAG);

