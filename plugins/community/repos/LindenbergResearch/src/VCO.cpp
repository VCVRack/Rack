#include "dsp/Oscillator.hpp"
#include "LindenbergResearch.hpp"

namespace rack_plugin_LindenbergResearch {

struct VCO : LRModule {
    enum ParamIds {
        FREQUENCY_PARAM,
        OCTAVE_PARAM,
        FM_CV_PARAM,
        SHAPE_CV_PARAM,
        PW_CV_PARAM,
        SHAPE_PARAM,
        PW_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        VOCT_INPUT,
        FM_CV_INPUT,
        PW_CV_INPUT,
        SHAPE_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        SAW_OUTPUT,
        PULSE_OUTPUT,
        SINE_OUTPUT,
        TRI_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    dsp::BLITOscillator *osc = new dsp::BLITOscillator();
    LCDWidget *label1 = new LCDWidget(COLOR_CYAN, 6);

    VCO() : LRModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}


    void step() override;
};


void VCO::step() {
    LRModule::step();

    float fm = clamp(inputs[FM_CV_INPUT].value, -10.f, 10.f) * 400.f * quadraticBipolar(params[FM_CV_PARAM].value);

    osc->updatePitch(inputs[VOCT_INPUT].value, clamp(fm, -10000.f, 10000.f), params[FREQUENCY_PARAM].value, params[OCTAVE_PARAM].value);

    float shape = quadraticBipolar(params[SHAPE_PARAM].value);
    float pw = params[PW_CV_PARAM].value;

    if (osc->shape != shape) {
        osc->setShape(shape);
    }

    if (osc->pw != pw) {
        osc->setPulseWidth(pw);
    }

    osc->proccess();

    outputs[SAW_OUTPUT].value = osc->saw;

    outputs[PULSE_OUTPUT].value = osc->pulse;
    outputs[SINE_OUTPUT].value = osc->sine;

    outputs[TRI_OUTPUT].value = osc->tri;

    if (cnt % 1200 == 0) {
        label1->text = stringf("%.2f Hz", osc->getFrequency());
    }
}


/**
 * @brief Woldemar VCO
 */
struct VCOWidget : LRModuleWidget {
    VCOWidget(VCO *module);
};


VCOWidget::VCOWidget(VCO *module) : LRModuleWidget(module) {
  //  setPanel(SVG::load(assetPlugin(plugin, "res/VCO.svg")));

    panel = new LRPanel(20,40);
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/VCO.svg")));
    addChild(panel);

    box.size = panel->box.size;

    // ***** SCREWS **********
    addChild(Widget::create<ScrewDarkA>(Vec(15, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(15, 366)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 366)));
    // ***** SCREWS **********


    // ***** MAIN KNOBS ******
    addParam(ParamWidget::create<LRMiddleKnob>(Vec(83, 172.0), module, VCO::FREQUENCY_PARAM, -15.f, 15.f, 0.f));
    addParam(ParamWidget::create<LRToggleKnob>(Vec(85, 240), module, VCO::OCTAVE_PARAM, -3.f, 3.f, 0.f));

    addParam(ParamWidget::create<LRSmallKnob>(Vec(118, 111.5), module, VCO::PW_PARAM, -.1f, 1.f, 1.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(65, 60), module, VCO::SHAPE_CV_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(15, 267), module, VCO::FM_CV_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(65, 111.5), module, VCO::PW_CV_PARAM, 0.02f, 1.f, 1.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(118, 59), module, VCO::SHAPE_PARAM, 1.f, 5.f, 1.f));


    // ***** MAIN KNOBS ******


    // ***** INPUTS **********
    addInput(Port::create<IOPort>(Vec(15, 182), Port::INPUT, module, VCO::VOCT_INPUT));
    addInput(Port::create<IOPort>(Vec(15, 228), Port::INPUT, module, VCO::FM_CV_INPUT));
    addInput(Port::create<IOPort>(Vec(15, 112), Port::INPUT, module, VCO::PW_CV_INPUT));
    addInput(Port::create<IOPort>(Vec(15, 60), Port::INPUT, module, VCO::SHAPE_CV_INPUT));

    //  addInput(createInput<IOPort>(Vec(71, 60), module, VCO::RESHAPER_CV_INPUT));
    // ***** INPUTS **********

    // ***** OUTPUTS *********
    // addOutput(createOutput<IOPort>(Vec(20, 320), module, VCO::SAW_OUTPUT));
    addOutput(Port::create<IOPort>(Vec(20.8, 304.5), Port::OUTPUT, module, VCO::SAW_OUTPUT));
    addOutput(Port::create<IOPort>(Vec(57.2, 304.5), Port::OUTPUT, module, VCO::PULSE_OUTPUT));
    addOutput(Port::create<IOPort>(Vec(96.1, 304.5), Port::OUTPUT, module, VCO::SINE_OUTPUT));
    addOutput(Port::create<IOPort>(Vec(132, 304.5), Port::OUTPUT, module, VCO::TRI_OUTPUT));
    // ***** OUTPUTS *********

    module->label1->box.pos = Vec(30,110);

    addChild(module->label1);
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, VCO) {
   Model *modelVCO = Model::create<VCO, VCOWidget>("Lindenberg Research", "VCO", "Voltage Controlled Oscillator", OSCILLATOR_TAG);
   return modelVCO;
}
