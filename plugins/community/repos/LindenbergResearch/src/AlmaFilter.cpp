#include "dsp/LadderFilter.hpp"
#include "LindenbergResearch.hpp"

namespace rack_plugin_LindenbergResearch {

struct AlmaFilter : LRModule {

    enum ParamIds {
        CUTOFF_PARAM,
        RESONANCE_PARAM,
        DRIVE_PARAM,
        SLOPE_PARAM,
        CUTOFF_CV_PARAM,
        RESONANCE_CV_PARAM,
        DRIVE_CV_PARAM,
        NUM_PARAMS
    };

    enum InputIds {
        FILTER_INPUT,
        CUTOFF_CV_INPUT,
        RESONANCE_CV_INPUT,
        DRIVE_CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        LP_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds {
        OVERLOAD_LIGHT,
        NUM_LIGHTS
    };

    LadderFilter filter;

    LRBigKnob *frqKnob = NULL;
    LRMiddleKnob *peakKnob = NULL;
    LRMiddleKnob *driveKnob = NULL;


    AlmaFilter() : LRModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}


    void step() override;
};


void AlmaFilter::step() {
    float frqcv = inputs[CUTOFF_CV_INPUT].value * 0.1f * quadraticBipolar(params[CUTOFF_CV_PARAM].value);
    float rescv = inputs[RESONANCE_CV_INPUT].value * 0.1f * quadraticBipolar(params[RESONANCE_CV_PARAM].value);
    float drvcv = inputs[DRIVE_CV_INPUT].value * 0.1f * quadraticBipolar(params[DRIVE_CV_PARAM].value);

    filter.setFrequency(params[CUTOFF_PARAM].value + frqcv);
    filter.setResonance(params[RESONANCE_PARAM].value + rescv);
    filter.setDrive(params[DRIVE_PARAM].value + drvcv);
    filter.setSlope(params[SLOPE_PARAM].value);


    /* pass modulated parameter to knob widget for cv indicator */
    if (frqKnob != NULL && peakKnob != NULL && driveKnob != NULL) {
        frqKnob->setIndicatorActive(inputs[CUTOFF_CV_INPUT].active);
        peakKnob->setIndicatorActive(inputs[RESONANCE_CV_INPUT].active);
        driveKnob->setIndicatorActive(inputs[DRIVE_CV_INPUT].active);

        frqKnob->setIndicatorValue(params[CUTOFF_PARAM].value + frqcv);
        peakKnob->setIndicatorValue(params[RESONANCE_PARAM].value + rescv);
        driveKnob->setIndicatorValue(params[DRIVE_PARAM].value + drvcv);
    }


    float y = inputs[FILTER_INPUT].value;

    filter.setIn(y);
    filter.process();

    outputs[LP_OUTPUT].value = filter.getLpOut();


    lights[OVERLOAD_LIGHT].value = filter.getLightValue();
}


/**
 * @brief ALMA filter
 */
struct AlmaFilterWidget : LRModuleWidget {
    AlmaFilterWidget(AlmaFilter *module);
};


AlmaFilterWidget::AlmaFilterWidget(AlmaFilter *module) : LRModuleWidget(module) {
    //setPanel(SVG::load(assetPlugin(plugin, "res/VCF.svg")));


    panel = new LRPanel();
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/VCF.svg")));
    addChild(panel);

    box.size = panel->box.size;

    // ***** SCREWS **********
    addChild(Widget::create<ScrewDarkA>(Vec(15, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(15, 366)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 366)));
    // ***** SCREWS **********

    // ***** MAIN KNOBS ******
    module->frqKnob = LRKnob::create<LRBigKnob>(Vec(62, 150), module, AlmaFilter::CUTOFF_PARAM, 0.f, 1.f, 0.8f);
    module->peakKnob = LRKnob::create<LRMiddleKnob>(Vec(24, 229), module, AlmaFilter::RESONANCE_PARAM, -0.f, 1.5, 0.0f);
    module->driveKnob = LRKnob::create<LRMiddleKnob>(Vec(116, 228), module, AlmaFilter::DRIVE_PARAM, 0.0f, 1.f, 0.0f);

    addParam(module->frqKnob);
    addParam(module->peakKnob);
    addParam(module->driveKnob);

    addParam(ParamWidget::create<LRMiddleKnob>(Vec(70, 288), module, AlmaFilter::SLOPE_PARAM, 0.0f, 4.f, 2.0f));
    // ***** MAIN KNOBS ******

    // ***** CV INPUTS *******
    addParam(ParamWidget::create<LRSmallKnob>(Vec(27.5, 106), module, AlmaFilter::RESONANCE_CV_PARAM, -1.f, 1.0f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(78, 106), module, AlmaFilter::CUTOFF_CV_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(127.1, 106), module, AlmaFilter::DRIVE_CV_PARAM, -1.f, 1.f, 0.f));

    addInput(Port::create<IOPort>(Vec(26, 50), Port::INPUT, module, AlmaFilter::RESONANCE_CV_INPUT));
    addInput(Port::create<IOPort>(Vec(76, 50), Port::INPUT, module, AlmaFilter::CUTOFF_CV_INPUT));
    addInput(Port::create<IOPort>(Vec(125, 50), Port::INPUT, module, AlmaFilter::DRIVE_CV_INPUT));
    // ***** CV INPUTS *******

    // ***** INPUTS **********
    addInput(Port::create<IOPort>(Vec(25, 326.5), Port::INPUT, module, AlmaFilter::FILTER_INPUT));
    // ***** INPUTS **********

    // ***** OUTPUTS *********
    addOutput(Port::create<IOPort>(Vec(124.5, 326.5), Port::OUTPUT, module, AlmaFilter::LP_OUTPUT));
    // ***** OUTPUTS *********

    // ***** LIGHTS **********
    addChild(ModuleLightWidget::create<LRRedLight>(Vec(85, 247), module, AlmaFilter::OVERLOAD_LIGHT));
    // ***** LIGHTS **********
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, AlmaFilter) {
   Model *modelAlmaFilter = Model::create<AlmaFilter, AlmaFilterWidget>("Lindenberg Research", "VCF", "Alma Ladder Filter", FILTER_TAG);
   return modelAlmaFilter;
}
