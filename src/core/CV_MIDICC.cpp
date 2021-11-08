#include "plugin.hpp"


namespace rack {
namespace core {


struct CCMidiOutput : midi::Output {
	int lastValues[128];
	double frame = 0.0;

	CCMidiOutput() {
		reset();
	}

	void reset() {
		for (int n = 0; n < 128; n++) {
			lastValues[n] = -1;
		}
		Output::reset();
	}

	void setValue(int value, int cc) {
		if (value == lastValues[cc])
			return;
		lastValues[cc] = value;
		// CC
		midi::Message m;
		m.setStatus(0xb);
		m.setNote(cc);
		m.setValue(value);
		m.setFrame(frame);
		sendMessage(m);
	}

	void setFrame(double frame) {
		this->frame = frame;
	}
};


struct CV_MIDICC : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(CC_INPUTS, 16),
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	CCMidiOutput midiOutput;
	dsp::Timer rateLimiterTimer;
	int learningId = -1;
	int learnedCcs[16] = {};

	CV_MIDICC() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for (int i = 0; i < 16; i++)
			configInput(CC_INPUTS + i, string::f("Cell %d", i + 1));
		onReset();
	}

	void onReset() override {
		for (int i = 0; i < 16; i++) {
			learnedCcs[i] = i;
		}
		learningId = -1;
		midiOutput.reset();
		midiOutput.midi::Output::reset();
	}

	void process(const ProcessArgs& args) override {
		const float rateLimiterPeriod = 1 / 200.f;
		bool rateLimiterTriggered = (rateLimiterTimer.process(args.sampleTime) >= rateLimiterPeriod);
		if (rateLimiterTriggered)
			rateLimiterTimer.time -= rateLimiterPeriod;
		else
			return;

		midiOutput.setFrame(args.frame);

		for (int i = 0; i < 16; i++) {
			int value = (int) std::round(inputs[CC_INPUTS + i].getVoltage() / 10.f * 127);
			value = clamp(value, 0, 127);
			midiOutput.setValue(value, learnedCcs[i]);
		}
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		json_t* ccsJ = json_array();
		for (int i = 0; i < 16; i++) {
			json_array_append_new(ccsJ, json_integer(learnedCcs[i]));
		}
		json_object_set_new(rootJ, "ccs", ccsJ);

		json_object_set_new(rootJ, "midi", midiOutput.toJson());
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* ccsJ = json_object_get(rootJ, "ccs");
		if (ccsJ) {
			for (int i = 0; i < 16; i++) {
				json_t* ccJ = json_array_get(ccsJ, i);
				if (ccJ)
					learnedCcs[i] = json_integer_value(ccJ);
			}
		}

		json_t* midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiOutput.fromJson(midiJ);
	}
};


struct CV_MIDICCWidget : ModuleWidget {
	CV_MIDICCWidget(CV_MIDICC* module) {
		setModule(module);
		setPanel(Svg::load(asset::system("res/Core/CV_MIDICC.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.189, 78.431)), module, CV_MIDICC::CC_INPUTS + 0));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.739, 78.431)), module, CV_MIDICC::CC_INPUTS + 1));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31.289, 78.431)), module, CV_MIDICC::CC_INPUTS + 2));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.838, 78.431)), module, CV_MIDICC::CC_INPUTS + 3));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.189, 89.946)), module, CV_MIDICC::CC_INPUTS + 4));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.739, 89.946)), module, CV_MIDICC::CC_INPUTS + 5));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31.289, 89.946)), module, CV_MIDICC::CC_INPUTS + 6));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.838, 89.946)), module, CV_MIDICC::CC_INPUTS + 7));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.189, 101.466)), module, CV_MIDICC::CC_INPUTS + 8));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.739, 101.466)), module, CV_MIDICC::CC_INPUTS + 9));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31.289, 101.466)), module, CV_MIDICC::CC_INPUTS + 10));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.838, 101.466)), module, CV_MIDICC::CC_INPUTS + 11));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.189, 112.998)), module, CV_MIDICC::CC_INPUTS + 12));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.739, 112.984)), module, CV_MIDICC::CC_INPUTS + 13));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31.289, 112.984)), module, CV_MIDICC::CC_INPUTS + 14));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.838, 112.984)), module, CV_MIDICC::CC_INPUTS + 15));

		typedef Grid16MidiDisplay<CcChoice<CV_MIDICC>> TMidiDisplay;
		TMidiDisplay* display = createWidget<TMidiDisplay>(mm2px(Vec(0.0, 13.039)));
		display->box.size = mm2px(Vec(50.8, 55.88));
		display->setMidiPort(module ? &module->midiOutput : NULL);
		display->setModule(module);
		addChild(display);
	}
};


Model* modelCV_MIDICC = createModel<CV_MIDICC, CV_MIDICCWidget>("CV-CC");


} // namespace core
} // namespace rack
