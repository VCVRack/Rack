#include "RJModules.hpp"
#include <iostream>
#include <cmath>

namespace rack_plugin_RJModules {

struct Splitters: Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        CH1_INPUT,
        CH2_INPUT,
        CH3_INPUT,
        CH4_INPUT,
        CH5_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        CH1_OUTPUT,
        CH2_OUTPUT,
        CH3_OUTPUT,
        CH4_OUTPUT,
        CH5_OUTPUT,
        CH6_OUTPUT,
        CH7_OUTPUT,
        CH8_OUTPUT,
        CH9_OUTPUT,
        CH10_OUTPUT,
        NUM_OUTPUTS
    };

    Splitters() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
    void step() override;
};


#define ROUND(f) ((float)((f > 0.0) ? floor(f + 0.5) : ceil(f - 0.5)))

void Splitters::step() {

    outputs[CH1_OUTPUT].value = inputs[CH1_INPUT].value;
    outputs[CH2_OUTPUT].value = inputs[CH1_INPUT].value;

    outputs[CH3_OUTPUT].value = inputs[CH2_INPUT].value;
    outputs[CH4_OUTPUT].value = inputs[CH2_INPUT].value;

    outputs[CH5_OUTPUT].value = inputs[CH3_INPUT].value;
    outputs[CH6_OUTPUT].value = inputs[CH3_INPUT].value;

    outputs[CH7_OUTPUT].value = inputs[CH4_INPUT].value;
    outputs[CH8_OUTPUT].value = inputs[CH4_INPUT].value;

    outputs[CH9_OUTPUT].value = inputs[CH5_INPUT].value;
    outputs[CH10_OUTPUT].value = inputs[CH5_INPUT].value;

}

struct SplittersWidget: ModuleWidget {
    SplittersWidget(Splitters *module);
};

SplittersWidget::SplittersWidget(Splitters *module) : ModuleWidget(module) {
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Splitters.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    addInput(Port::create<PJ301MPort>(Vec(24, 73), Port::INPUT, module, Splitters::CH1_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(24, 123), Port::INPUT, module, Splitters::CH2_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(24, 173), Port::INPUT, module, Splitters::CH3_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(24, 223), Port::INPUT, module, Splitters::CH4_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(24, 274), Port::INPUT, module, Splitters::CH5_INPUT));

    addOutput(Port::create<PJ301MPort>(Vec(65, 73), Port::OUTPUT, module, Splitters::CH1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(105, 73), Port::OUTPUT, module, Splitters::CH2_OUTPUT));

    addOutput(Port::create<PJ301MPort>(Vec(65, 123), Port::OUTPUT, module, Splitters::CH3_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(105, 123), Port::OUTPUT, module, Splitters::CH4_OUTPUT));

    addOutput(Port::create<PJ301MPort>(Vec(65, 173), Port::OUTPUT, module, Splitters::CH5_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(105, 173), Port::OUTPUT, module, Splitters::CH6_OUTPUT));

    addOutput(Port::create<PJ301MPort>(Vec(65, 223), Port::OUTPUT, module, Splitters::CH7_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(105, 223), Port::OUTPUT, module, Splitters::CH8_OUTPUT));

    addOutput(Port::create<PJ301MPort>(Vec(65, 274), Port::OUTPUT, module, Splitters::CH9_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(105, 274), Port::OUTPUT, module, Splitters::CH10_OUTPUT));

}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, Splitters) {
   Model *modelSplitters = Model::create<Splitters, SplittersWidget>("RJModules", "Splitters", "[UTIL] Splitters", MULTIPLE_TAG);
   return modelSplitters;
}
