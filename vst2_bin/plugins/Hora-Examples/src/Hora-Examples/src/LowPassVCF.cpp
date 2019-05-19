#include "Examples.hpp"
#include "MMFilter.h" 

struct LowPassVCF : Module {
    MMFilter MMFilter_mul23;

    enum ParamIds {
           FREQ_PARAM,
           RESO_PARAM,
           AUDIO_LEVEL_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
          FREQ_INPUT,
          RESO_INPUT,
          AUDIO_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
          AUDIO_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS 
    };
    LowPassVCF();
    void step()override;
};

LowPassVCF::LowPassVCF() {
    params.resize(NUM_PARAMS);
    inputs.resize(NUM_INPUTS);
    outputs.resize(NUM_OUTPUTS);
}
void LowPassVCF::step() {
    int int24_out = 0;
    float FREQ_PARAM_output = params[FREQ_PARAM].value;
    float RESO_PARAM_output = params[RESO_PARAM].value;
    float FREQ_INPUT_signal = inputs[FREQ_INPUT].value * (1.0f / 10.0f);
    float RESO_INPUT_signal = inputs[RESO_INPUT].value * (1.0f / 10.0f);
    float AUDIO_INPUT_signal = inputs[AUDIO_INPUT].value;
    float AUDIO_LEVEL_PARAM_output = params[AUDIO_LEVEL_PARAM].value;
    float dou12_out = 5;
    float dou14_out = 5;
    float dou19_out = 5;
    float x__Op20_result = RESO_INPUT_signal/dou19_out;
    float dou22_out = 5;
    float x__Op8_result = AUDIO_LEVEL_PARAM_output*AUDIO_INPUT_signal;
    float x__Op11_result = x__Op8_result/dou12_out;
    float x__Op21_result = FREQ_INPUT_signal/dou22_out;
    MMFilter_mul23.setinput(x__Op11_result);
    MMFilter_mul23.setfreq(x__Op21_result + FREQ_PARAM_output);
    MMFilter_mul23.setreso(RESO_PARAM_output + x__Op20_result);
    MMFilter_mul23.setmode(int24_out);
    float mul23_output = MMFilter_mul23.getoutput();

    float x__Op13_result = mul23_output*dou14_out;
    outputs[AUDIO_OUTPUT].value = x__Op13_result;

}
struct LowPassVCFWidget : ModuleWidget {
   LowPassVCFWidget(LowPassVCF *module);
};

LowPassVCFWidget::LowPassVCFWidget(LowPassVCF *module) : ModuleWidget(module) {
   box.size = Vec(120, 380);
   {
       SVGPanel *panel = new SVGPanel();
       panel->box.size = box.size;
       panel->setBackground(SVG::load(assetPlugin(plugin, "res/LPVCF.svg")));
       addChild(panel);
    }
    addParam(ParamWidget::create<mediumKnob>(Vec(69.6985,188.29), module, LowPassVCF::FREQ_PARAM, 0, 1, 0));
    addParam(ParamWidget::create<mediumKnob>(Vec(9.46014,188.131), module, LowPassVCF::RESO_PARAM, 0, 1, 0));
    addInput(Port::create<jack>(Vec(71.9289,135.616), Port::INPUT, module, LowPassVCF::FREQ_INPUT));
    addInput(Port::create<jack>(Vec(12.6396,135.853), Port::INPUT, module, LowPassVCF::RESO_INPUT));
    addInput(Port::create<jack>(Vec(14.7729,310.231), Port::INPUT, module, LowPassVCF::AUDIO_INPUT));
    addParam(ParamWidget::create<mediumKnob>(Vec(40.9485,42.9485), module, LowPassVCF::AUDIO_LEVEL_PARAM, 0, 1, 0.99));
    addOutput(Port::create<jack>(Vec(72.1264,309.651), Port::OUTPUT, module, LowPassVCF::AUDIO_OUTPUT));
}

RACK_PLUGIN_MODEL_INIT(Hora_Examples, LowPassVCF) {
   Model *modelLowPassVCF = Model::create<LowPassVCF,LowPassVCFWidget>("Hora", "Hora-ExamplesVCF", "LPfilter", FILTER_TAG);
   return modelLowPassVCF;
}
