#include "RJModules.hpp"
#include <iostream>
#include <cmath>

namespace rack_plugin_RJModules {

struct Panner: Module {
    enum ParamIds {
        CH1_PARAM,
        CH2_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        CH1_INPUT,
        CH1_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        CH1_OUTPUT,
        CH2_OUTPUT,
        NUM_OUTPUTS
    };

    Panner() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
    void step() override;
};


void Panner::step() {
    float ch1 = inputs[CH1_INPUT].value;

    float combined_input = params[CH1_PARAM].value * clamp(inputs[CH1_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);

    float left_percent = combined_input;
    float right_percent = 1 - combined_input;

    outputs[CH2_OUTPUT].value = ch1 * left_percent;
    outputs[CH1_OUTPUT].value = ch1 * right_percent;
}


struct PannerWidget: ModuleWidget {
    PannerWidget(Panner *module);
};

PannerWidget::PannerWidget(Panner *module) : ModuleWidget(module) {
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Panner.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    addParam(ParamWidget::create<RoundBlackKnob>(Vec(57, 139), module, Panner::CH1_PARAM, 0.0, 1.0, 0.0));

    addInput(Port::create<PJ301MPort>(Vec(22, 129), Port::INPUT, module, Panner::CH1_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(22, 160), Port::INPUT, module, Panner::CH1_CV_INPUT));

    addOutput(Port::create<PJ301MPort>(Vec(110, 125), Port::OUTPUT, module, Panner::CH1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(110, 175), Port::OUTPUT, module, Panner::CH2_OUTPUT));
}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, Panner) {
   Model *modelPanner = Model::create<Panner, PannerWidget>("RJModules", "Panner", "[MIX] Panner", PANNING_TAG);
   return modelPanner;
}
