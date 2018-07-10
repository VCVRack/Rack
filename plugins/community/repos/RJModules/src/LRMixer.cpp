#include "RJModules.hpp"
#include <iostream>
#include <cmath>

namespace rack_plugin_RJModules {

struct LRMixer: Module {
    enum ParamIds {
        CH1_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        L1_INPUT,
        L2_INPUT,
        L3_INPUT,
        L4_INPUT,
        L5_INPUT,
        L6_INPUT,

        R1_INPUT,
        R2_INPUT,
        R3_INPUT,
        R4_INPUT,
        R5_INPUT,
        R6_INPUT,

        NUM_INPUTS
    };
    enum OutputIds {
        CH1_OUTPUT,
        CH2_OUTPUT,
        NUM_OUTPUTS
    };

    LRMixer() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
    void step() override;
};


#define ROUND(f) ((float)((f > 0.0) ? floor(f + 0.5) : ceil(f - 0.5)))

void LRMixer::step() {

    outputs[CH1_OUTPUT].value = (inputs[L1_INPUT].value + inputs[L2_INPUT].value + inputs[L3_INPUT].value + inputs[L4_INPUT].value + inputs[L5_INPUT].value + inputs[L6_INPUT].value) * params[CH1_PARAM].value;
    outputs[CH2_OUTPUT].value = (inputs[R1_INPUT].value + inputs[R2_INPUT].value + inputs[R3_INPUT].value + inputs[R4_INPUT].value + inputs[R5_INPUT].value + inputs[R6_INPUT].value) * params[CH1_PARAM].value;
}

struct LRMixerWidget: ModuleWidget {
    LRMixerWidget(LRMixer *module);
};

LRMixerWidget::LRMixerWidget(LRMixer *module) : ModuleWidget(module) {
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/LRMixer.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(23, 320), module, LRMixer::CH1_PARAM, 0.0, 1.0, 1.0));

    addInput(Port::create<PJ301MPort>(Vec(25, 96), Port::INPUT, module, LRMixer::L1_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(65, 96), Port::INPUT, module, LRMixer::L2_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(105, 96), Port::INPUT, module, LRMixer::L3_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(25, 148), Port::INPUT, module, LRMixer::L4_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(65, 148), Port::INPUT, module, LRMixer::L5_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(105, 148), Port::INPUT, module, LRMixer::L6_INPUT));

    addInput(Port::create<PJ301MPort>(Vec(25, 220), Port::INPUT, module, LRMixer::R1_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(65, 220), Port::INPUT, module, LRMixer::R2_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(105, 220), Port::INPUT, module, LRMixer::R3_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(25, 270), Port::INPUT, module, LRMixer::R4_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(65, 270), Port::INPUT, module, LRMixer::R5_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(105, 270), Port::INPUT, module, LRMixer::R6_INPUT));

    addOutput(Port::create<PJ301MPort>(Vec(65, 322), Port::OUTPUT, module, LRMixer::CH1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(104, 322), Port::OUTPUT, module, LRMixer::CH2_OUTPUT));

}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, LRMixer) {
   Model *modelLRMixer = Model::create<LRMixer, LRMixerWidget>("RJModules", "LRMixer", "[MIX] LRMixer", MIXER_TAG);
   return modelLRMixer;
}
