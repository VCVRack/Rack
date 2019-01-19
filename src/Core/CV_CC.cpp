#include "Core.hpp"


template <int N>
struct CCMidiOutput : midi::Output {
	int ccs[N];
	int lastValues[N];

	CCMidiOutput() {
		reset();
	}

	void reset() {
		for (int n = 0; n < N; n++) {
			ccs[n] = n;
			lastValues[n] = -1;
		}
	}

	void setCC(int cc, int n) {
		ccs[n] = cc;
	}

	void setValue(int value, int n) {
		if (value == lastValues[n])
			return;
		lastValues[n] = value;
		// CC
		midi::Message m;
		m.setStatus(0xb);
		m.setNote(ccs[n]);
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

	CCMidiOutput<16> midiOutput;
	float rateLimiterPhase = 0.f;

	CV_CC() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void step() override {
		const float rateLimiterPeriod = 0.010f;
		rateLimiterPhase += app()->engine->getSampleTime() / rateLimiterPeriod;
		if (rateLimiterPhase >= 1.f) {
			rateLimiterPhase -= 1.f;
		}
		else {
			return;
		}

		for (int n = 0; n < 16; n++) {
			int value = (int) std::round(inputs[CC_INPUTS + n].getVoltage() / 10.f * 127);
			value = clamp(value, 0, 127);
			midiOutput.setValue(value, n);
		}
	}
};


struct CV_CCWidget : ModuleWidget {
	CV_CCWidget(CV_CC *module) {
		setModule(module);
		setPanel(SVG::load(asset::system("res/Core/CV-CC.svg")));

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

		MidiWidget *midiWidget = createWidget<MidiWidget>(mm2px(Vec(3.4, 14.839)));
		midiWidget->box.size = mm2px(Vec(44, 54.667));
		if (module)
			midiWidget->midiIO = &module->midiOutput;
		// midiWidget->createGridChoices();
		addChild(midiWidget);
	}
};


Model *modelCV_CC = createModel<CV_CC, CV_CCWidget>("CV-CC");

