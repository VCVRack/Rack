#include "RJModules.hpp"

#include "dsp/digital.hpp"
#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace rack_plugin_RJModules {

/*
Thank you to MSCHacks for the starting point on this one!
Thanks to Strum for the display widget!
*/

struct BPM: Module {
    enum ParamIds {
        BPM_PARAM,
        RESET_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        CH1_CV_INPUT,
        RESET_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        CH1_OUTPUT,
        CH2_OUTPUT,
        CH3_OUTPUT,
        CH4_OUTPUT,
        CH5_OUTPUT,
        CH6_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        RESET_LIGHT,
        PULSE_LIGHT,
        NUM_LIGHTS
    };

    float resetLight = 0.0;
    float pulseLight = 0.0;
    int m_fBPM = 133.0;
    float m_fBeatsPers;
    float m_fMainClockCount;

    BPM() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
    void    BPMChange( float fbmp, bool bforce );
};

template <typename BASE>
struct BigOlLight : BASE {
        BigOlLight() {
                this->box.size = mm2px(Vec(34, 34));
        }
};

struct NumberDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  NumberDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  };

  void draw(NVGcontext *vg) override
  {
    // Background
    NVGcolor backgroundColor = nvgRGB(0xC0, 0xC0, 0xC0);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);

    // text
    nvgFontSize(vg, 32);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);

    std::stringstream to_display;
    to_display << std::setw(3) << (int) *value;

    Vec textPos = Vec(16.0f, 33.0f);
    NVGcolor textColor = nvgRGB(0x00, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};

void BPM::BPMChange( float fbpm, bool bforce ){

    // // don't change if it is already the same
    if( !bforce && ( (int)(fbpm * 1000.0f ) == (int)(m_fBPM * 1000.0f ) ) )
        return;

    m_fBPM = fbpm;
    m_fBeatsPers = fbpm / 60.0;
}

void BPM::step() {

    const float lightLambda = 0.075;
    float output = 0.0;
    SchmittTrigger resetTrigger;

    bool bMainClockTrig = false;

    // new_value = ( (old_value - old_min) / (old_max - old_min) ) * (new_max - new_min) + new_min
    float bpm_val = params[BPM_PARAM].value * clamp(inputs[CH1_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
    float mapped_bpm = ((bpm_val - 0.0) / (1.0 - 0.0) ) * (600.0 - 40.0) + 40.0;

    m_fBPM = mapped_bpm;
    m_fMainClockCount += (mapped_bpm/60.0);

    if( ( m_fMainClockCount ) >= engineGetSampleRate() )
    {
        m_fMainClockCount = m_fMainClockCount - engineGetSampleRate();
        bMainClockTrig = true;
    }

    if( bMainClockTrig )
    {
        output = 12.0;
        resetLight = 1.0;
        pulseLight = 1.0;
    }

    // Reset
    if (params[RESET_PARAM].value > 0 || inputs[RESET_CV_INPUT].value > 0) {
        resetLight = 1.0;
        output = 12.0;
        m_fMainClockCount = 0;
    }

    pulseLight -= pulseLight / lightLambda / engineGetSampleRate();

    outputs[CH1_OUTPUT].value = output;
    outputs[CH2_OUTPUT].value = output;
    outputs[CH3_OUTPUT].value = output;
    outputs[CH4_OUTPUT].value = output;
    outputs[CH5_OUTPUT].value = output;
    outputs[CH6_OUTPUT].value = output;
    lights[PULSE_LIGHT].value = pulseLight;

}

struct BPMWidget: ModuleWidget {
    BPMWidget(BPM *module);
};

BPMWidget::BPMWidget(BPM *module) : ModuleWidget(module) {
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/BPM.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    addInput(Port::create<PJ301MPort>(Vec(24, 160), Port::INPUT, module, BPM::CH1_CV_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(106, 165), Port::INPUT, module, BPM::RESET_CV_INPUT));
    addParam(ParamWidget::create<LEDButton>(Vec(109, 132), module, BPM::RESET_PARAM, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(113.4, 136.4), module, BPM::RESET_LIGHT));

    addOutput(Port::create<PJ301MPort>(Vec(24, 223), Port::OUTPUT, module, BPM::CH1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(65, 223), Port::OUTPUT, module, BPM::CH2_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(105, 223), Port::OUTPUT, module, BPM::CH3_OUTPUT));

    addOutput(Port::create<PJ301MPort>(Vec(24, 274), Port::OUTPUT, module, BPM::CH4_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(65, 274), Port::OUTPUT, module, BPM::CH5_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(106, 274), Port::OUTPUT, module, BPM::CH6_OUTPUT));

    addParam(ParamWidget::create<RoundBlackKnob>(Vec(58, 140), module, BPM::BPM_PARAM, 0.0, 1.0, 0.165));
    // addChild(ModuleLightWidget::create<LargeLight<GreenLight>>(Vec(28, 130), module, BPM::PULSE_LIGHT));
    //addChild(ModuleLightWidget::create<BigOlLight<GreenLight>>(Vec(25, 70), module, BPM::RESET_LIGHT));

    NumberDisplayWidget *display = new NumberDisplayWidget();
    display->box.pos = Vec(28, 70);
    display->box.size = Vec(100, 40);
    display->value = &module->m_fBPM;
    addChild(display);
}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, BPM) {
   Model *modelBPM = Model::create<BPM, BPMWidget>("RJModules", "BPM", "[LIVE] BPM", UTILITY_TAG);
   return modelBPM;
}
