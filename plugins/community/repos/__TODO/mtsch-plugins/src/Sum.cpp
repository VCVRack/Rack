#include "mtsch.hpp"
#include <iostream>

#define NUM_CHANNELS 8

// IO and switch positions.
#define X_POSITION 7
#define Y_INPUT_POSITION 77
#define Y_OUTPUT_POSITION 23
#define SPACING 35

struct Sum : Module {
    enum ParamIds {
        PARAMS,
        NUM_PARAMS = PARAMS + NUM_CHANNELS
    };
    enum InputIds {
        INPUTS,
        NUM_INPUTS = INPUTS + NUM_CHANNELS
    };
    enum OutputIds {
        OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    Sum() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void Sum::step() {
    float acc = 0;

    for (int i = 0; i < NUM_CHANNELS; i++) {
        acc += inputs[INPUTS + i].value * params[PARAMS + i].value;
    }

    outputs[OUTPUT].value = acc;
}

struct SumWidget : ModuleWidget {
	SumWidget(Sum *module);
};

SumWidget::SumWidget(Sum *module) : ModuleWidget(module) {
    box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Sum.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    for (int i = 0; i < NUM_CHANNELS; i++) {
        addInput(Port::create<PJ301MPort>(Vec(X_POSITION, Y_INPUT_POSITION + i * SPACING),
                                         Port::INPUT, module, Sum::INPUTS + i));
        addParam(ParamWidget::create<CKSSThree>(Vec(X_POSITION + 30, Y_INPUT_POSITION + i * SPACING),
                                        module, Sum::PARAMS + i, -1, 1, 1));
    }

    addOutput(Port::create<PJ301MPort>(Vec(X_POSITION, Y_OUTPUT_POSITION), Port::OUTPUT, module, Sum::OUTPUT));
}


Model *modelSum = Model::create<Sum, SumWidget>(
        "mtsch", "Sum", "Sum", UTILITY_TAG);
