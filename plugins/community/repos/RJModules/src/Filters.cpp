#include <iostream>
#include <stdlib.h>

#include "RJModules.hpp"
#include "dsp/digital.hpp"
#include "VAStateVariableFilter.h"

namespace rack_plugin_RJModules {

#define NUM_CHANNELS 10

struct Filters : Module {
    enum ParamIds {
        MUTE_PARAM,
        NUM_PARAMS = MUTE_PARAM + NUM_CHANNELS
    };
    enum InputIds {
        IN_INPUT,
        NUM_INPUTS = IN_INPUT + NUM_CHANNELS
    };
    enum OutputIds {
        OUT_OUTPUT,
        NUM_OUTPUTS = OUT_OUTPUT + NUM_CHANNELS
    };

    bool state[NUM_CHANNELS];

    VAStateVariableFilter lpFilters[NUM_CHANNELS];
    VAStateVariableFilter hpFilters[NUM_CHANNELS];
    VAStateVariableFilter *lpFilter = new VAStateVariableFilter() ; // create a lpFilter;
    VAStateVariableFilter *hpFilter = new VAStateVariableFilter() ; // create a hpFilter;

    Filters() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
        reset();
    }
    void step() override;

    void reset() override {
        for (int i = 0; i < NUM_CHANNELS; i++) {
            state[i] = true;
        }
    }
    void randomize() override {
        for (int i = 0; i < NUM_CHANNELS; i++) {
            state[i] = (randomUniform() < 0.5);
        }
    }

    json_t *toJson() override {
        json_t *rootJ = json_object();
        // states
        json_t *statesJ = json_array();
        for (int i = 0; i < NUM_CHANNELS; i++) {
            json_t *stateJ = json_boolean(state[i]);
            json_array_append_new(statesJ, stateJ);
        }
        json_object_set_new(rootJ, "states", statesJ);
        return rootJ;
    }
    void fromJson(json_t *rootJ) override {
        // states
        json_t *statesJ = json_object_get(rootJ, "states");
        if (statesJ) {
            for (int i = 0; i < NUM_CHANNELS; i++) {
                json_t *stateJ = json_array_get(statesJ, i);
                if (stateJ)
                    state[i] = json_boolean_value(stateJ);
            }
        }
    }
};

void Filters::step() {

    VAStateVariableFilter *lpFilter;
    VAStateVariableFilter *hpFilter;

    for (int i = 0; i < NUM_CHANNELS; i++) {
        lpFilter = &lpFilters[i];
        hpFilter = &hpFilters[i];

        lpFilter->setFilterType(0);
        hpFilter->setFilterType(2);

        lpFilter->setResonance(.7);
        hpFilter->setResonance(.7);

        lpFilter->setSampleRate(engineGetSampleRate());
        hpFilter->setSampleRate(engineGetSampleRate());

        float dry = inputs[IN_INPUT + i].value;
        float param = params[MUTE_PARAM + i].value;
        float wet = dry;

        // if param < .5
        //      LPF, 30:8000
        // if param > .5
        //      HPF, 30:8000
        // if param == .5, wet = dry

        if(param < .5){

            // new_value = ( (old_value - old_min) / (old_max - old_min) ) * (new_max - new_min) + new_min
            float lp_cutoff = ( (param - 0) / (.5 - 0.0)) * (8000.0 - 30.0) + 30.0;
            lpFilter->setCutoffFreq(lp_cutoff);
            wet = lpFilter->processAudioSample(dry, 1);
        }
        if(param > .5){

            // new_value = ( (old_value - old_min) / (old_max - old_min) ) * (new_max - new_min) + new_min
            float hp_cutoff = ( (param - .5) / (1.0 - 0.5)) * (8000.0 - 200.0) + 200.0;
            hpFilter->setCutoffFreq(hp_cutoff);
            wet = hpFilter->processAudioSample(dry, 1);
        }
        if(param == .5){
            wet = dry;
        }

        outputs[OUT_OUTPUT + i].value = wet;

    }
}


template <typename BASE>
struct MuteLight : BASE {
    MuteLight() {
        this->box.size = mm2px(Vec(6.0, 6.0));
    }
};

struct FiltersWidget: ModuleWidget {
    FiltersWidget(Filters *module);
};

FiltersWidget::FiltersWidget(Filters *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/Filters.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 17.165)), module, Filters::MUTE_PARAM + 0, 0.0, 1.0, 0.5));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 27.164)), module, Filters::MUTE_PARAM + 1, 0.0, 1.0, 0.5));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 37.164)), module, Filters::MUTE_PARAM + 2, 0.0, 1.0, 0.5));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 47.165)), module, Filters::MUTE_PARAM + 3, 0.0, 1.0, 0.5));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 57.164)), module, Filters::MUTE_PARAM + 4, 0.0, 1.0, 0.5));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 67.165)), module, Filters::MUTE_PARAM + 5, 0.0, 1.0, 0.5));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 77.164)), module, Filters::MUTE_PARAM + 6, 0.0, 1.0, 0.5));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 87.164)), module, Filters::MUTE_PARAM + 7, 0.0, 1.0, 0.5));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 97.165)), module, Filters::MUTE_PARAM + 8, 0.0, 1.0, 0.5));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 107.166)), module, Filters::MUTE_PARAM + 9, 0.0, 1.0, 0.5));

    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 17.81)), Port::INPUT, module, Filters::IN_INPUT + 0));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 27.809)), Port::INPUT, module, Filters::IN_INPUT + 1));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 37.809)), Port::INPUT, module, Filters::IN_INPUT + 2));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 47.81)), Port::INPUT, module, Filters::IN_INPUT + 3));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 57.81)), Port::INPUT, module, Filters::IN_INPUT + 4));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 67.809)), Port::INPUT, module, Filters::IN_INPUT + 5));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 77.81)), Port::INPUT, module, Filters::IN_INPUT + 6));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 87.81)), Port::INPUT, module, Filters::IN_INPUT + 7));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 97.809)), Port::INPUT, module, Filters::IN_INPUT + 8));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 107.809)), Port::INPUT, module, Filters::IN_INPUT + 9));

    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 17.81)), Port::OUTPUT, module, Filters::OUT_OUTPUT + 0));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 27.809)), Port::OUTPUT, module, Filters::OUT_OUTPUT + 1));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 37.809)), Port::OUTPUT, module, Filters::OUT_OUTPUT + 2));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 47.81)), Port::OUTPUT, module, Filters::OUT_OUTPUT + 3));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 57.809)), Port::OUTPUT, module, Filters::OUT_OUTPUT + 4));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 67.809)), Port::OUTPUT, module, Filters::OUT_OUTPUT + 5));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 77.81)), Port::OUTPUT, module, Filters::OUT_OUTPUT + 6));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 87.81)), Port::OUTPUT, module, Filters::OUT_OUTPUT + 7));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 97.809)), Port::OUTPUT, module, Filters::OUT_OUTPUT + 8));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 107.809)), Port::OUTPUT, module, Filters::OUT_OUTPUT + 9));
}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, Filters) {
   Model *modelFilters = Model::create<Filters, FiltersWidget>("RJModules", "Filters", "[FILT] Filters", FILTER_TAG);
   return modelFilters;
}
