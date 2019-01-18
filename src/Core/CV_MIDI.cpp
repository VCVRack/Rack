#include "Core.hpp"


template <int C>
struct PolyphonicMidiOutput : midi::Output {
	int vels[C];
	int lastNotes[C];
	int notes[C];
	bool lastGates[C];
	bool gates[C];
	int lastAfts[C];
	int lastPw;
	int lastMw;
	bool lastClk;
	int lastVol;
	int lastPan;
	bool lastStart;
	bool lastStop;
	bool lastCont;

	PolyphonicMidiOutput() {
		reset();
	}

	void reset() {
		for (int c = 0; c < C; c++) {
			vels[c] = 100;
			lastNotes[c] = notes[c] = 60;
			lastGates[c] = gates[c] = false;
			lastAfts[c] = -1;
		}
		lastPw = 0x2000;
		lastMw = 0;
		lastClk = false;
		lastVol = 127;
		lastPan = 64;
		lastStart = false;
		lastStop = false;
		lastCont = false;
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

	void setVel(int vel, int c) {
		vels[c] = vel;
	}

	void setNote(int note, int c) {
		notes[c] = note;
	}

	void setGate(bool gate, int c) {
		gates[c] = gate;
	}

	void stepChannel(int c) {
		bool changedNote = gates[c] && lastGates[c] && notes[c] != lastNotes[c];
		bool enabledGate = gates[c] && !lastGates[c];
		bool disabledGate = !gates[c] && lastGates[c];
		if (changedNote || enabledGate) {
			// Note on
			midi::Message m;
			m.setStatus(0x9);
			m.setNote(notes[c]);
			m.setValue(vels[c]);
			sendMessage(m);
		}
		if (changedNote || disabledGate) {
			// Note off
			midi::Message m;
			m.setStatus(0x8);
			m.setNote(lastNotes[c]);
			m.setValue(vels[c]);
			sendMessage(m);
		}
		lastNotes[c] = notes[c];
		lastGates[c] = gates[c];
	}

	void setAftertouch(int aft, int c) {
		if (lastAfts[c] == aft)
			return;
		lastAfts[c] = aft;
		// Polyphonic key pressure
		midi::Message m;
		m.setStatus(0xa);
		m.setNote(notes[c]);
		m.setValue(aft);
		sendMessage(m);
	}

	void setPitchWheel(int pw) {
		if (lastPw == pw)
			return;
		lastPw = pw;
		// Pitch wheel
		midi::Message m;
		m.setStatus(0xe);
		m.setNote(pw & 0x7f);
		m.setValue((pw >> 7) & 0x7f);
		sendMessage(m);
	}

	void setModWheel(int mw) {
		if (lastMw == mw)
			return;
		lastMw = mw;
		// CC Mod wheel
		midi::Message m;
		m.setStatus(0xb);
		m.setNote(0x01);
		m.setValue(mw);
		sendMessage(m);
	}

	void setClock(bool clk) {
		if (lastClk == clk)
			return;
		lastClk = clk;
		if (clk) {
			// Timing clock
			midi::Message m;
			m.setStatus(0xf);
			m.setChannel(0x8);
			sendMessage(m);
		}
	}

	void setVolume(int vol) {
		if (lastVol == vol)
			return;
		lastVol = vol;
		// CC Volume
		midi::Message m;
		m.setStatus(0xb);
		m.setNote(0x07);
		m.setValue(vol);
		sendMessage(m);
	}

	void setPan(int pan) {
		if (lastPan == pan)
			return;
		lastPan = pan;
		// CC Pan
		midi::Message m;
		m.setStatus(0xb);
		m.setNote(0x0a);
		m.setValue(pan);
		sendMessage(m);
	}

	void setStart(bool start) {
		if (lastStart == start)
			return;
		lastStart = start;
		if (start) {
			// Start
			midi::Message m;
			m.setStatus(0xf);
			m.setChannel(0xa);
			sendMessage(m);
		}
	}

	void setStop(bool stop) {
		if (lastStop == stop)
			return;
		lastStop = stop;
		if (stop) {
			// Stop
			midi::Message m;
			m.setStatus(0xf);
			m.setChannel(0xb);
			sendMessage(m);
		}
	}

	void setContinue(bool cont) {
		if (lastCont == cont)
			return;
		lastCont = cont;
		if (cont) {
			// Continue
			midi::Message m;
			m.setStatus(0xf);
			m.setChannel(0xc);
			sendMessage(m);
		}
	}
};


struct CV_MIDI : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		PITCH_INPUT,
		GATE_INPUT,
		VEL_INPUT,
		AFT_INPUT,
		PW_INPUT,
		MW_INPUT,
		CLK_INPUT,
		VOL_INPUT,
		PAN_INPUT,
		START_INPUT,
		STOP_INPUT,
		CONTINUE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	PolyphonicMidiOutput<PORT_MAX_CHANNELS> midiOutput;

