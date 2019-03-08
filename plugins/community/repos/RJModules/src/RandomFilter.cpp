#include "RJModules.hpp"
#include <iostream>
#include <cmath>
#include <random>
#include "dsp/digital.hpp"
#include "VAStateVariableFilter.h"

namespace rack_plugin_RJModules {

struct RandomFilter: Module {
    enum ParamIds {
        RESET_PARAM,
        MIX_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        CH1_INPUT,
        BUTTON_CV_INPUT,
        MIX_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        CH1_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        RESET_LIGHT,
        NUM_LIGHTS
    };

    float resetLight = 0.0;
    float last_press = 999999;

    SchmittTrigger resetTrigger;
    VAStateVariableFilter *rFilter = new VAStateVariableFilter() ; // create a lpFilter;

    RandomFilter() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

struct BigAssLEDButton : SVGSwitch, MomentarySwitch {
        BigAssLEDButton() {
                addFrame(SVG::load(assetPlugin(plugin, "res/BigLEDButton.svg")));
        }
};

template <typename BASE>
struct GiantLight : BASE {
        GiantLight() {
                this->box.size = mm2px(Vec(34, 34));
        }
};

void RandomFilter::step() {

    const float lightLambda = 0.075;
    float output = 0.0;

    // Reset
    if ((params[RESET_PARAM].value > 0 && last_press > 7000) || inputs[BUTTON_CV_INPUT].value > 0)   {
        resetLight = 1.0;
        last_press = 0;

        std::random_device rd; // obtain a random number from hardware
        std::mt19937 eng(rd()); // seed the generator
        std::uniform_int_distribution<> distr(0, 7); // define the range
        std::uniform_int_distribution<> distr2(-10, 10); // define the range
        std::uniform_int_distribution<> distr3(0, 7); // define the range

        float filter_type = distr(eng);

        rFilter->setFilterType(filter_type);
        // rFilter->setCutoffFreq(distr2(eng));
        // rFilter->setResonance(distr2(eng));
        rFilter->setSampleRate(engineGetSampleRate());

    }

    resetLight -= resetLight / lightLambda / engineGetSampleRate();

    float dry = inputs[CH1_INPUT].value;
    float wet = rFilter->processAudioSample(dry, 1);
    float mix_cv = clamp(inputs[MIX_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
    float mixed = (wet * (params[MIX_PARAM].value * mix_cv)) + (dry * ((1-params[MIX_PARAM].value) * mix_cv));

    outputs[CH1_OUTPUT].value = mixed;
    lights[RESET_LIGHT].value = resetLight;
    last_press++;
}


struct RandomFilterWidget: ModuleWidget {
    RandomFilterWidget(RandomFilter *module);
};

RandomFilterWidget::RandomFilterWidget(RandomFilter *module) : ModuleWidget(module) {
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/RandomFilter.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    addParam(ParamWidget::create<BigAssLEDButton>(Vec(15, 60), module, RandomFilter::RESET_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<RoundHugeBlackKnob>(Vec(47, 228), module, RandomFilter::MIX_PARAM, 0.0, 1.0, 1.0));

    addInput(Port::create<PJ301MPort>(Vec(22, 180), Port::INPUT, module, RandomFilter::BUTTON_CV_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(22, 260), Port::INPUT, module, RandomFilter::MIX_CV_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(22, 310), Port::INPUT, module, RandomFilter::CH1_INPUT));

    addChild(ModuleLightWidget::create<GiantLight<GreenLight>>(Vec(25, 70), module, RandomFilter::RESET_LIGHT));

    addOutput(Port::create<PJ301MPort>(Vec(110, 310), Port::OUTPUT, module, RandomFilter::CH1_OUTPUT));
}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, RandomFilter) {
   Model *modelRandomFilter = Model::create<RandomFilter, RandomFilterWidget>("RJModules", "RandomFilter", "[FILT] RandomFilter", UTILITY_TAG);
   return modelRandomFilter;
}
