#include "plugin.hpp"


namespace rack {
namespace core {


struct CCMidiOutput : midi::Output {
	int lastValues[128];

	CCMidiOutput() {
		reset();
	}

	void reset() {
		for (int n = 0; n < 128; n++) {
			lastValues[n] = -1;
		}
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
		sendMessage(m);
	}
};


struct CV_CC : Module {
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
	float rateLimiterPhase = 0.f;
	int learningId = -1;
	int learnedCcs[16] = {};

	CV_CC() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
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
		const float rateLimiterPeriod = 0.010f;
		rateLimiterPhase += args.sampleTime / rateLimiterPeriod;
		if (rateLimiterPhase >= 1.f) {
			rateLimiterPhase -= 1.f;
		}
		else {
			return;
		}

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


struct CV_CCWidget : ModuleWidget {
	CV_CCWidget(CV_CC* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::system("res/Core/CV-CC.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 77)), module, CV_CC::CC_INPUTS + 0));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 77)), module, CV_CC::CC_INPUTS + 1));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31, 77)), module, CV_CC::CC_INPUTS + 2));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43, 77)), module, CV_CC::CC_INPUTS + 3));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 89)), module, CV_CC::CC_INPUTS + 4));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 89)), module, CV_CC::CC_INPUTS + 5));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31, 89)), module, CV_CC::CC_INPUTS + 6));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43, 89)), module, CV_CC::CC_INPUTS + 7));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 101)), module, CV_CC::CC_INPUTS + 8));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 101)), module, CV_CC::CC_INPUTS + 9));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31, 101)), module, CV_CC::CC_INPUTS + 10));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43, 101)), module, CV_CC::CC_INPUTS + 11));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 112)), module, CV_CC::CC_INPUTS + 12));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 112)), module, CV_CC::CC_INPUTS + 13));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31, 112)), module, CV_CC::CC_INPUTS + 14));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43, 112)), module, CV_CC::CC_INPUTS + 15));

		typedef Grid16MidiWidget<CcChoice<CV_CC>> TMidiWidget;
		TMidiWidget* midiWidget = createWidget<TMidiWidget>(mm2px(Vec(3.399621, 14.837339)));
		midiWidget->box.size = mm2px(Vec(44, 54.667));
		midiWidget->setMidiPort(module ? &module->midiOutput : NULL);
		midiWidget->setModule(module);
		addChild(midiWidget);
	}
};


Model* modelCV_CC = createModel<CV_CC, CV_CCWidget>("CV-CC");


} // namespace core
} // namespace rack
