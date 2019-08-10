#include "plugin.hpp"


namespace rack {
namespace core {


struct MidiOutput : dsp::MidiGenerator<PORT_MAX_CHANNELS>, midi::Output {
	void onMessage(midi::Message message) override {
		midi::Output::sendMessage(message);
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
	float rateLimiterPhase = 0.f;

	CV_MIDI() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		onReset();
	}

	void onReset() override {
		midiOutput.reset();
	}

	void process(const ProcessArgs& args) override {
		const float rateLimiterPeriod = 0.005f;
		rateLimiterPhase += args.sampleTime / rateLimiterPeriod;
		if (rateLimiterPhase >= 1.f) {
			rateLimiterPhase -= 1.f;
		}
		else {
			return;
		}

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
	void onAction(const event::Action& e) override {
		module->midiOutput.panic();
	}
};


struct CV_MIDIWidget : ModuleWidget {
	CV_MIDIWidget(CV_MIDI* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::system("res/Core/CV-MIDI.svg")));

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

		MidiWidget* midiWidget = createWidget<MidiWidget>(mm2px(Vec(3.41891, 14.8373)));
		midiWidget->box.size = mm2px(Vec(33.840, 28));
		midiWidget->setMidiPort(module ? &module->midiOutput : NULL);
		addChild(midiWidget);
	}

	void appendContextMenu(Menu* menu) override {
		CV_MIDI* module = dynamic_cast<CV_MIDI*>(this->module);

		menu->addChild(new MenuEntry);

		CV_MIDIPanicItem* panicItem = new CV_MIDIPanicItem;
		panicItem->text = "Panic";
		panicItem->module = module;
		menu->addChild(panicItem);
	}
};


Model* modelCV_MIDI = createModel<CV_MIDI, CV_MIDIWidget>("CV-MIDI");


} // namespace core
} // namespace rack
