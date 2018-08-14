#include "dsp/MS20zdf.hpp"
#include "LindenbergResearch.hpp"

namespace rack_plugin_LindenbergResearch {

struct MS20Filter : LRModule {

    enum ParamIds {
        FREQUENCY_PARAM,
        PEAK_PARAM,
        DRIVE_PARAM,
        CUTOFF_CV_PARAM,
        PEAK_CV_PARAM,
        GAIN_CV_PARAM,
        MODE_SWITCH_PARAM,
        NUM_PARAMS
    };

    enum InputIds {
        FILTER_INPUT,
        CUTOFF_CV_INPUT,
        PEAK_CV_INPUT,
        GAIN_CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        FILTER_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds {
        NUM_LIGHTS
    };

    dsp::MS20zdf *ms20zdf = new dsp::MS20zdf(engineGetSampleRate());

    LRBigKnob *frqKnob = NULL;
    LRMiddleKnob *peakKnob = NULL;
    LRMiddleKnob *driveKnob = NULL;


    MS20Filter() : LRModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}


    void step() override;
    void onSampleRateChange() override;
};


void MS20Filter::step() {
    /* compute control voltages */
    float frqcv = inputs[CUTOFF_CV_INPUT].value * 0.1f * quadraticBipolar(params[CUTOFF_CV_PARAM].value);
    float peakcv = inputs[PEAK_CV_INPUT].value * 0.1f * quadraticBipolar(params[PEAK_CV_PARAM].value);
    float gaincv = inputs[GAIN_CV_INPUT].value * 0.1f * quadraticBipolar(params[GAIN_CV_PARAM].value);

    /* set cv modulated parameters */
    ms20zdf->setFrequency(params[FREQUENCY_PARAM].value + frqcv);
    ms20zdf->setPeak(params[PEAK_PARAM].value + peakcv);
    ms20zdf->setDrive(params[DRIVE_PARAM].value + gaincv);

    /* pass modulated parameter to knob widget for cv indicator */
    if (frqKnob != NULL && peakKnob != NULL && driveKnob != NULL) {
        frqKnob->setIndicatorActive(inputs[CUTOFF_CV_INPUT].active);
        peakKnob->setIndicatorActive(inputs[PEAK_CV_INPUT].active);
        driveKnob->setIndicatorActive(inputs[GAIN_CV_INPUT].active);

        frqKnob->setIndicatorValue(params[FREQUENCY_PARAM].value + frqcv);
        peakKnob->setIndicatorValue(params[PEAK_PARAM].value + peakcv);
        driveKnob->setIndicatorValue(params[DRIVE_PARAM].value + gaincv);
    }

    /* process signal */
    ms20zdf->setType(params[MODE_SWITCH_PARAM].value);
    ms20zdf->setIn(inputs[FILTER_INPUT].value);
    ms20zdf->process();

    outputs[FILTER_OUTPUT].value = ms20zdf->getLPOut();
}


void MS20Filter::onSampleRateChange() {
    Module::onSampleRateChange();
    ms20zdf->updateSampleRate(engineGetSampleRate());
}


/**
 * @brief Valerie MS20 filter
 */
struct MS20FilterWidget : LRModuleWidget {
    MS20FilterWidget(MS20Filter *module);
};


MS20FilterWidget::MS20FilterWidget(MS20Filter *module) : LRModuleWidget(module) {
    //setPanel(SVG::load(assetPlugin(plugin, "res/MS20.svg")));

    panel = new LRPanel();
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/MS20.svg")));
    addChild(panel);

    box.size = panel->box.size;

    // ***** SCREWS **********
    addChild(Widget::create<ScrewDarkA>(Vec(15, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(15, 366)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 366)));
    // ***** SCREWS **********

    // ***** MAIN KNOBS ******
    module->frqKnob = LRKnob::create<LRBigKnob>(Vec(102.f, 64.9f), module, MS20Filter::FREQUENCY_PARAM, 0.f, 1.f, 1.f);
    module->peakKnob = LRKnob::create<LRMiddleKnob>(Vec(110.f, 160.8f), module, MS20Filter::PEAK_PARAM, 0.0f, 1.0f, 0.0f);
    module->driveKnob = LRKnob::create<LRMiddleKnob>(Vec(110.f, 230.6f), module, MS20Filter::DRIVE_PARAM, 0.f, 1.0f, 0.0f);

    addParam(module->frqKnob);
    addParam(module->peakKnob);
    addParam(module->driveKnob);

    // ***** MAIN KNOBS ******

    // ***** CV INPUTS *******
    addParam(ParamWidget::create<LRSmallKnob>(Vec(61.f, 169.3f), module, MS20Filter::PEAK_CV_PARAM, -1.f, 1.0f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(61.f, 82.4f), module, MS20Filter::CUTOFF_CV_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(61.f, 239.f), module, MS20Filter::GAIN_CV_PARAM, -1.f, 1.f, 0.f));

    addInput(Port::create<IOPort>(Vec(18, 168.5), Port::INPUT, module, MS20Filter::PEAK_CV_INPUT));
    addInput(Port::create<IOPort>(Vec(18, 81.5), Port::INPUT, module, MS20Filter::CUTOFF_CV_INPUT));
    addInput(Port::create<IOPort>(Vec(18, 239), Port::INPUT, module, MS20Filter::GAIN_CV_INPUT));
    // ***** CV INPUTS *******

    // ***** INPUTS **********
    addInput(Port::create<IOPort>(Vec(17.999f, 326.05f), Port::INPUT, module, MS20Filter::FILTER_INPUT));
    // ***** INPUTS **********

    // ***** OUTPUTS *********
    addOutput(Port::create<IOPort>(Vec(58.544f, 326.05f), Port::OUTPUT, module, MS20Filter::FILTER_OUTPUT));
    // ***** OUTPUTS *********

    // ***** SWITCH  *********
    addParam(ParamWidget::create<LRSwitch>(Vec(119.f, 331.f), module, MS20Filter::MODE_SWITCH_PARAM, 0.0f, 1.0f, 1.0f));
    // ***** SWITCH  *********
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, MS20Filter) {
   Model *modelMS20Filter = Model::create<MS20Filter, MS20FilterWidget>("Lindenberg Research", "MS20 VCF", "Valerie MS20 Filter", FILTER_TAG);
   return modelMS20Filter;
}
