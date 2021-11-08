#include "plugin.hpp"


namespace rack {
namespace core {


struct GateMidiOutput : midi::Output {
	int vels[128];
	bool lastGates[128];
	double frame = 0.0;

	GateMidiOutput() {
		reset();
	}

	void reset() {
		for (int note = 0; note < 128; note++) {
			vels[note] = 100;
			lastGates[note] = false;
		}
		Output::reset();
	}

	void panic() {
		// Send all note off commands
		for (int note = 0; note < 128; note++) {
			// Note off
			midi::Message m;
			m.setStatus(0x8);
			m.setNote(note);
			m.setValue(0);
			m.setFrame(frame);
			sendMessage(m);
			lastGates[note] = false;
		}
	}

	void setVelocity(int vel, int note) {
		vels[note] = vel;
	}

	void setGate(bool gate, int note) {
		if (gate && !lastGates[note]) {
			// Note on
			midi::Message m;
			m.setStatus(0x9);
			m.setNote(note);
			m.setValue(vels[note]);
			m.setFrame(frame);
			sendMessage(m);
		}
		else if (!gate && lastGates[note]) {
			// Note off
			midi::Message m;
			m.setStatus(0x8);
			m.setNote(note);
			m.setValue(vels[note]);
			m.setFrame(frame);
			sendMessage(m);
		}
		lastGates[note] = gate;
	}

	void setFrame(double frame) {
		this->frame = frame;
	}
};


struct Gate_MIDI : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(GATE_INPUTS, 16),
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	GateMidiOutput midiOutput;
	bool velocityMode = false;
	int learningId = -1;
	uint8_t learnedNotes[16] = {};

	Gate_MIDI() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for (int i = 0; i < 16; i++)
			configInput(GATE_INPUTS + i, string::f("Cell %d", i + 1));
		onReset();
	}

	void onReset() override {
		for (int y = 0; y < 4; y++) {
			for (int x = 0; x < 4; x++) {
				learnedNotes[4 * y + x] = 36 + 4 * (3 - y) + x;
			}
		}
		learningId = -1;
		midiOutput.reset();
		midiOutput.midi::Output::reset();
	}

	void process(const ProcessArgs& args) override {
		midiOutput.setFrame(args.frame);

		for (int i = 0; i < 16; i++) {
			int note = learnedNotes[i];
			if (velocityMode) {
				int vel = (int) std::round(inputs[GATE_INPUTS + i].getVoltage() / 10.f * 127);
				vel = clamp(vel, 0, 127);
				midiOutput.setVelocity(vel, note);
				midiOutput.setGate(vel > 0, note);
			}
			else {
				bool gate = inputs[GATE_INPUTS + i].getVoltage() >= 1.f;
				midiOutput.setVelocity(100, note);
				midiOutput.setGate(gate, note);
			}
		}
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		json_t* notesJ = json_array();
		for (int i = 0; i < 16; i++) {
			json_t* noteJ = json_integer(learnedNotes[i]);
			json_array_append_new(notesJ, noteJ);
		}
		json_object_set_new(rootJ, "notes", notesJ);

		json_object_set_new(rootJ, "velocity", json_boolean(velocityMode));

		json_object_set_new(rootJ, "midi", midiOutput.toJson());
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* notesJ = json_object_get(rootJ, "notes");
		if (notesJ) {
			for (int i = 0; i < 16; i++) {
				json_t* noteJ = json_array_get(notesJ, i);
				if (noteJ)
					learnedNotes[i] = json_integer_value(noteJ);
			}
		}

		json_t* velocityJ = json_object_get(rootJ, "velocity");
		if (velocityJ)
			velocityMode = json_boolean_value(velocityJ);

		json_t* midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiOutput.fromJson(midiJ);
	}
};


struct Gate_MIDIWidget : ModuleWidget {
	Gate_MIDIWidget(Gate_MIDI* module) {
		setModule(module);
		setPanel(Svg::load(asset::system("res/Core/Gate_MIDI.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.189, 78.431)), module, Gate_MIDI::GATE_INPUTS + 0));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.739, 78.431)), module, Gate_MIDI::GATE_INPUTS + 1));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31.289, 78.431)), module, Gate_MIDI::GATE_INPUTS + 2));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.838, 78.431)), module, Gate_MIDI::GATE_INPUTS + 3));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.189, 89.946)), module, Gate_MIDI::GATE_INPUTS + 4));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.739, 89.946)), module, Gate_MIDI::GATE_INPUTS + 5));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31.289, 89.946)), module, Gate_MIDI::GATE_INPUTS + 6));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.838, 89.946)), module, Gate_MIDI::GATE_INPUTS + 7));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.189, 101.466)), module, Gate_MIDI::GATE_INPUTS + 8));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.739, 101.466)), module, Gate_MIDI::GATE_INPUTS + 9));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31.289, 101.466)), module, Gate_MIDI::GATE_INPUTS + 10));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.838, 101.466)), module, Gate_MIDI::GATE_INPUTS + 11));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.189, 112.998)), module, Gate_MIDI::GATE_INPUTS + 12));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.739, 112.984)), module, Gate_MIDI::GATE_INPUTS + 13));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31.289, 112.984)), module, Gate_MIDI::GATE_INPUTS + 14));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.838, 112.984)), module, Gate_MIDI::GATE_INPUTS + 15));

		typedef Grid16MidiDisplay<NoteChoice<Gate_MIDI>> TMidiDisplay;
		TMidiDisplay* display = createWidget<TMidiDisplay>(mm2px(Vec(0.0, 13.039)));
		display->box.size = mm2px(Vec(50.8, 55.88));
		display->setMidiPort(module ? &module->midiOutput : NULL);
		display->setModule(module);
		addChild(display);
	}

	void appendContextMenu(Menu* menu) override {
		Gate_MIDI* module = dynamic_cast<Gate_MIDI*>(this->module);

		menu->addChild(new MenuSeparator);

		menu->addChild(createBoolPtrMenuItem("Velocity mode", "", &module->velocityMode));

		menu->addChild(createMenuItem("Panic", "",
			[=]() {module->midiOutput.panic();}
		));
	}
};


Model* modelGate_MIDI = createModel<Gate_MIDI, Gate_MIDIWidget>("CV-Gate");


} // namespace core
} // namespace rack
