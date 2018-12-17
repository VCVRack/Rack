#include "Core.hpp"
#include "midi.hpp"
#include "dsp/filter.hpp"


struct MIDICCToCVInterface : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(CC_OUTPUT, 16),
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	MidiInputQueue midiInput;
	int8_t ccs[128];
	ExponentialFilter ccFilters[16];

	int learningId = -1;
	int learnedCcs[16] = {};

	MIDICCToCVInterface() {
		setup(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		onReset();
	}

	void onReset() override {
		for (int i = 0; i < 128; i++) {
			ccs[i] = 0;
		}
		for (int i = 0; i < 16; i++) {
			learnedCcs[i] = i;
		}
		learningId = -1;
	}

	void step() override {
		MidiMessage msg;
		while (midiInput.shift(&msg)) {
			processMessage(msg);
		}

		float lambda = 100.f * gEngine->getSampleTime();
		for (int i = 0; i < 16; i++) {
			int learnedCc = learnedCcs[i];
			float value = rescale(clamp(ccs[learnedCc], -127, 127), 0, 127, 0.f, 10.f);
			ccFilters[i].lambda = lambda;
			outputs[CC_OUTPUT + i].value = ccFilters[i].process(value);
		}
	}

	void processMessage(MidiMessage msg) {
		switch (msg.status()) {
			// cc
			case 0xb: {
				uint8_t cc = msg.note();
				// Learn
				if (learningId >= 0 && ccs[cc] != msg.data2) {
					learnedCcs[learningId] = cc;
					learningId = -1;
				}
				// Set CV
				// Allow CC to be negative if the 8th bit is set
				ccs[cc] = msg.data2;
			} break;
			default: break;
		}
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();

		json_t *ccsJ = json_array();
		for (int i = 0; i < 16; i++) {
			json_t *ccJ = json_integer(learnedCcs[i]);
			json_array_append_new(ccsJ, ccJ);
		}
		json_object_set_new(rootJ, "ccs", ccsJ);

		json_object_set_new(rootJ, "midi", midiInput.toJson());
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t *ccsJ = json_object_get(rootJ, "ccs");
		if (ccsJ) {
			for (int i = 0; i < 16; i++) {
				json_t *ccJ = json_array_get(ccsJ, i);
				if (ccJ)
					learnedCcs[i] = json_integer_value(ccJ);
			}
		}

		json_t *midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiInput.fromJson(midiJ);
	}
};


struct MidiCcChoice : GridChoice {
	MIDICCToCVInterface *module;
	int id;
	int focusCc;

	MidiCcChoice() {
		box.size.y = mm2px(6.666);
		textOffset.y -= 4;
	}

	void setId(int id) override {
		this->id = id;
	}

	void step() override {
		if (module->learningId == id) {
			if (0 <= focusCc)
				text = string::f("%d", focusCc);
			else
				text = "LRN";
			color.a = 0.5;
		}
		else {
			text = string::f("%d", module->learnedCcs[id]);
			color.a = 1.0;
			if (event::gContext->selectedWidget == this)
				event::gContext->selectedWidget = NULL;
		}
	}

	void onSelect(event::Select &e) override {
		e.target = this;
		module->learningId = id;
		focusCc = -1;
	}

	void onDeselect(event::Deselect &e) override {
		if (0 <= focusCc && focusCc < 128) {
			module->learnedCcs[id] = focusCc;
		}
		module->learningId = -1;
	}

	void onSelectText(event::SelectText &e) override {
		char c = e.codepoint;
		if ('0' <= c && c <= '9') {
			if (focusCc < 0)
				focusCc = 0;
			focusCc = focusCc * 10 + (c - '0');
		}
		e.target = this;
	}

	void onSelectKey(event::SelectKey &e) override {
		if (event::gContext->selectedWidget == this) {
			if (e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER) {
				event::Deselect eDeselect;
				onDeselect(eDeselect);
				event::gContext->selectedWidget = NULL;
				e.target = this;
			}
		}
	}
};


struct MidiCcWidget : Grid16MidiWidget {
	MIDICCToCVInterface *module;
	GridChoice *createGridChoice() override {
		MidiCcChoice *gridChoice = new MidiCcChoice;
		gridChoice->module = module;
		return gridChoice;
	}
};


struct MIDICCToCVInterfaceWidget : ModuleWidget {
	MIDICCToCVInterfaceWidget(MIDICCToCVInterface *module) : ModuleWidget(module) {
		setPanel(SVG::load(asset::global("res/Core/MIDICCToCVInterface.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.894335, 73.344704)), module, MIDICCToCVInterface::CC_OUTPUT + 0));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.494659, 73.344704)), module, MIDICCToCVInterface::CC_OUTPUT + 1));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.094982, 73.344704)), module, MIDICCToCVInterface::CC_OUTPUT + 2));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(38.693932, 73.344704)), module, MIDICCToCVInterface::CC_OUTPUT + 3));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.8943355, 84.945023)), module, MIDICCToCVInterface::CC_OUTPUT + 4));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.49466, 84.945023)), module, MIDICCToCVInterface::CC_OUTPUT + 5));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.094982, 84.945023)), module, MIDICCToCVInterface::CC_OUTPUT + 6));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(38.693932, 84.945023)), module, MIDICCToCVInterface::CC_OUTPUT + 7));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.8943343, 96.543976)), module, MIDICCToCVInterface::CC_OUTPUT + 8));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.494659, 96.543976)), module, MIDICCToCVInterface::CC_OUTPUT + 9));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.09498, 96.543976)), module, MIDICCToCVInterface::CC_OUTPUT + 10));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(38.693932, 96.543976)), module, MIDICCToCVInterface::CC_OUTPUT + 11));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.894335, 108.14429)), module, MIDICCToCVInterface::CC_OUTPUT + 12));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.49466, 108.14429)), module, MIDICCToCVInterface::CC_OUTPUT + 13));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.09498, 108.14429)), module, MIDICCToCVInterface::CC_OUTPUT + 14));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(38.693932, 108.14429)), module, MIDICCToCVInterface::CC_OUTPUT + 15));

		MidiCcWidget *midiWidget = createWidget<MidiCcWidget>(mm2px(Vec(3.399621, 14.837339)));
		midiWidget->module = module;
		midiWidget->box.size = mm2px(Vec(44, 54.667));
		midiWidget->midiIO = &module->midiInput;
		midiWidget->createGridChoices();
		addChild(midiWidget);
	}
};


Model *modelMIDICCToCVInterface = createModel<MIDICCToCVInterface, MIDICCToCVInterfaceWidget>("Core", "MIDICCToCVInterface", "MIDI-CC", MIDI_TAG, EXTERNAL_TAG);
