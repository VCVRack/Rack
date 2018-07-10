#include "RJModules.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_RJModules {

#define NUM_CHANNELS 10

struct Volumes : Module {
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
    enum LightIds {
        MUTE_LIGHT,
        NUM_LIGHTS = MUTE_LIGHT + NUM_CHANNELS
    };

    bool state[NUM_CHANNELS];
    SchmittTrigger muteTrigger[NUM_CHANNELS];

    Volumes() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
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

void Volumes::step() {
    for (int i = 0; i < NUM_CHANNELS; i++) {
        float in = inputs[IN_INPUT + i].value;
        outputs[OUT_OUTPUT + i].value = in * params[MUTE_PARAM + i].value;
    }
}


template <typename BASE>
struct MuteLight : BASE {
    MuteLight() {
        this->box.size = mm2px(Vec(6.0, 6.0));
    }
};

struct VolumesWidget: ModuleWidget {
    VolumesWidget(Volumes *module);
};

VolumesWidget::VolumesWidget(Volumes *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/Volumes.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 17.165)), module, Volumes::MUTE_PARAM + 0, 0.0, 2.0, 1.0));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 27.164)), module, Volumes::MUTE_PARAM + 1, 0.0, 2.0, 1.0));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 37.164)), module, Volumes::MUTE_PARAM + 2, 0.0, 2.0, 1.0));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 47.165)), module, Volumes::MUTE_PARAM + 3, 0.0, 2.0, 1.0));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 57.164)), module, Volumes::MUTE_PARAM + 4, 0.0, 2.0, 1.0));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 67.165)), module, Volumes::MUTE_PARAM + 5, 0.0, 2.0, 1.0));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 77.164)), module, Volumes::MUTE_PARAM + 6, 0.0, 2.0, 1.0));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 87.164)), module, Volumes::MUTE_PARAM + 7, 0.0, 2.0, 1.0));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 97.165)), module, Volumes::MUTE_PARAM + 8, 0.0, 2.0, 1.0));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(mm2px(Vec(15.57, 107.166)), module, Volumes::MUTE_PARAM + 9, 0.0, 2.0, 1.0));

    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 17.81)), Port::INPUT, module, Volumes::IN_INPUT + 0));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 27.809)), Port::INPUT, module, Volumes::IN_INPUT + 1));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 37.809)), Port::INPUT, module, Volumes::IN_INPUT + 2));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 47.81)), Port::INPUT, module, Volumes::IN_INPUT + 3));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 57.81)), Port::INPUT, module, Volumes::IN_INPUT + 4));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 67.809)), Port::INPUT, module, Volumes::IN_INPUT + 5));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 77.81)), Port::INPUT, module, Volumes::IN_INPUT + 6));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 87.81)), Port::INPUT, module, Volumes::IN_INPUT + 7));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 97.809)), Port::INPUT, module, Volumes::IN_INPUT + 8));
    addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 107.809)), Port::INPUT, module, Volumes::IN_INPUT + 9));

    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 17.81)), Port::OUTPUT, module, Volumes::OUT_OUTPUT + 0));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 27.809)), Port::OUTPUT, module, Volumes::OUT_OUTPUT + 1));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 37.809)), Port::OUTPUT, module, Volumes::OUT_OUTPUT + 2));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 47.81)), Port::OUTPUT, module, Volumes::OUT_OUTPUT + 3));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 57.809)), Port::OUTPUT, module, Volumes::OUT_OUTPUT + 4));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 67.809)), Port::OUTPUT, module, Volumes::OUT_OUTPUT + 5));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 77.81)), Port::OUTPUT, module, Volumes::OUT_OUTPUT + 6));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 87.81)), Port::OUTPUT, module, Volumes::OUT_OUTPUT + 7));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 97.809)), Port::OUTPUT, module, Volumes::OUT_OUTPUT + 8));
    addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 107.809)), Port::OUTPUT, module, Volumes::OUT_OUTPUT + 9));
}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, Volumes) {
   Model *modelVolumes = Model::create<Volumes, VolumesWidget>("RJModules", "Volumes", "[MIX] Volumes", AMPLIFIER_TAG);
   return modelVolumes;
}