	CV_MIDI() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void step() override {
		for (int c = 0; c < inputs[PITCH_INPUT].channels; c++) {
			int vel = (int) std::round(inputs[VEL_INPUT].normalize(10.f * 100 / 127, c) / 10.f * 127);
			vel = clamp(vel, 0, 127);
			midiOutput.setVel(vel, c);

			int note = (int) std::round(inputs[PITCH_INPUT].getVoltage(c) * 12.f + 60.f);
			note = clamp(note, 0, 127);
			midiOutput.setNote(note, c);

			bool gate = inputs[GATE_INPUT].getVoltage(c) >= 1.f;
			midiOutput.setGate(gate, c);

			midiOutput.stepChannel(c);

			int aft = (int) std::round(inputs[AFT_INPUT].getVoltage(c) / 10.f * 127);
			aft = clamp(aft, 0, 127);
			midiOutput.setAftertouch(aft, c);
		}

		int pw = (int) std::round((inputs[PW_INPUT].getVoltage() + 5.f) / 10.f * 0x4000);
		pw = clamp(pw, 0, 0x3fff);
		midiOutput.setPitchWheel(pw);

		int mw = (int) std::round(inputs[MW_INPUT].getVoltage() / 10.f * 127);
		mw = clamp(mw, 0, 127);
		midiOutput.setModWheel(mw);

		bool clk = inputs[CLK_INPUT].value >= 1.f;
		midiOutput.setClock(clk);

		int vol = (int) std::round(inputs[VOL_INPUT].normalize(10.f) / 10.f * 127);
		vol = clamp(vol, 0, 127);
		midiOutput.setVolume(vol);

		int pan = (int) std::round((inputs[PAN_INPUT].getVoltage() + 5.f) / 10.f * 127);
		pan = clamp(pan, 0, 127);
		midiOutput.setPan(pan);

		bool start = inputs[START_INPUT].value >= 1.f;
		midiOutput.setStart(start);

		bool stop = inputs[STOP_INPUT].value >= 1.f;
		midiOutput.setStop(stop);

		bool cont = inputs[CONTINUE_INPUT].value >= 1.f;
		midiOutput.setContinue(cont);
	}
};


struct CV_MIDIWidget : ModuleWidget {
	CV_MIDIWidget(CV_MIDI *module) : ModuleWidget(module) {
		setPanel(SVG::load(asset::system("res/Core/CV-MIDI.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9, 64)), module, CV_MIDI::PITCH_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 64)), module, CV_MIDI::GATE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32, 64)), module, CV_MIDI::VEL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9, 80)), module, CV_MIDI::AFT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 80)), module, CV_MIDI::PW_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32, 80)), module, CV_MIDI::MW_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9, 96)), module, CV_MIDI::CLK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 96)), module, CV_MIDI::VOL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32, 96)), module, CV_MIDI::PAN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9, 112)), module, CV_MIDI::START_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 112)), module, CV_MIDI::STOP_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32, 112)), module, CV_MIDI::CONTINUE_INPUT));

		MidiWidget *midiWidget = createWidget<MidiWidget>(mm2px(Vec(3.41891, 14.8373)));
		midiWidget->box.size = mm2px(Vec(33.840, 28));
		if (module)
			midiWidget->midiIO = &module->midiOutput;
		addChild(midiWidget);
	}
};


Model *modelCV_MIDI = createModel<CV_MIDI, CV_MIDIWidget>("CV-MIDI");
