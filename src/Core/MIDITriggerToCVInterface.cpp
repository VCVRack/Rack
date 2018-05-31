#include "Core.hpp"
#include "midi.hpp"


struct MIDITriggerToCVInterface : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(TRIG_OUTPUT, 16),
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	MidiInputQueue midiInput;

	bool gates[16];
	float gateTimes[16];
	uint8_t velocities[16];
	int learningId = -1;
	uint8_t learnedNotes[16] = {};
	bool velocity = false;

	MIDITriggerToCVInterface() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void onReset() override {
		for (int i = 0; i < 16; i++) {
			gates[i] = false;
			gateTimes[i] = 0.f;
			learnedNotes[i] = i + 36;
		}
		learningId = -1;
	}

	void pressNote(uint8_t note, uint8_t vel) {
		// Learn
		if (learningId >= 0) {
			learnedNotes[learningId] = note;
			learningId = -1;
		}
		// Find id
		for (int i = 0; i < 16; i++) {
			if (learnedNotes[i] == note) {
				gates[i] = true;
				gateTimes[i] = 1e-3f;
				velocities[i] = vel;
			}
		}
	}

	void releaseNote(uint8_t note) {
		// Find id
		for (int i = 0; i < 16; i++) {
			if (learnedNotes[i] == note) {
				gates[i] = false;
			}
		}
	}

	void step() override {
		MidiMessage msg;
		while (midiInput.shift(&msg)) {
			processMessage(msg);
		}
		float deltaTime = engineGetSampleTime();

		for (int i = 0; i < 16; i++) {
			if (gateTimes[i] > 0.f) {
				outputs[TRIG_OUTPUT + i].value = velocity ? rescale(velocities[i], 0, 127, 0.f, 10.f) : 10.f;
				// If the gate is off, wait 1 ms before turning the pulse off.
				// This avoids drum controllers sending a pulse with 0 ms duration.
				if (!gates[i]) {
					gateTimes[i] -= deltaTime;
				}
			}
			else {
				outputs[TRIG_OUTPUT + i].value = 0.f;
			}
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
					pressNote(msg.note(), msg.value());
				}
				else {
					// Many stupid keyboards send a "note on" command with 0 velocity to mean "note release"
					releaseNote(msg.note());
				}
			} break;
			default: break;
		}
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();

		json_t *notesJ = json_array();
		for (int i = 0; i < 16; i++) {
			json_t *noteJ = json_integer(learnedNotes[i]);
			json_array_append_new(notesJ, noteJ);
		}
		json_object_set_new(rootJ, "notes", notesJ);

		json_object_set_new(rootJ, "midi", midiInput.toJson());
		json_object_set_new(rootJ, "velocity", json_boolean(velocity));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *notesJ = json_object_get(rootJ, "notes");
		if (notesJ) {
			for (int i = 0; i < 16; i++) {
				json_t *noteJ = json_array_get(notesJ, i);
				if (noteJ)
					learnedNotes[i] = json_integer_value(noteJ);
			}
		}

		json_t *midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiInput.fromJson(midiJ);

		json_t *velocityJ = json_object_get(rootJ, "velocity");
		if (velocityJ)
			velocity = json_boolean_value(velocityJ);
	}
};


struct MidiTrigChoice : GridChoice {
	MIDITriggerToCVInterface *module;
	int id;

	MidiTrigChoice() {
		box.size.y = mm2px(6.666);
		textOffset.y -= 4;
		textOffset.x -= 4;
	}

	void setId(int id) override {
		this->id = id;
	}

	void step() override {
		if (module->learningId == id) {
			text = "LRN";
			color.a = 0.5;
		}
		else {
			uint8_t note = module->learnedNotes[id];
			static const char *noteNames[] = {
				"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
			};
			int oct = note / 12 - 1;
			int semi = note % 12;
			text = stringf("%s%d", noteNames[semi], oct);
			color.a = 1.0;

			if (gFocusedWidget == this)
				gFocusedWidget = NULL;
		}
	}

