#include "AudibleInstruments.hpp"
#include "dsp/digital.hpp"


struct Branches : Module {
	enum ParamIds {
		THRESHOLD1_PARAM,
		THRESHOLD2_PARAM,
		MODE1_PARAM,
		MODE2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN1_INPUT,
		IN2_INPUT,
		P1_INPUT,
		P2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1A_OUTPUT,
		OUT2A_OUTPUT,
		OUT1B_OUTPUT,
		OUT2B_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		MODE1_LIGHT,
		MODE2_LIGHT,
		STATE1_POS_LIGHT, STATE1_NEG_LIGHT,
		STATE2_POS_LIGHT, STATE2_NEG_LIGHT,
		NUM_LIGHTS
	};

	SchmittTrigger gateTrigger[2];
	SchmittTrigger modeTrigger[2];
	bool mode[2] = {};
	bool outcome[2] = {};

	Branches() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	void onReset() override {
		for (int i = 0; i < 2; i++) {
			mode[i] = false;
			outcome[i] = false;
		}
	}
};


void Branches::step() {
	float gate = 0.0;
	for (int i = 0; i < 2; i++) {
		// mode button
		if (modeTrigger[i].process(params[MODE1_PARAM + i].value))
			mode[i] = !mode[i];

		if (inputs[IN1_INPUT + i].active)
			gate = inputs[IN1_INPUT + i].value;

		if (gateTrigger[i].process(gate)) {
			// trigger
			float r = randomUniform();
			float threshold = clamp(params[THRESHOLD1_PARAM + i].value + inputs[P1_INPUT + i].value / 10.f, 0.f, 1.f);
			bool toss = (r < threshold);
			if (!mode[i]) {
				// direct mode
				outcome[i] = toss;
			}
			else {
				// toggle mode
				outcome[i] = (outcome[i] != toss);
			}

			if (!outcome[i])
				lights[STATE1_POS_LIGHT + 2*i].value = 1.0;
			else
				lights[STATE1_NEG_LIGHT + 2*i].value = 1.0;
		}

		lights[STATE1_POS_LIGHT + 2*i].value *= 1.0 - engineGetSampleTime() * 15.0;
		lights[STATE1_NEG_LIGHT + 2*i].value *= 1.0 - engineGetSampleTime() * 15.0;
		lights[MODE1_LIGHT + i].value = mode[i] ? 1.0 : 0.0;

		outputs[OUT1A_OUTPUT + i].value = outcome[i] ? 0.0 : gate;
		outputs[OUT1B_OUTPUT + i].value = outcome[i] ? gate : 0.0;
	}
}


struct BranchesWidget : ModuleWidget {
	BranchesWidget(Branches *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Branches.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

		addParam(ParamWidget::create<Rogan1PSRed>(Vec(24, 64), module, Branches::THRESHOLD1_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<TL1105>(Vec(69, 58), module, Branches::MODE1_PARAM, 0.0, 1.0, 0.0));
		addInput(Port::create<PJ301MPort>(Vec(9, 122), Port::INPUT, module, Branches::IN1_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(55, 122), Port::INPUT, module, Branches::P1_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(9, 160), Port::OUTPUT, module, Branches::OUT1A_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(55, 160), Port::OUTPUT, module, Branches::OUT1B_OUTPUT));

		addParam(ParamWidget::create<Rogan1PSGreen>(Vec(24, 220), module, Branches::THRESHOLD2_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<TL1105>(Vec(69, 214), module, Branches::MODE2_PARAM, 0.0, 1.0, 0.0));
		addInput(Port::create<PJ301MPort>(Vec(9, 278), Port::INPUT, module, Branches::IN2_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(55, 278), Port::INPUT, module, Branches::P2_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(9, 316), Port::OUTPUT, module, Branches::OUT2A_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(55, 316), Port::OUTPUT, module, Branches::OUT2B_OUTPUT));

		addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(40, 169), module, Branches::STATE1_POS_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(40, 325), module, Branches::STATE2_POS_LIGHT));
	}

	void appendContextMenu(Menu *menu) override {
		Branches *branches = dynamic_cast<Branches*>(module);
		assert(branches);

		struct BranchesModeItem : MenuItem {
			Branches *branches;
			int channel;
			void onAction(EventAction &e) override {
				branches->mode[channel] ^= 1;
			}
			void step() override {
				rightText = branches->mode[channel] ? "Toggle" : "Latch";
				MenuItem::step();
			}
		};

		menu->addChild(construct<MenuLabel>());

		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Channels"));
		menu->addChild(construct<BranchesModeItem>(&MenuItem::text, "Channel 1 mode", &BranchesModeItem::branches, branches, &BranchesModeItem::channel, 0));
		menu->addChild(construct<BranchesModeItem>(&MenuItem::text, "Channel 2 mode", &BranchesModeItem::branches, branches, &BranchesModeItem::channel, 1));
	}
};

RACK_PLUGIN_MODEL_INIT(AudibleInstruments, Branches) {
   Model *modelBranches = Model::create<Branches, BranchesWidget>("Audible Instruments", "Branches", "Bernoulli Gate", RANDOM_TAG, DUAL_TAG);
   return modelBranches;
}

