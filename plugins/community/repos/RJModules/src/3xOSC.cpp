#include "dsp/digital.hpp"
#include <iostream>
#include "RJModules.hpp"

namespace rack_plugin_RJModules {

struct Osc {
    float phase = 0.0;
    float pw = 0.5;
    float freq = 1.0;
    bool offset = false;
    bool invert = false;

    SchmittTrigger resetTrigger;
    Osc() {}

    void setPitch(float pitch) {
        pitch = fminf(pitch, 8.0);
        freq = powf(2.0, pitch);
    }

    void setFreq(float freqi){
        freq = freqi;
    }

    float getFreq() {
        return freq;
    }
    void setReset(float reset) {
        if (resetTrigger.process(reset)) {
            phase = 0.0;
        }
    }
    void step(float dt) {
        float deltaPhase = fminf(freq * dt, 0.5);
        phase += deltaPhase;
        if (phase >= 1.0)
            phase -= 1.0;
    }
    float saw(float x) {
        return 2.0 * (x - roundf(x));
    }
    float saw() {
        if (offset)
            return invert ? 2.0 * (1.0 - phase) : 2.0 * phase;
        else
            return saw(phase) * (invert ? -1.0 : 1.0);
    }
    float light() {
        return sinf(2*M_PI * phase);
    }
};

struct ThreeXOSC : Module {
    enum ParamIds {

        SHAPE_PARAM_1,
        ATTACK_PARAM_1,
        DECAY_PARAM_1,
        MIX_PARAM_1,

        SHAPE_PARAM_2,
        ATTACK_PARAM_2,
        DECAY_PARAM_2,
        MIX_PARAM_2,

        SHAPE_PARAM_3,
        ATTACK_PARAM_3,
        DECAY_PARAM_3,
        MIX_PARAM_3,

        NUM_PARAMS
    };
    enum InputIds {

        SHAPE_CV1_1,
        ATTACK_CV1_1,
        DECAY_CV1_1,
        MIX_CV1_1,
        VOCT_1,

        SHAPE_CV1_2,
        ATTACK_CV1_2,
        DECAY_CV1_2,
        MIX_CV1_2,
        VOCT_2,

        SHAPE_CV1_3,
        ATTACK_CV1_3,
        DECAY_CV1_3,
        MIX_CV1_3,
        VOCT_3,

        NUM_INPUTS
    };
    enum OutputIds {
        OUTPUT_1,
        OUTPUT_2,
        OUTPUT_3,
        OUTPUT_ALL,
        NUM_OUTPUTS
    };
    enum LightIds {
        PHASE_POS_LIGHT_1,
        PHASE_POS_LIGHT_2,
        PHASE_POS_LIGHT_3,

        NUM_LIGHTS
    };

    // Pitchies
    float referenceFrequency = 261.626; // C4; frequency at which Rack 1v/octave CVs are zero.
    float referenceSemitone = 60.0; // C4; value of C4 in semitones is arbitrary here, so have it match midi note numbers when rounded to integer.
    float twelfthRootTwo = 1.0594630943592953;
    float logTwelfthRootTwo = logf(1.0594630943592953);
    int referencePitch = 0;
    int referenceOctave = 4;

    float frequencyToSemitone(float frequency) {
        return logf(frequency / referenceFrequency) / logTwelfthRootTwo + referenceSemitone;
    }

    float semitoneToFrequency(float semitone) {
        return powf(twelfthRootTwo, semitone - referenceSemitone) * referenceFrequency;
    }

    float frequencyToCV(float frequency) {
        return log2f(frequency / referenceFrequency);
    }

    float cvToFrequency(float cv) {
        return powf(2.0, cv) * referenceFrequency;
    }

    float cvToSemitone(float cv) {
        return frequencyToSemitone(cvToFrequency(cv));
    }

    float semitoneToCV(float semitone) {
        return frequencyToCV(semitoneToFrequency(semitone));
    }

    Osc osc1;
    Osc osc2;
    Osc osc3;

    float last_1 = 0.0;
    bool gated1 = false;

    float DETUNE_STEP = .075;

    bool decaying1 = false;
    float env1 = 0.0f;
    SchmittTrigger trigger1;

