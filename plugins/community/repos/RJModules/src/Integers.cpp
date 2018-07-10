#include "RJModules.hpp"
#include <iostream>
#include <cmath>

namespace rack_plugin_RJModules {

struct Integers: Module {
    enum ParamIds {
        CH1_PARAM,
        CH2_PARAM,
        CH3_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        CH1_CV_INPUT,
        CH2_CV_INPUT,
        CH3_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        CH1_OUTPUT,
        CH2_OUTPUT,
        CH3_OUTPUT,
        NUM_OUTPUTS
    };

    Integers() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
    void step() override;
};


#define ROUND(f) ((float)((f > 0.0) ? floor(f + 0.5) : ceil(f - 0.5)))

void Integers::step() {
    float combined_input_1 = params[CH1_PARAM].value * clamp(inputs[CH1_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
    float combined_input_2 = params[CH2_PARAM].value * clamp(inputs[CH2_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
    float combined_input_3 = params[CH3_PARAM].value * clamp(inputs[CH3_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);

    // new_value = ( (old_value - old_min) / (old_max - old_min) ) * (new_max - new_min) + new_min
    float mapped_input_1 = ((combined_input_1 - 0.0) / (1.0 - 0.0) ) * (12.0 - -12.0) + -12.0;
    float mapped_input_2 = ((combined_input_2 - 0.0) / (1.0 - 0.0) ) * (12.0 - -12.0) + -12.0;
    float mapped_input_3 = ((combined_input_3 - 0.0) / (1.0 - 0.0) ) * (12.0 - -12.0) + -12.0;

    int cast_input_1 = static_cast<int>(mapped_input_1);
    int cast_input_2 = static_cast<int>(mapped_input_2);
    int cast_input_3 = static_cast<int>(mapped_input_3);

    outputs[CH1_OUTPUT].value = cast_input_1;
    outputs[CH2_OUTPUT].value = cast_input_2;
    outputs[CH3_OUTPUT].value = cast_input_3;
}


struct IntegersWidget: ModuleWidget {
    IntegersWidget(Integers *module);
};

IntegersWidget::IntegersWidget(Integers *module) : ModuleWidget(module) {
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Integers.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    addParam(ParamWidget::create<RoundBlackKnob>(Vec(57, 79), module, Integers::CH1_PARAM, 0.0, 1.0, 0.5));
    addParam(ParamWidget::create<RoundBlackKnob>(Vec(57, 159), module, Integers::CH2_PARAM, 0.0, 1.0, 0.5));
    addParam(ParamWidget::create<RoundBlackKnob>(Vec(57, 239), module, Integers::CH3_PARAM, 0.0, 1.0, 0.5));

    addInput(Port::create<PJ301MPort>(Vec(22, 100), Port::INPUT, module, Integers::CH1_CV_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(22, 180), Port::INPUT, module, Integers::CH2_CV_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(22, 260), Port::INPUT, module, Integers::CH3_CV_INPUT));

    addOutput(Port::create<PJ301MPort>(Vec(110, 85), Port::OUTPUT, module, Integers::CH1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(110, 165), Port::OUTPUT, module, Integers::CH2_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(110, 245), Port::OUTPUT, module, Integers::CH3_OUTPUT));
}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, Integers) {
   Model *modelIntegers = Model::create<Integers, IntegersWidget>("RJModules", "Integers", "[NUM] Integers", UTILITY_TAG);
   return modelIntegers;
}
