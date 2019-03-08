#include "QuantalAudio.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_QuantalAudio {

struct DaisyMaster : Module {
    enum ParamIds {
        MIX_LVL_PARAM,
        MUTE_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        MIX_CV_INPUT,
        CHAIN_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        MIX_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightsIds {
        MUTE_LIGHT,
        NUM_LIGHTS
    };

    // Hypothetically the max number of channels that could be chained
    // Needs to match the divisor in the daisy channel class
    float DAISY_DIVISOR = 16.f;
    bool muted = false;
    SchmittTrigger muteTrigger;

    DaisyMaster() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    json_t *toJson() override {
        json_t *rootJ = json_object();

        // mute
        json_object_set_new(rootJ, "muted", json_boolean(muted));

        return rootJ;
    }

    void fromJson(json_t *rootJ) override {
        // mute
        json_t *mutedJ = json_object_get(rootJ, "muted");
        if (mutedJ)
            muted = json_is_true(mutedJ);
    }

    void step() override {
        if (muteTrigger.process(params[MUTE_PARAM].value)) {
            muted = !muted;
        }

        float mix = 0.f;
        if (!muted) {
            // Bring the voltage back up from the chained low voltage
            mix = clamp(inputs[CHAIN_INPUT].value * DAISY_DIVISOR, -12.f, 12.f);
            mix *= params[MIX_LVL_PARAM].value;

            float mix_cv = 1.f;
            if (inputs[MIX_CV_INPUT].active)
                mix_cv = clamp(inputs[MIX_CV_INPUT].value / 10.f, 0.f, 1.f);
            mix *= mix_cv;
        }

        outputs[MIX_OUTPUT].value = mix;
        lights[MUTE_LIGHT].value = (muted);
    }
};

struct DaisyMasterWidget : ModuleWidget {
    DaisyMasterWidget(DaisyMaster *module) : ModuleWidget(module) {
        setPanel(SVG::load(assetPlugin(plugin, "res/DaisyMaster.svg")));

        // Screws
        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Level & CV
        addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(RACK_GRID_WIDTH * 1.5 - (38.0/2), 52.0), module, DaisyMaster::MIX_LVL_PARAM, 0.0, 2.0, 1.0));
        addInput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH * 1.5 - (25.0/2), 96.0), Port::INPUT, module, DaisyMaster::MIX_CV_INPUT));

        // Mute
        addParam(ParamWidget::create<LEDButton>(Vec(RACK_GRID_WIDTH * 1.5 - 9.0, 206.0), module, DaisyMaster::MUTE_PARAM, 0.0f, 1.0f, 0.0f));
        addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(RACK_GRID_WIDTH * 1.5 - 4.5, 210.25f), module, DaisyMaster::MUTE_LIGHT));

        // Mix output
        addOutput(Port::create<PJ301MPort>(Vec((RACK_GRID_WIDTH * 1.5) - (25.0/2), 245.0), Port::OUTPUT, module, DaisyMaster::MIX_OUTPUT));

        // Chain input
        addInput(Port::create<PJ301MPort>(Vec((RACK_GRID_WIDTH * 1.5) - (25.0/2), 290.5), Port::INPUT, module, DaisyMaster::CHAIN_INPUT));
    }
};

} // namespace rack_plugin_QuantalAudio

using namespace rack_plugin_QuantalAudio;

RACK_PLUGIN_MODEL_INIT(QuantalAudio, DaisyMaster) {
   Model *modelDaisyMaster = Model::create<DaisyMaster, DaisyMasterWidget>("QuantalAudio", "DaisyMaster", "Daisy Mix Master | 3HP", MIXER_TAG, AMPLIFIER_TAG);
   return modelDaisyMaster;
}
