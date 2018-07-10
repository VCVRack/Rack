#include "dsp/digital.hpp"
#include <iostream>
#include "RJModules.hpp"

namespace rack_plugin_RJModules {

struct Mono : Module {
    enum ParamIds {
        MONO_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        CH1_INPUT,
        CH2_INPUT,
        MONO_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        CH1_OUTPUT,
        CH2_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    Mono() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void Mono::step() {

    float mono_amount = params[MONO_PARAM].value * clamp(inputs[MONO_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
    float mono_value = (inputs[CH1_INPUT].value + inputs[CH2_INPUT].value) / 2;

    outputs[CH1_OUTPUT].value = (mono_value * mono_amount) + (((1 - mono_amount)) * inputs[CH1_INPUT].value);
    outputs[CH2_OUTPUT].value = (mono_value * mono_amount) + (((1 - mono_amount)) * inputs[CH2_INPUT].value);

}

struct MonoWidget: ModuleWidget {
    MonoWidget(Mono *module);
};

MonoWidget::MonoWidget(Mono *module) : ModuleWidget(module) {
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Mono.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    addInput(Port::create<PJ301MPort>(Vec(22, 85), Port::INPUT, module, Mono::CH1_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(104, 85), Port::INPUT, module, Mono::CH2_INPUT));

    addParam(ParamWidget::create<RoundHugeBlackKnob>(Vec(47, 143), module, Mono::MONO_PARAM, 0.0, 1.0, 0.1));

    addInput(Port::create<PJ301MPort>(Vec(22, 190), Port::INPUT, module, Mono::MONO_CV_INPUT));

    addOutput(Port::create<PJ301MPort>(Vec(22, 255), Port::OUTPUT, module, Mono::CH1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(104, 255), Port::OUTPUT, module, Mono::CH2_OUTPUT));
}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, Mono) {
   Model *modelMono = Model::create<Mono, MonoWidget>("RJModules", "Mono", "[MIX] Mono", UTILITY_TAG);
   return modelMono;
}
