#include "RJModules.hpp"

#include "dsp/digital.hpp"
#include "common.hpp"
#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace rack_plugin_RJModules {

/*
Thanks to Strum for the display widget!
*/

struct Range: Module {
    enum ParamIds {
        CH1_PARAM,
        CH2_PARAM,
        CH3_PARAM,
        CH4_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        CH1_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        CH1_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    float display1_val;
    float display2_val;
    float display3_val;
    float display4_val;
    float display5_val;
    float display6_val;

    Range() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

struct SmallNumberDisplayWidgeter : TransparentWidget {

  float *value;
  std::shared_ptr<Font> font;

  SmallNumberDisplayWidgeter() {
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
    nvgFontSize(vg, 16);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 0.5);

    std::stringstream to_display;
    to_display = format4display(*value);

    Vec textPos = Vec(8.0f, 33.0f);
    NVGcolor textColor = nvgRGB(0x00, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);

    nvgFontSize(vg, 8);
    textPos = Vec(1.0f, (*value<0?28.0f:32.0f));
    nvgText(vg, textPos.x, textPos.y, (*value<0?"-":"+"), NULL);

  }
};

void Range::step() {

    display1_val = params[CH1_PARAM].value;
    display2_val = params[CH2_PARAM].value;
    display3_val = params[CH3_PARAM].value;
    display4_val = params[CH4_PARAM].value;
    display5_val = inputs[CH1_PARAM].value;

    // new_value = ( (old_value - old_min) / (old_max - old_min) ) * (new_max - new_min) + new_min
    float output = ( (inputs[CH1_PARAM].value - params[CH1_PARAM].value) / (params[CH2_PARAM].value - params[CH1_PARAM].value) ) * (params[CH4_PARAM].value - params[CH3_PARAM].value) + params[CH3_PARAM].value;

    display6_val = output;
    outputs[CH1_OUTPUT].value = output;

}

struct RangeWidget: ModuleWidget {
    RangeWidget(Range *module);
};

RangeWidget::RangeWidget(Range *module) : ModuleWidget(module) {
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Range.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));


    SmallNumberDisplayWidgeter *display = new SmallNumberDisplayWidgeter();
    display->box.pos = Vec(23, 60);
    display->box.size = Vec(50, 40);
    display->value = &module->display1_val;
    addChild(display);
    addParam(ParamWidget::create<RoundBlackKnob>(Vec(28, 105), module, Range::CH1_PARAM, -12.0, 12.0, -12.0));

    SmallNumberDisplayWidgeter *display2 = new SmallNumberDisplayWidgeter();
    display2->box.pos = Vec(83, 60);
    display2->box.size = Vec(50, 40);
    display2->value = &module->display2_val;
    addChild(display2);
    addParam(ParamWidget::create<RoundBlackKnob>(Vec(88, 105), module, Range::CH2_PARAM, -12.0, 12.0, 12.0));

    SmallNumberDisplayWidgeter *display3 = new SmallNumberDisplayWidgeter();
    display3->box.pos = Vec(23, 160);
    display3->box.size = Vec(50, 40);
    display3->value = &module->display3_val;
    addChild(display3);
    addParam(ParamWidget::create<RoundBlackKnob>(Vec(28, 205), module, Range::CH3_PARAM, -12.0, 12.0, -12.0));

    SmallNumberDisplayWidgeter *display4 = new SmallNumberDisplayWidgeter();
    display4->box.pos = Vec(83, 160);
    display4->box.size = Vec(50, 40);
    display4->value = &module->display4_val;
    addChild(display4);
    addParam(ParamWidget::create<RoundBlackKnob>(Vec(88, 205), module, Range::CH4_PARAM, -12.0, 12.0, 12.0));

    SmallNumberDisplayWidgeter *display5 = new SmallNumberDisplayWidgeter();
    display5->box.pos = Vec(23, 260);
    display5->box.size = Vec(50, 40);
    display5->value = &module->display5_val;
    addChild(display5);

    SmallNumberDisplayWidgeter *display6 = new SmallNumberDisplayWidgeter();
    display6->box.pos = Vec(83, 260);
    display6->box.size = Vec(50, 40);
    display6->value = &module->display6_val;
    addChild(display6);


    // addInput(Port::create<PJ301MPort>(Vec(35, 123), Port::INPUT, module, Range::CH1_INPUT));
    // addOutput(Port::create<PJ301MPort>(Vec(95, 123), Port::OUTPUT, module, Range::CH1_OUTPUT));

    // SmallNumberDisplayWidgeter *display2 = new SmallNumberDisplayWidgeter();
    // display2->box.pos = Vec(28, 160);
    // display2->box.size = Vec(100, 40);
    // display2->value = &module->display2_val;
    // addChild(display2);

    // addInput(Port::create<PJ301MPort>(Vec(35, 223), Port::INPUT, module, Range::CH2_INPUT));
    // addOutput(Port::create<PJ301MPort>(Vec(95, 223), Port::OUTPUT, module, Range::CH2_OUTPUT));

    // SmallNumberDisplayWidgeter *display3 = new SmallNumberDisplayWidgeter();
    // display3->box.pos = Vec(28, 260);
    // display3->box.size = Vec(100, 40);
    // display3->value = &module->display3_val;
    // addChild(display3);

    addInput(Port::create<PJ301MPort>(Vec(35, 323), Port::INPUT, module, Range::CH1_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(95, 323), Port::OUTPUT, module, Range::CH1_OUTPUT));

}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, Range) {
   Model *modelRange = Model::create<Range, RangeWidget>("RJModules", "Range", "[UTIL] Range", UTILITY_TAG);
   return modelRange;
}
