#include "mtsch.hpp"
#include "DigitDisplay.hpp"
#include <iostream>

#define NUM_CHANNELS 4

// The range of the knobs. 16 should be enough since Rationals can be stacked.
#define MAX_VALUE 16

// IO, digit and knob positions.
#define X_POS 13
#define Y_POS 80
#define CHANNEL_SPACING 70

#define KNOB_X_OFFSET 30

#define DIGIT_X_OFFSET 33
#define DIGIT_Y_OFFSET 4
#define DIGIT_SPACING 17
#define NUMDEN_SPACING 35

#define IO_Y_OFFSET 19
#define OUT_X_OFFSET KNOB_X_OFFSET + 71

#define GRID_X_POS 52
#define GRID_Y_POS 11
#define GRID_SPACING 24

struct Rationals : Module {
    enum ParamIds {
        PARAMS,
        NUM_PARAMS = PARAMS + 2 * NUM_CHANNELS
    };
    enum InputIds {
        INPUTS,
        NUM_INPUTS = INPUTS + 3 * NUM_CHANNELS
    };
    enum OutputIds {
        OUTPUTS,
        NUM_OUTPUTS = OUTPUTS + NUM_CHANNELS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    Rationals() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    char display[4*NUM_CHANNELS];
};

void Rationals::step() {

    for (int i = 0; i < NUM_CHANNELS; i++) {
        float num_cv  = std::round(inputs[INPUTS + 1 + 3*i].value);
        float num_par = std::round(params[PARAMS + 2*i].value);
        float den_cv  = std::round(inputs[INPUTS + 2 + 3*i].value);
        float den_par = std::round(params[PARAMS + 1 + 2*i].value);

        float num = num_cv + num_par;
        float den = den_cv + den_par;

        num = num > 0 ? num : 1;
        den = den > 0 ? den : 1;

        int digit_offset = NUM_CHANNELS * i;
        int dig1 = int(num / 10.0) % 10;
        int dig2 = int(num) % 10;
        if (dig1 != 0) {
            display[0 + digit_offset] = dig1 + '0';
        } else {
            display[0 + digit_offset] = '\0';
        }
        display[1 + digit_offset] = dig2 + '0';

        dig1 = int(den / 10.0) % 10;
        dig2 = int(den) % 10;
        if (dig1 != 0) {
            display[2 + digit_offset] = dig1 + '0';
        } else {
            display[2 + digit_offset] = '\0';
        }
        display[3 + digit_offset] = dig2 + '0';

        outputs[OUTPUTS + i].value = inputs[INPUTS + 3*i].value + log2f(num/den);
    }
}

struct RationalsWidget : ModuleWidget {
    RationalsWidget(Rationals *module);
};


RationalsWidget::RationalsWidget(Rationals *module) : ModuleWidget(module) {

    box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Rationals.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    for (int i = 0; i < NUM_CHANNELS; i++) {
        int y_pos = Y_POS + i * CHANNEL_SPACING;

        int offset = NUM_CHANNELS * i;
        int digit_x = X_POS + KNOB_X_OFFSET + DIGIT_X_OFFSET;
        // Numerator digits.
        addChild(new DigitDisplay(Vec(digit_x, y_pos + DIGIT_Y_OFFSET),
                                  5.f, &module->display[0+offset]));
        addChild(new DigitDisplay(Vec(digit_x + DIGIT_SPACING, y_pos + DIGIT_Y_OFFSET),
                                  5.f, &module->display[1+offset]));
        // Denominator digits.
        addChild(new DigitDisplay(Vec(digit_x, y_pos + DIGIT_Y_OFFSET + NUMDEN_SPACING),
                                  5.f, &module->display[2+offset]));
        addChild(new DigitDisplay(Vec(digit_x + DIGIT_SPACING, y_pos + DIGIT_Y_OFFSET + NUMDEN_SPACING),
                                  5.f, &module->display[3+offset]));

        // Numerator knob.
        addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(X_POS + KNOB_X_OFFSET, y_pos),
                                                  module, Rationals::PARAMS + 2*i, 1, MAX_VALUE, 1));
        // Denominator knob.
        addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(X_POS + KNOB_X_OFFSET, y_pos + NUMDEN_SPACING),
                                                  module, Rationals::PARAMS + 1 + 2*i, 1, MAX_VALUE, 1));

        // IO.
        addInput(Port::create<PJ301MPort>(Vec(X_POS, y_pos + IO_Y_OFFSET),
                                         Port::INPUT, module, Rationals::INPUTS + i*3));
        addOutput(Port::create<PJ301MPort>(Vec(X_POS + OUT_X_OFFSET, y_pos + IO_Y_OFFSET),
                                           Port::OUTPUT, module, Rationals::OUTPUTS + i));
    }

    // CV mod grid.
    for (int i = 0; i < NUM_CHANNELS; i++) {
        addInput(Port::create<PJ301MPort>(Vec(GRID_X_POS + i*GRID_SPACING, GRID_Y_POS),
                                         Port::INPUT, module, Rationals::INPUTS + 1 + i*3));
        addInput(Port::create<PJ301MPort>(Vec(GRID_X_POS + i*GRID_SPACING, GRID_Y_POS + GRID_SPACING),
                                         Port::INPUT, module, Rationals::INPUTS + 2 + i*3));
    }
}

Model *modelRationals = Model::create<Rationals, RationalsWidget>(
        "mtsch", "Rationals", "Rationals", UTILITY_TAG);
