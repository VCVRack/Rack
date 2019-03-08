#include "Squinky.hpp"

#ifdef _GROWLER
#include "WidgetComposite.h"
#include "VocalAnimator.h"
#include "ctrl/SqMenuItem.h"

/**
 * Implementation class for VocalWidget
 */
struct VocalModule : Module
{
    VocalModule();

    /**
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;
    using Animator = VocalAnimator<WidgetComposite>;
    Animator animator;
private:
    typedef float T;
};

VocalModule::VocalModule() : Module(animator.NUM_PARAMS, animator.NUM_INPUTS, animator.NUM_OUTPUTS, animator.NUM_LIGHTS),
animator(this)
{
    onSampleRateChange();
    animator.init();
}

void VocalModule::onSampleRateChange()
{
    T rate = engineGetSampleRate();
    animator.setSampleRate(rate);
}

void VocalModule::step()
{
    animator.step();
}

////////////////////
// module widget
////////////////////

struct VocalWidget : ModuleWidget
{
    VocalWidget(VocalModule *);
    Menu* createContextMenu() override;
};

inline Menu* VocalWidget::createContextMenu()
{
    Menu* theMenu = ModuleWidget::createContextMenu();
    ManualMenuItem* manual = new ManualMenuItem(
        "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/growler.md");
    theMenu->addChild(manual);
    return theMenu;
}

template <typename BASE>
struct MuteLight : BASE
{
    MuteLight()
    {
        this->box.size = mm2px(Vec(6.0f, 6.0f));
    }
};

struct NKK2 : SVGSwitch, ToggleSwitch
{
    NKK2()
    {
        addFrame(SVG::load(assetGlobal("res/ComponentLibrary/NKK_0.svg")));
        addFrame(SVG::load(assetGlobal("res/ComponentLibrary/NKK_2.svg")));
    }
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
#ifdef __V1
VocalWidget::VocalWidget(VocalModule *module)
{
    setModule(module);
#else
VocalWidget::VocalWidget(VocalModule *module) : ModuleWidget(module)
{
#endif
    const float width = 14 * RACK_GRID_WIDTH;
    box.size = Vec(width, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/vocal_animator_panel.svg")));
        addChild(panel);
    }
    /**
     *  LEDs and LFO outputs
     */

    const float lfoBlockY = 38;     // was 22. move down to make space

    const float ledX = width - 46;
    const float ledY = lfoBlockY + 7.5;
    const float ledSpacingY = 30;

    const float lfoOutY = lfoBlockY;
    const float lfoOutX = width - 30;

    const float lfoInputX = 24;
    const float lfoInputY = lfoBlockY + 0;
    const float lfoTrimX = 68;
    const float lfoTrimY = lfoInputY + 3;

    const float lfoRateKnobX = 100;
    const float lfoRateKnobY = lfoBlockY + 24;

    addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
        Vec(ledX, ledY), module, module->animator.LFO0_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
        Vec(ledX, ledY + ledSpacingY), module, module->animator.LFO1_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
        Vec(ledX, ledY + 2 * ledSpacingY), module, module->animator.LFO2_LIGHT));

    addOutput(Port::create<PJ301MPort>(
        Vec(lfoOutX, lfoOutY), Port::OUTPUT, module, VocalModule::Animator::LFO0_OUTPUT));
    addOutput(Port::create<PJ301MPort>(
        Vec(lfoOutX, lfoOutY + 1 * ledSpacingY), Port::OUTPUT, module, VocalModule::Animator::LFO1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(
        Vec(lfoOutX, lfoOutY + 2 * ledSpacingY), Port::OUTPUT, module, VocalModule::Animator::LFO2_OUTPUT));

    addParam(ParamWidget::create<Rogan1PSBlue>(
        Vec(lfoRateKnobX, lfoRateKnobY), module, module->animator.LFO_RATE_PARAM, -5.0, 5.0, 0.0));

    addInput(Port::create<PJ301MPort>(
        Vec(lfoInputX, lfoInputY), Port::INPUT, module, VocalModule::Animator::LFO_RATE_CV_INPUT));
    addParam(ParamWidget::create<Trimpot>(
        Vec(lfoTrimX, lfoTrimY), module, module->animator.LFO_RATE_TRIM_PARAM, -1.0, 1.0, 1.0));

    // the matrix switch
    addParam(ParamWidget::create<NKK>(
        Vec(42, 65), module, module->animator.LFO_MIX_PARAM, 0.0f, 2.0f, 0.0f));

     /**
      * Parameters and CV
      */
    const float mainBlockY = 140;
    const float mainBlockX = 20;

    const float colSpacingX = 64;

    const float knobX = mainBlockX + 0;
    const float knobY = mainBlockY + 24;

    const float trimX = mainBlockX + 11;
    const float trimY = mainBlockY + 78;

    const float inputX = mainBlockX + 8;
    const float inputY = mainBlockY + 108;

    addParam(ParamWidget::create<Rogan1PSBlue>(
        Vec(knobX, knobY), module, module->animator.FILTER_FC_PARAM, -5.0, 5.0, 0.0));
    addInput(Port::create<PJ301MPort>(
        Vec(inputX, inputY), Port::INPUT, module, VocalModule::Animator::FILTER_FC_CV_INPUT));
    addParam(ParamWidget::create<Trimpot>(
        Vec(trimX, trimY), module, module->animator.FILTER_FC_TRIM_PARAM, -1.0, 1.0, 1.0));

    addParam(ParamWidget::create<Rogan1PSBlue>(
        Vec(knobX + colSpacingX, knobY), module, module->animator.FILTER_Q_PARAM, -5.0, 5.0, 0.0));
    addInput(Port::create<PJ301MPort>(
        Vec(inputX + colSpacingX, inputY), Port::INPUT, module, VocalModule::Animator::FILTER_Q_CV_INPUT));
    addParam(ParamWidget::create<Trimpot>(
        Vec(trimX + colSpacingX, trimY), module, module->animator.FILTER_Q_TRIM_PARAM, -1.0, 1.0, 1.0));

    addParam(ParamWidget::create<Rogan1PSBlue>(
        Vec(knobX + 2 * colSpacingX, knobY), module, module->animator.FILTER_MOD_DEPTH_PARAM, -5.0, 5.0, 0.0));
    addInput(Port::create<PJ301MPort>(
        Vec(inputX + 2 * colSpacingX, inputY), Port::INPUT, module, VocalModule::Animator::FILTER_MOD_DEPTH_CV_INPUT));
    addParam(ParamWidget::create<Trimpot>(
        Vec(trimX + 2 * colSpacingX, trimY), module, module->animator.FILTER_MOD_DEPTH_TRIM_PARAM, -1.0, 1.0, 1.0));

    const float row3 = 310;

    // I.O on row 3
    const float AudioInputX = inputX;
    const float outputX = inputX + 2 * colSpacingX;

    addInput(Port::create<PJ301MPort>(
        Vec(AudioInputX, row3), Port::INPUT, module, VocalModule::Animator::AUDIO_INPUT));
    addOutput(Port::create<PJ301MPort>(
        Vec(outputX, row3), Port::OUTPUT, module, VocalModule::Animator::AUDIO_OUTPUT));

    const float bassX = inputX + colSpacingX - 4;
    const float bassY = row3 - 8;

     // the bass boost switch
    addParam(ParamWidget::create<NKK2>(
        Vec(bassX, bassY), module, module->animator.BASS_EXP_PARAM, 0.0f, 1.0f, 0.0f));

     /*************************************************
      *  screws
      */
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

RACK_PLUGIN_MODEL_INIT(squinkylabs_plug1, Vocal) {
   Model *modelVocalModule = Model::create<VocalModule, VocalWidget>("Squinky Labs",
                                                                     "squinkylabs-vocalanimator",
                                                                     "Growler: Vocal Animator", EFFECT_TAG, FILTER_TAG, LFO_TAG, RANDOM_TAG);
   return modelVocalModule;
}

#endif

