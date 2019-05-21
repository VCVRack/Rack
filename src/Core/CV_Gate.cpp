#include "plugin.hpp"


struct GateMidiOutput : midi::Output {
	int vels[128];
	bool lastGates[128];

	GateMidiOutput() {
		reset();
	}

	void reset() {
		for (int note = 0; note < 128; note++) {
			vels[note] = 100;
			lastGates[note] = false;
		}
	}

	void panic() {
		reset();
		// Send all note off commands
		for (int note = 0; note < 128; note++) {
			// Note off
			midi::Message m;
			m.setStatus(0x8);
			m.setNote(note);
			m.setValue(0);
			sendMessage(m);
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
			sendMessage(m);
		}
		else if (!gate && lastGates[note]) {
			// Note off
			midi::Message m;
			m.setStatus(0x8);
			m.setNote(note);
			m.setValue(vels[note]);
			sendMessage(m);
		}
		lastGates[note] = gate;
	}
};


struct CV_Gate : Module {
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

	CV_Gate() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		onReset();
	}

	void onReset() override {
		for (int y = 0; y < 4; y++) {
			for (int x = 0; x < 4; x++) {
				learnedNotes[4 * y + x] = 36 + 4 * (3-y) + x;
			}
		}
		learningId = -1;
		midiOutput.reset();
		midiOutput.midi::Output::reset();
	}

	void process(const ProcessArgs &args) override {
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

	json_t *dataToJson() override {
		json_t *rootJ = json_object();

		json_t *notesJ = json_array();
		for (int i = 0; i < 16; i++) {
			json_t *noteJ = json_integer(learnedNotes[i]);
			json_array_append_new(notesJ, noteJ);
		}
		json_object_set_new(rootJ, "notes", notesJ);

		json_object_set_new(rootJ, "velocity", json_boolean(velocityMode));

		json_object_set_new(rootJ, "midi", midiOutput.toJson());
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t *notesJ = json_object_get(rootJ, "notes");
		if (notesJ) {
			for (int i = 0; i < 16; i++) {
				json_t *noteJ = json_array_get(notesJ, i);
				if (noteJ)
					learnedNotes[i] = json_integer_value(noteJ);
			}
		}

		json_t *velocityJ = json_object_get(rootJ, "velocity");
		if (velocityJ)
			velocityMode = json_boolean_value(velocityJ);

		json_t *midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiOutput.fromJson(midiJ);
	}
};


struct CV_GateVelocityItem : MenuItem {
	CV_Gate *module;
	void onAction(const event::Action &e) override {
		module->velocityMode ^= true;
	}
};


struct CV_GatePanicItem : MenuItem {
	CV_Gate *module;
	void onAction(const event::Action &e) override {
		module->midiOutput.panic();
	}
};


struct CV_GateWidget : ModuleWidget {
	CV_GateWidget(CV_Gate *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::system("res/Core/CV-Gate.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 77)), module, CV_Gate::GATE_INPUTS + 0));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 77)), module, CV_Gate::GATE_INPUTS + 1));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31, 77)), module, CV_Gate::GATE_INPUTS + 2));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43, 77)), module, CV_Gate::GATE_INPUTS + 3));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 89)), module, CV_Gate::GATE_INPUTS + 4));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 89)), module, CV_Gate::GATE_INPUTS + 5));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31, 89)), module, CV_Gate::GATE_INPUTS + 6));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43, 89)), module, CV_Gate::GATE_INPUTS + 7));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 101)), module, CV_Gate::GATE_INPUTS + 8));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 101)), module, CV_Gate::GATE_INPUTS + 9));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31, 101)), module, CV_Gate::GATE_INPUTS + 10));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43, 101)), module, CV_Gate::GATE_INPUTS + 11));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 112)), module, CV_Gate::GATE_INPUTS + 12));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 112)), module, CV_Gate::GATE_INPUTS + 13));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31, 112)), module, CV_Gate::GATE_INPUTS + 14));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43, 112)), module, CV_Gate::GATE_INPUTS + 15));

		typedef Grid16MidiWidget<NoteChoice<CV_Gate>> TMidiWidget;
		TMidiWidget *midiWidget = createWidget<TMidiWidget>(mm2px(Vec(3.399621, 14.837339)));
		midiWidget->box.size = mm2px(Vec(44, 54.667));
		midiWidget->setMidiPort(module ? &module->midiOutput : NULL);
		midiWidget->setModule(module);
		addChild(midiWidget);
	}

	void appendContextMenu(Menu *menu) override {
		CV_Gate *module = dynamic_cast<CV_Gate*>(this->module);

		menu->addChild(new MenuEntry);
		CV_GateVelocityItem *velocityItem = createMenuItem<CV_GateVelocityItem>("Velocity mode", CHECKMARK(module->velocityMode));
		velocityItem->module = module;
		menu->addChild(velocityItem);

		CV_GatePanicItem *panicItem = new CV_GatePanicItem;
		panicItem->text = "Panic";
		panicItem->module = module;
		menu->addChild(panicItem);
	}
};


Model *modelCV_Gate = createModel<CV_Gate, CV_GateWidget>("CV-Gate");
