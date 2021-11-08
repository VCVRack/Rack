#include "plugin.hpp"


namespace rack {
namespace core {


struct MIDI_Gate : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(GATE_OUTPUTS, 16),
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	midi::InputQueue midiInput;

	/** [cell][channel] */
	bool gates[16][16];
	/** [cell][channel] */
	float gateTimes[16][16];
	/** [cell][channel] */
	uint8_t velocities[16][16];
	/** Cell ID in learn mode, or -1 if none. */
	int learningId;
	/** [cell] */
	uint8_t learnedNotes[16];
	bool velocityMode;
	bool mpeMode;

	MIDI_Gate() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for (int i = 0; i < 16; i++)
			configOutput(GATE_OUTPUTS + i, string::f("Gate %d", i + 1));

		onReset();
	}

	void onReset() override {
		for (int y = 0; y < 4; y++) {
			for (int x = 0; x < 4; x++) {
				learnedNotes[4 * y + x] = 36 + 4 * (3 - y) + x;
			}
		}
		learningId = -1;
		panic();
		midiInput.reset();
		velocityMode = false;
		mpeMode = false;
	}

	void panic() {
		for (int i = 0; i < 16; i++) {
			for (int c = 0; c < 16; c++) {
				gates[i][c] = false;
				gateTimes[i][c] = 0.f;
			}
		}
	}

	void process(const ProcessArgs& args) override {
		midi::Message msg;
		while (midiInput.tryPop(&msg, args.frame)) {
			processMessage(msg);
		}

		int channels = mpeMode ? 16 : 1;

		for (int i = 0; i < 16; i++) {
			outputs[GATE_OUTPUTS + i].setChannels(channels);
			for (int c = 0; c < channels; c++) {
				// Make sure all pulses last longer than 1ms
				if (gates[i][c] || gateTimes[i][c] > 0.f) {
					float velocity = velocityMode ? (velocities[i][c] / 127.f) : 1.f;
					outputs[GATE_OUTPUTS + i].setVoltage(velocity * 10.f, c);
					gateTimes[i][c] -= args.sampleTime;
				}
				else {
					outputs[GATE_OUTPUTS + i].setVoltage(0.f, c);
				}
			}
		}
	}

	void processMessage(const midi::Message& msg) {
		switch (msg.getStatus()) {
			// note off
			case 0x8: {
				releaseNote(msg.getChannel(), msg.getNote());
			} break;
			// note on
			case 0x9: {
				if (msg.getValue() > 0) {
					pressNote(msg.getChannel(), msg.getNote(), msg.getValue());
				}
				else {
					// Many stupid keyboards send a "note on" command with 0 velocity to mean "note release"
					releaseNote(msg.getChannel(), msg.getNote());
				}
			} break;
			default: break;
		}
	}

	void pressNote(uint8_t channel, uint8_t note, uint8_t vel) {
		int c = mpeMode ? channel : 0;
		// Learn
		if (learningId >= 0) {
			learnedNotes[learningId] = note;
			learningId = -1;
		}
		// Find id
		for (int i = 0; i < 16; i++) {
			if (learnedNotes[i] == note) {
				gates[i][c] = true;
				gateTimes[i][c] = 1e-3f;
				velocities[i][c] = vel;
			}
		}
	}

	void releaseNote(uint8_t channel, uint8_t note) {
		int c = mpeMode ? channel : 0;
		// Find id
		for (int i = 0; i < 16; i++) {
			if (learnedNotes[i] == note) {
				gates[i][c] = false;
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

		json_object_set_new(rootJ, "midi", midiInput.toJson());

		json_object_set_new(rootJ, "mpeMode", json_boolean(mpeMode));
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
			midiInput.fromJson(midiJ);

		json_t* mpeModeJ = json_object_get(rootJ, "mpeMode");
		if (mpeModeJ)
			mpeMode = json_boolean_value(mpeModeJ);
	}
};


struct MIDI_GateWidget : ModuleWidget {
	MIDI_GateWidget(MIDI_Gate* module) {
		setModule(module);
		setPanel(Svg::load(asset::system("res/Core/MIDI_Gate.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.189, 78.431)), module, MIDI_Gate::GATE_OUTPUTS + 0));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19.739, 78.431)), module, MIDI_Gate::GATE_OUTPUTS + 1));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.289, 78.431)), module, MIDI_Gate::GATE_OUTPUTS + 2));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.838, 78.431)), module, MIDI_Gate::GATE_OUTPUTS + 3));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.189, 89.946)), module, MIDI_Gate::GATE_OUTPUTS + 4));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19.739, 89.946)), module, MIDI_Gate::GATE_OUTPUTS + 5));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.289, 89.946)), module, MIDI_Gate::GATE_OUTPUTS + 6));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.838, 89.946)), module, MIDI_Gate::GATE_OUTPUTS + 7));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.189, 101.466)), module, MIDI_Gate::GATE_OUTPUTS + 8));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19.739, 101.466)), module, MIDI_Gate::GATE_OUTPUTS + 9));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.289, 101.466)), module, MIDI_Gate::GATE_OUTPUTS + 10));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.838, 101.466)), module, MIDI_Gate::GATE_OUTPUTS + 11));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.189, 112.998)), module, MIDI_Gate::GATE_OUTPUTS + 12));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19.739, 112.984)), module, MIDI_Gate::GATE_OUTPUTS + 13));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.289, 112.984)), module, MIDI_Gate::GATE_OUTPUTS + 14));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.838, 112.984)), module, MIDI_Gate::GATE_OUTPUTS + 15));

		typedef Grid16MidiDisplay<NoteChoice<MIDI_Gate>> TMidiDisplay;
		TMidiDisplay* display = createWidget<TMidiDisplay>(mm2px(Vec(0.0, 13.039)));
		display->box.size = mm2px(Vec(50.8, 55.88));
		display->setMidiPort(module ? &module->midiInput : NULL);
		display->setModule(module);
		addChild(display);
	}

	void appendContextMenu(Menu* menu) override {
		MIDI_Gate* module = dynamic_cast<MIDI_Gate*>(this->module);

		menu->addChild(new MenuSeparator);

		menu->addChild(createBoolPtrMenuItem("Velocity mode", "", &module->velocityMode));

		menu->addChild(createBoolPtrMenuItem("MPE mode", "", &module->mpeMode));

		menu->addChild(createMenuItem("Panic", "",
			[=]() {module->panic();}
		));
	}
};


// Use legacy slug for compatibility
Model* modelMIDI_Gate = createModel<MIDI_Gate, MIDI_GateWidget>("MIDITriggerToCVInterface");


} // namespace core
} // namespace rack
