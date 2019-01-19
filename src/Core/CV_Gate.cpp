#include "Core.hpp"


template <int N>
struct GateMidiOutput : midi::Output {
	int vels[N];
	bool lastGates[N];
	int notes[N];

	GateMidiOutput() {
		reset();
	}

	void reset() {
		for (int n = 0; n < N; n++) {
			vels[n] = 100;
			lastGates[n] = false;
			notes[n] = 60 + n;
		}
	}

	void panic() {
		reset();
		// Send all note off commands
		for (int note = 0; note <= 127; note++) {
			// Note off
			midi::Message m;
			m.setStatus(0x8);
			m.setNote(note);
			m.setValue(0);
			sendMessage(m);
		}
	}

	void setVelocity(int vel, int n) {
		vels[n] = vel;
	}

	void setGate(bool gate, int n) {
		if (gate && !lastGates[n]) {
			// Note on
			midi::Message m;
			m.setStatus(0x9);
			m.setNote(notes[n]);
			m.setValue(vels[n]);
			sendMessage(m);
		}
		else if (!gate && lastGates[n]) {
			// Note off
			midi::Message m;
			m.setStatus(0x8);
			m.setNote(notes[n]);
			m.setValue(vels[n]);
			sendMessage(m);
		}
		lastGates[n] = gate;
	}

	void setNote(int note, int n) {
		if (note == notes[n])
			return;
		if (lastGates[n]) {
			// Note off
			midi::Message m1;
			m1.setStatus(0x8);
			m1.setNote(notes[n]);
			m1.setValue(vels[n]);
			sendMessage(m1);
			// Note on
			midi::Message m2;
			m2.setStatus(0x9);
			m2.setNote(note);
			m2.setValue(vels[n]);
			sendMessage(m2);
		}
		notes[n] = note;
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

	GateMidiOutput<16> midiOutput;
	bool velocityMode = false;

	CV_Gate() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void step() override {
		for (int n = 0; n < 16; n++) {
			if (velocityMode) {
				int vel = (int) std::round(inputs[GATE_INPUTS + n].getVoltage() / 10.f * 127);
				vel = clamp(vel, 0, 127);
				midiOutput.setVelocity(vel, n);
				midiOutput.setGate(vel > 0, n);
			}
			else {
				bool gate = inputs[GATE_INPUTS + n].getVoltage() >= 1.f;
				midiOutput.setVelocity(100, n);
				midiOutput.setGate(gate, n);
			}
		}
	}
};


struct CV_GateWidget : ModuleWidget {
	CV_GateWidget(CV_Gate *module) {
		setModule(module);
		setPanel(SVG::load(asset::system("res/Core/CV-Gate.svg")));

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

		MidiWidget *midiWidget = createWidget<MidiWidget>(mm2px(Vec(3.4, 14.839)));
		midiWidget->box.size = mm2px(Vec(44, 54.667));
		if (module)
			midiWidget->midiIO = &module->midiOutput;
		// midiWidget->createGridChoices();
		addChild(midiWidget);
	}
};


Model *modelCV_Gate = createModel<CV_Gate, CV_GateWidget>("CV-Gate");