    ThreeXOSC() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void ThreeXOSC::step() {

    float attack1 = clamp(params[ATTACK_PARAM_1].value + inputs[ATTACK_CV1_1].value / 10.0f, 0.0f, 1.0f);
    float decay1 = 0.0f;
    float sustain1 = 1.0f;
    float release1 = clamp(params[DECAY_PARAM_1].value + inputs[DECAY_CV1_1].value / 10.0f, 0.0f, 1.0f);

    float osc1_pitch = inputs[VOCT_1].value;
    osc1.setFreq(cvToFrequency(osc1_pitch));
    osc1.step(1.0 / engineGetSampleRate());

    // Gate
    if (last_1 == inputs[VOCT_1].value){
        gated1 = false;
    } else{
        gated1 = true;
        last_1 = inputs[VOCT_1].value;
    }

    // ADSR
    const float base = 20000.0f;
    const float maxTime = 10.0f;
    if (gated1) {
        if (decaying1) {
            // Decay
            if (decay1 < 1e-4) {
                env1 = sustain1;
            }
            else {
                env1 += powf(base, 1 - decay1) / maxTime * (sustain1 - env1) * engineGetSampleTime();
            }
        }
        else {
            // Attack
            // Skip ahead if attack is all the way down (infinitely fast)
            if (attack1 < 1e-4) {
                env1 = 1.0f;
            }
            else {
                env1 += powf(base, 1 - attack1) / maxTime * (1.01f - env1) * engineGetSampleTime();
            }
            if (env1 >= 1.0f) {
                env1 = 1.0f;
                decaying1 = true;
            }
        }
    }
    else {
        // Release
        if (release1 < 1e-4) {
            env1 = 0.0f;
        }
        else {
            env1 += powf(base, 1 - release1) / maxTime * (0.0f - env1) * engineGetSampleTime();
        }
        decaying1 = false;
    }

    float env_out1 = 10.0f * env1;

    // Osci
    float osc_out1 = osc1.saw();

    printf("env_out1 %f \n", env_out1);

    // VCA
    float cv = 1.f;
    cv = fmaxf(env_out1 / 10.f, 0.f);
    cv = powf(cv, 4.f);

    printf("cv %f \n", cv);
    printf("output %f\n", osc_out1 * cv);
    //lastCv = cv;
    outputs[OUTPUT_1].value = osc_out1 - cv;
    //outputs[OUTPUT_1].value = osc1.saw() * env_out1;

    // float root_pitch = params[FREQ_PARAM].value * clamp(inputs[FREQ_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
    // oscillator.setPitch(root_pitch);
    // oscillator.offset = (params[OFFSET_PARAM].value > 0.0);
    // oscillator.invert = (params[INVERT_PARAM].value <= 0.0);
    // oscillator.step(1.0 / engineGetSampleRate());
    // oscillator.setReset(inputs[RESET_INPUT].value);

    // oscillator2.setPitch(root_pitch + (params[DETUNE_PARAM].value * DETUNE_STEP * clamp(inputs[DETUNE_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f)));
    // oscillator2.offset = (params[OFFSET_PARAM].value > 0.0);
    // oscillator2.invert = (params[INVERT_PARAM].value <= 0.0);
    // oscillator2.step(1.0 / engineGetSampleRate());
    // oscillator2.setReset(inputs[RESET_INPUT].value);

    // oscillator3.setPitch(root_pitch - (params[DETUNE_PARAM].value * DETUNE_STEP * clamp(inputs[DETUNE_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f)));
    // oscillator3.offset = (params[OFFSET_PARAM].value > 0.0);
    // oscillator3.invert = (params[INVERT_PARAM].value <= 0.0);
    // oscillator3.step(1.0 / engineGetSampleRate());
    // oscillator3.setReset(inputs[RESET_INPUT].value);

    // float osc3_saw = oscillator3.saw();
    // if (params[OFFSET_PARAM].value < 1){
    //     osc3_saw = 0;
    // } else{
    //     osc3_saw = oscillator3.saw();
    // }

    // float mix_percent = params[MIX_PARAM].value * clamp(inputs[MIX_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
    // outputs[SAW_OUTPUT].value = 5.0 * (( oscillator.saw() + (oscillator2.saw() * mix_percent) + (osc3_saw * mix_percent) / 3));

    // lights[PHASE_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0, oscillator.light()));
    // lights[PHASE_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0, -oscillator.light()));
}

struct ThreeXOSCWidget: ModuleWidget {
    ThreeXOSCWidget(ThreeXOSC *module);
};

ThreeXOSCWidget::ThreeXOSCWidget(ThreeXOSC *module) : ModuleWidget(module) {
     box.size = Vec(240, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/ThreeXOSC.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    int row_base = 30; 
    addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(25, row_base), module, ThreeXOSC::SHAPE_PARAM_1, 0.0, 8.0, 5.0));
    addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(75, row_base), module, ThreeXOSC::ATTACK_PARAM_1, 0.0, 1.0, 0.1));
    addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(125, row_base), module, ThreeXOSC::DECAY_PARAM_1, 0.0, 1.0, 1.0));
    addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(175, row_base), module, ThreeXOSC::MIX_PARAM_1, 0.0, 1.0, 1.0));
    addInput(Port::create<PJ301MPort>(Vec(30, row_base + 45), Port::INPUT, module, ThreeXOSC::SHAPE_CV1_1));
    addInput(Port::create<PJ301MPort>(Vec(80, row_base + 45), Port::INPUT, module, ThreeXOSC::ATTACK_CV1_1));
    addInput(Port::create<PJ301MPort>(Vec(130, row_base + 45), Port::INPUT, module, ThreeXOSC::DECAY_CV1_1));
    addInput(Port::create<PJ301MPort>(Vec(180, row_base + 45), Port::INPUT, module, ThreeXOSC::MIX_CV1_1));
    addInput(Port::create<PJ301MPort>(Vec(30, row_base + 75), Port::INPUT, module, ThreeXOSC::VOCT_1));
    addOutput(Port::create<PJ301MPort>(Vec(180, row_base + 75), Port::OUTPUT, module, ThreeXOSC::OUTPUT_1));
    addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(210, row_base + 80), module, ThreeXOSC::PHASE_POS_LIGHT_1));

    row_base = 140; 
    addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(25, row_base), module, ThreeXOSC::SHAPE_PARAM_2, 0.0, 8.0, 5.0));
    addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(75, row_base), module, ThreeXOSC::ATTACK_PARAM_2, 0.0, 1.0, 0.1));
    addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(125, row_base), module, ThreeXOSC::DECAY_PARAM_2, 0.0, 1.0, 1.0));
    addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(175, row_base), module, ThreeXOSC::MIX_PARAM_2, 0.0, 1.0, 1.0));
    addInput(Port::create<PJ301MPort>(Vec(30, row_base + 45), Port::INPUT, module, ThreeXOSC::SHAPE_CV1_2));
    addInput(Port::create<PJ301MPort>(Vec(80, row_base + 45), Port::INPUT, module, ThreeXOSC::ATTACK_CV1_2));
    addInput(Port::create<PJ301MPort>(Vec(130, row_base + 45), Port::INPUT, module, ThreeXOSC::DECAY_CV1_2));
    addInput(Port::create<PJ301MPort>(Vec(180, row_base + 45), Port::INPUT, module, ThreeXOSC::MIX_CV1_2));
    addInput(Port::create<PJ301MPort>(Vec(30, row_base + 75), Port::INPUT, module, ThreeXOSC::VOCT_2));
    addOutput(Port::create<PJ301MPort>(Vec(180, row_base + 75), Port::OUTPUT, module, ThreeXOSC::OUTPUT_2));
    addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(210, row_base + 80), module, ThreeXOSC::PHASE_POS_LIGHT_2));

    row_base = 250; 
    addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(25, row_base), module, ThreeXOSC::SHAPE_PARAM_3, 0.0, 8.0, 5.0));
    addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(75, row_base), module, ThreeXOSC::ATTACK_PARAM_3, 0.0, 1.0, 0.1));
    addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(125, row_base), module, ThreeXOSC::DECAY_PARAM_3, 0.0, 1.0, 1.0));
    addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(175, row_base), module, ThreeXOSC::MIX_PARAM_3, 0.0, 1.0, 1.0));
    addInput(Port::create<PJ301MPort>(Vec(30, row_base + 45), Port::INPUT, module, ThreeXOSC::SHAPE_CV1_3));
    addInput(Port::create<PJ301MPort>(Vec(80, row_base + 45), Port::INPUT, module, ThreeXOSC::ATTACK_CV1_3));
    addInput(Port::create<PJ301MPort>(Vec(130, row_base + 45), Port::INPUT, module, ThreeXOSC::DECAY_CV1_3));
    addInput(Port::create<PJ301MPort>(Vec(180, row_base + 45), Port::INPUT, module, ThreeXOSC::MIX_CV1_3));
    addInput(Port::create<PJ301MPort>(Vec(30, row_base + 75), Port::INPUT, module, ThreeXOSC::VOCT_3));
    addOutput(Port::create<PJ301MPort>(Vec(180, row_base + 75), Port::OUTPUT, module, ThreeXOSC::OUTPUT_3));
    // addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(210, row_base + 80), module, ThreeXOSC::PHASE_POS_LIGHT_3));

    // addInput(Port::create<PJ301MPort>(Vec(22, 100), Port::INPUT, module, ThreeXOSC::FREQ_CV_INPUT));
    // addInput(Port::create<PJ301MPort>(Vec(22, 190), Port::INPUT, module, ThreeXOSC::DETUNE_CV_INPUT));
    // addInput(Port::create<PJ301MPort>(Vec(22, 270), Port::INPUT, module, ThreeXOSC::MIX_CV_INPUT));
    // addInput(Port::create<PJ301MPort>(Vec(38, 310), Port::INPUT, module, ThreeXOSC::RESET_INPUT));

    // addOutput(Port::create<PJ301MPort>(Vec(100, 310), Port::OUTPUT, module, ThreeXOSC::SAW_OUTPUT));


}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, ThreeXOSC) {
   Model *modelThreeXOSC = Model::create<ThreeXOSC, ThreeXOSCWidget>("RJModules", "ThreeXOSC", "[GEN] 3xOSC", LFO_TAG);
   return modelThreeXOSC;
}
