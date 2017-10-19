#include "core.hpp"

using namespace rack;


struct Bridge : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};

	Bridge() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
	}
	~Bridge() {
	}
	void step() override;
};


void Bridge::step() {
}


BridgeWidget::BridgeWidget() {
	Bridge *module = new Bridge();
	setModule(module);
	box.size = Vec(15*8, 380);

	{
		Panel *panel = new LightPanel();
		panel->box.size = box.size;
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 365)));
}
