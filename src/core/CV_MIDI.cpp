#include "plugin.hpp"


namespace rack {
namespace core {


struct MidiOutput : dsp::MidiGenerator<PORT_MAX_CHANNELS>, midi::Output {
	void onMessage(const midi::Message& message) override {
		Output::sendMessage(message);
	}

	void reset() {
		Output::reset();
		MidiGenerator::reset();
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

	MidiOutput midiOutput;
	dsp::Timer rateLimiterTimer;

	CV_MIDI() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configInput(PITCH_INPUT, "1V/octave pitch");
		configInput(GATE_INPUT, "Gate");
		configInput(VEL_INPUT, "Velocity");
		configInput(AFT_INPUT, "Aftertouch");
		configInput(PW_INPUT, "Pitch wheel");
		configInput(MW_INPUT, "Mod wheel");
		configInput(CLK_INPUT, "Clock");
		configInput(VOL_INPUT, "Volume");
		configInput(PAN_INPUT, "Pan");
		configInput(START_INPUT, "Start trigger");
		configInput(STOP_INPUT, "Stop trigger");
		configInput(CONTINUE_INPUT, "Continue trigger");
		onReset();
	}

	void onReset() override {
		midiOutput.reset();
	}

	void process(const ProcessArgs& args) override {
		// MIDI baud rate is 31250 b/s, or 3125 B/s.
		// CC messages are 3 bytes, so we can send a maximum of 1041 CC messages per second.
		// Since multiple CCs can be generated, play it safe and limit the CC rate to 200 Hz.
		const float rateLimiterPeriod = 1 / 200.f;
		bool rateLimiterTriggered = (rateLimiterTimer.process(args.sampleTime) >= rateLimiterPeriod);
		if (rateLimiterTriggered)
			rateLimiterTimer.time -= rateLimiterPeriod;

		midiOutput.setFrame(args.frame);

		for (int c = 0; c < inputs[PITCH_INPUT].getChannels(); c++) {
			int vel = (int) std::round(inputs[VEL_INPUT].getNormalPolyVoltage(10.f * 100 / 127, c) / 10.f * 127);
			vel = clamp(vel, 0, 127);
			midiOutput.setVelocity(vel, c);

			int note = (int) std::round(inputs[PITCH_INPUT].getVoltage(c) * 12.f + 60.f);
			note = clamp(note, 0, 127);
			bool gate = inputs[GATE_INPUT].getPolyVoltage(c) >= 1.f;
			midiOutput.setNoteGate(note, gate, c);

			int aft = (int) std::round(inputs[AFT_INPUT].getPolyVoltage(c) / 10.f * 127);
			aft = clamp(aft, 0, 127);
			midiOutput.setKeyPressure(aft, c);
		}

		if (rateLimiterTriggered) {
			int pw = (int) std::round((inputs[PW_INPUT].getVoltage() + 5.f) / 10.f * 0x4000);
			pw = clamp(pw, 0, 0x3fff);
			midiOutput.setPitchWheel(pw);

			int mw = (int) std::round(inputs[MW_INPUT].getVoltage() / 10.f * 127);
			mw = clamp(mw, 0, 127);
			midiOutput.setModWheel(mw);

			int vol = (int) std::round(inputs[VOL_INPUT].getNormalVoltage(10.f) / 10.f * 127);
			vol = clamp(vol, 0, 127);
			midiOutput.setVolume(vol);

			int pan = (int) std::round((inputs[PAN_INPUT].getVoltage() + 5.f) / 10.f * 127);
			pan = clamp(pan, 0, 127);
			midiOutput.setPan(pan);
		}

		bool clk = inputs[CLK_INPUT].getVoltage() >= 1.f;
		midiOutput.setClock(clk);

		bool start = inputs[START_INPUT].getVoltage() >= 1.f;
		midiOutput.setStart(start);

		bool stop = inputs[STOP_INPUT].getVoltage() >= 1.f;
		midiOutput.setStop(stop);

		bool cont = inputs[CONTINUE_INPUT].getVoltage() >= 1.f;
		midiOutput.setContinue(cont);
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "midi", midiOutput.toJson());
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiOutput.fromJson(midiJ);
	}
};


struct CV_MIDIPanicItem : MenuItem {
	CV_MIDI* module;
	void onAction(const ActionEvent& e) override {
		module->midiOutput.panic();
	}
};


struct CV_MIDIWidget : ModuleWidget {
	CV_MIDIWidget(CV_MIDI* module) {
		setModule(module);
		setPanel(Svg::load(asset::system("res/Core/CV_MIDI.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.906, 64.347)), module, CV_MIDI::PITCH_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.249, 64.347)), module, CV_MIDI::GATE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.591, 64.347)), module, CV_MIDI::VEL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.906, 80.603)), module, CV_MIDI::AFT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.249, 80.603)), module, CV_MIDI::PW_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.591, 80.603)), module, CV_MIDI::MW_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.906, 96.859)), module, CV_MIDI::CLK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.249, 96.707)), module, CV_MIDI::VOL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.591, 96.859)), module, CV_MIDI::PAN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.906, 113.115)), module, CV_MIDI::START_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.249, 113.115)), module, CV_MIDI::STOP_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.591, 112.975)), module, CV_MIDI::CONTINUE_INPUT));

		MidiDisplay* display = createWidget<MidiDisplay>(mm2px(Vec(0.0, 13.039)));
		display->box.size = mm2px(Vec(40.64, 29.021));
		display->setMidiPort(module ? &module->midiOutput : NULL);
		addChild(display);
	}

	void appendContextMenu(Menu* menu) override {
		CV_MIDI* module = dynamic_cast<CV_MIDI*>(this->module);

		menu->addChild(new MenuSeparator);

		menu->addChild(createMenuItem("Panic", "",
			[=]() {module->midiOutput.panic();}
		));
	}
};


Model* modelCV_MIDI = createModel<CV_MIDI, CV_MIDIWidget>("CV-MIDI");


} // namespace core
} // namespace rack