	void onFocus(EventFocus &e) override {
		e.consumed = true;
		module->learningId = id;
	}

	void onDefocus(EventDefocus &e) override {
		module->learningId = -1;
	}
};


struct MidiTrigWidget : Grid16MidiWidget {
	MIDITriggerToCVInterface *module;
	GridChoice *createGridChoice() override {
		MidiTrigChoice *gridChoice = new MidiTrigChoice();
		gridChoice->module = module;
		return gridChoice;
	}
};


struct MIDITriggerToCVInterfaceWidget : ModuleWidget {
	MIDITriggerToCVInterfaceWidget(MIDITriggerToCVInterface *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetGlobal("res/Core/MIDITriggerToCVInterface.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.894335, 73.344704)), Port::OUTPUT, module, MIDITriggerToCVInterface::TRIG_OUTPUT + 0));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.494659, 73.344704)), Port::OUTPUT, module, MIDITriggerToCVInterface::TRIG_OUTPUT + 1));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.094982, 73.344704)), Port::OUTPUT, module, MIDITriggerToCVInterface::TRIG_OUTPUT + 2));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.693932, 73.344704)), Port::OUTPUT, module, MIDITriggerToCVInterface::TRIG_OUTPUT + 3));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.8943355, 84.945023)), Port::OUTPUT, module, MIDITriggerToCVInterface::TRIG_OUTPUT + 4));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.49466, 84.945023)), Port::OUTPUT, module, MIDITriggerToCVInterface::TRIG_OUTPUT + 5));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.094982, 84.945023)), Port::OUTPUT, module, MIDITriggerToCVInterface::TRIG_OUTPUT + 6));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.693932, 84.945023)), Port::OUTPUT, module, MIDITriggerToCVInterface::TRIG_OUTPUT + 7));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.8943343, 96.543976)), Port::OUTPUT, module, MIDITriggerToCVInterface::TRIG_OUTPUT + 8));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.494659, 96.543976)), Port::OUTPUT, module, MIDITriggerToCVInterface::TRIG_OUTPUT + 9));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.09498, 96.543976)), Port::OUTPUT, module, MIDITriggerToCVInterface::TRIG_OUTPUT + 10));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.693932, 96.543976)), Port::OUTPUT, module, MIDITriggerToCVInterface::TRIG_OUTPUT + 11));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.894335, 108.14429)), Port::OUTPUT, module, MIDITriggerToCVInterface::TRIG_OUTPUT + 12));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.49466, 108.14429)), Port::OUTPUT, module, MIDITriggerToCVInterface::TRIG_OUTPUT + 13));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.09498, 108.14429)), Port::OUTPUT, module, MIDITriggerToCVInterface::TRIG_OUTPUT + 14));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.693932, 108.14429)), Port::OUTPUT, module, MIDITriggerToCVInterface::TRIG_OUTPUT + 15));

		MidiTrigWidget *midiWidget = Widget::create<MidiTrigWidget>(mm2px(Vec(3.399621, 14.837339)));
		midiWidget->module = module;
		midiWidget->box.size = mm2px(Vec(44, 54.667));
		midiWidget->midiIO = &module->midiInput;
		midiWidget->createGridChoices();
		addChild(midiWidget);
	}

	void appendContextMenu(Menu *menu) override {
		MIDITriggerToCVInterface *module = dynamic_cast<MIDITriggerToCVInterface*>(this->module);

		struct VelocityItem : MenuItem {
			MIDITriggerToCVInterface *module;
			void onAction(EventAction &e) override {
				module->velocity ^= true;
			}
		};

		menu->addChild(MenuEntry::create());
		VelocityItem *velocityItem = MenuItem::create<VelocityItem>("Velocity", CHECKMARK(module->velocity));
		velocityItem->module = module;
		menu->addChild(velocityItem);
	}
};


Model *modelMIDITriggerToCVInterface = Model::create<MIDITriggerToCVInterface, MIDITriggerToCVInterfaceWidget>("Core", "MIDITriggerToCVInterface", "MIDI-Trig", MIDI_TAG, EXTERNAL_TAG);
