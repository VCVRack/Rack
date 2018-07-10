#include "RJModules.hpp"
#include <iostream>
#include <cmath>
#include <random>

namespace rack_plugin_RJModules {

struct Randoms: Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        CH1_CV_INPUT_1,
        CH1_CV_INPUT_2,
        CH2_CV_INPUT_1,
        CH2_CV_INPUT_2,
        CH3_CV_INPUT_1,
        CH3_CV_INPUT_2,
        NUM_INPUTS
    };
    enum OutputIds {
        CH1_OUTPUT,
        CH2_OUTPUT,
        CH3_OUTPUT,
        NUM_OUTPUTS
    };

    Randoms() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
    void step() override;
};

void Randoms::step() {

    float mapped_ch1v1 = inputs[CH1_CV_INPUT_1].value;
    float mapped_ch1v2 = inputs[CH1_CV_INPUT_2].value;
    float mapped_ch2v1 = inputs[CH2_CV_INPUT_1].value;
    float mapped_ch2v2 = inputs[CH2_CV_INPUT_2].value;
    float mapped_ch3v1 = inputs[CH3_CV_INPUT_1].value;
    float mapped_ch3v2 = inputs[CH3_CV_INPUT_2].value;

    std::random_device rd; // obtain a random number from hardware
    std::mt19937 eng(rd()); // seed the generator

    if (mapped_ch1v1 == mapped_ch1v2){
        mapped_ch1v1 = -12;
        mapped_ch1v2 = 12;
    }
    std::uniform_real_distribution<> distr1(mapped_ch1v1, mapped_ch1v2); // define the range
    outputs[CH1_OUTPUT].value = distr1(eng);

    if (mapped_ch2v1 == mapped_ch2v2){
        mapped_ch2v1 = -12;
        mapped_ch2v2 = 12;
    }
    std::uniform_real_distribution<> distr2(mapped_ch2v1, mapped_ch2v2);
    outputs[CH1_OUTPUT].value = distr1(eng);

    if (mapped_ch3v1 == mapped_ch3v2){
        mapped_ch3v1 = -12;
        mapped_ch3v2 = 12;
    }
    std::uniform_real_distribution<> distr3(mapped_ch3v1, mapped_ch3v2);
    outputs[CH1_OUTPUT].value = distr1(eng);
}

struct RandomsWidget: ModuleWidget {
    RandomsWidget(Randoms *module);
};

RandomsWidget::RandomsWidget(Randoms *module) : ModuleWidget(module) {
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Randoms.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));


    addInput(Port::create<PJ301MPort>(Vec(22, 70), Port::INPUT, module, Randoms::CH1_CV_INPUT_1));
    addInput(Port::create<PJ301MPort>(Vec(22, 100), Port::INPUT, module, Randoms::CH1_CV_INPUT_2));

    addInput(Port::create<PJ301MPort>(Vec(22, 150), Port::INPUT, module, Randoms::CH2_CV_INPUT_1));
    addInput(Port::create<PJ301MPort>(Vec(22, 180), Port::INPUT, module, Randoms::CH2_CV_INPUT_2));

    addInput(Port::create<PJ301MPort>(Vec(22, 230), Port::INPUT, module, Randoms::CH3_CV_INPUT_1));
    addInput(Port::create<PJ301MPort>(Vec(22, 260), Port::INPUT, module, Randoms::CH3_CV_INPUT_2));

    addOutput(Port::create<PJ301MPort>(Vec(110, 85), Port::OUTPUT, module, Randoms::CH1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(110, 165), Port::OUTPUT, module, Randoms::CH2_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(110, 245), Port::OUTPUT, module, Randoms::CH3_OUTPUT));
}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, Randoms) {
   Model *modelRandoms = Model::create<Randoms, RandomsWidget>("RJModules", "Randoms", "[NUM] Randoms", UTILITY_TAG);
   return modelRandoms;
}
