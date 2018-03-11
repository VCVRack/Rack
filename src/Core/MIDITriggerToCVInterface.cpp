#include "Core.hpp"
#include "midi.hpp"
#include "dsp/filter.hpp"



struct CcChoice : LedDisplayChoice {
	CcChoice() {
		box.size.y = mm2px(6.666);
		textOffset.y -= 4;
	}
};


struct CcMidiWidget : MidiWidget {
	LedDisplaySeparator *hSeparators[4];
	LedDisplaySeparator *vSeparators[4];
	LedDisplayChoice *ccChoices[4][4];

	CcMidiWidget() {
		Vec pos = channelChoice->box.getBottomLeft();
		for (int x = 1; x < 4; x++) {
			vSeparators[x] = Widget::create<LedDisplaySeparator>(pos);
			addChild(vSeparators[x]);
		}
		for (int y = 0; y < 4; y++) {
			hSeparators[y] = Widget::create<LedDisplaySeparator>(pos);
			addChild(hSeparators[y]);
			for (int x = 0; x < 4; x++) {
				CcChoice *ccChoice = Widget::create<CcChoice>(pos);
				ccChoice->text = stringf("%d", x*4+y);
				ccChoices[x][y] = ccChoice;
				addChild(ccChoice);
			}
			pos = ccChoices[0][y]->box.getBottomLeft();
		}
		for (int x = 1; x < 4; x++) {
			vSeparators[x]->box.size.y = pos.y - vSeparators[x]->box.pos.y;
		}
	}
	void step() override {
		MidiWidget::step();
		for (int x = 1; x < 4; x++) {
			vSeparators[x]->box.pos.x = box.size.x / 4 * x;
		}
		for (int y = 0; y < 4; y++) {
			hSeparators[y]->box.size.x = box.size.x;
			for (int x = 0; x < 4; x++) {
				ccChoices[x][y]->box.size.x = box.size.x / 4;
				ccChoices[x][y]->box.pos.x = box.size.x / 4 * x;
			}
		}
	}
};


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

	MIDITriggerToCVInterface() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void onReset() override {
		for (int i = 0; i < 16; i++) {
			gates[i] = false;
			gateTimes[i] = 0.f;
		}
	}

	void pressNote(uint8_t note) {
		// TEMP
		if (note >= 16)
			return;
		int i = note;

		gates[i] = true;
		gateTimes[i] = 1e-3f;
	}

	void releaseNote(uint8_t note) {
		// TEMP
		if (note >= 16)
			return;
		int i = note;

		gates[i] = false;
	}

	void step() override {
		MidiMessage msg;
		while (midiInput.shift(&msg)) {
			processMessage(msg);
		}
		float deltaTime = engineGetSampleTime();

		for (int i = 0; i < 16; i++) {
			if (gateTimes[i] > 0.f) {
				outputs[TRIG_OUTPUT + i].value = 10.f;
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
					pressNote(msg.note());
				}
				else {
					releaseNote(msg.note());
				}
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

		MidiWidget *midiWidget = Widget::create<CcMidiWidget>(mm2px(Vec(3.399621, 14.837339)));
		midiWidget->box.size = mm2px(Vec(44, 54.667));
		midiWidget->midiIO = &module->midiInput;
		addChild(midiWidget);

	}
};


Model *modelMIDITriggerToCVInterface = Model::create<MIDITriggerToCVInterface, MIDITriggerToCVInterfaceWidget>("Core", "MIDITriggerToCVInterface", "MIDI-TRIG", MIDI_TAG, EXTERNAL_TAG);
