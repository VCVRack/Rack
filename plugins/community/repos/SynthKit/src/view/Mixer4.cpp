#include "../controller/Mixer4.hpp"
#include "../../deps/rack-components/display.hpp"
#include "../../deps/rack-components/jacks.hpp"
#include "../../deps/rack-components/knobs.hpp"
#include "../../deps/rack-components/sliders.hpp"

#include "componentlibrary.hpp"

namespace rack_plugin_SynthKit {

struct Mixer4Widget : ModuleWidget {
  Mixer4Widget(Mixer4Module *module);
};

Mixer4Widget::Mixer4Widget(Mixer4Module *module) : ModuleWidget(module) {
  box.size = Vec(20 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/Mixer4.svg")));
    addChild(panel);
  }

  for (uint8_t i = 0; i < MIXER_CHANNELS; i++) {
    // channel input
    addInput(Port::create<RCJackSmallDark>(Vec(11 + (i * 42), 10), Port::INPUT,
                                           module,
                                           Mixer4Module::MIXER_INPUT + i));

    // volume CV
    addInput(Port::create<RCJackSmallDark>(Vec(11 + (i * 42), 37), Port::INPUT,
                                           module,
                                           Mixer4Module::MIXER_CV_INPUT + i));

    // pan CV
    addInput(Port::create<RCJackSmallDark>(Vec(11 + (i * 42), 64), Port::INPUT,
                                           module,
                                           Mixer4Module::PAN_CV_INPUT + i));

    // left LED
    {
      LEDSmallDisplay *led = new LEDSmallDisplay();
      led->value = &module->channel_led_l[i];
      led->box.pos = Vec(13 + (i * 42), 100);
      led->box.size = Vec(8, 80);
      addChild(led);
    }

    // right LED
    {
      LEDSmallDisplay *led = new LEDSmallDisplay();
      led->value = &module->channel_led_r[i];
      led->box.pos = Vec(27 + (i * 42), 100);
      led->box.size = Vec(8, 80);
      addChild(led);
    }

    // volume slider
    addParam(ParamWidget::create<RCSlider>(Vec(13 + (i * 42), 180), module,
                                           Mixer4Module::VOLUME_PARAM + i, 0.0f,
                                           1.2f, 0.8f));

    // pan param
    addParam(ParamWidget::create<RCKnobWhite>(Vec(9 + (i * 42), 278), module,
                                              Mixer4Module::PAN_PARAM + i, 0.0,
                                              1.0, 0.5));

    // solo button
    addParam(ParamWidget::create<LEDBezel>(Vec(13 + (i * 42), 320), module,
                                           Mixer4Module::SOLO_PARAM + i, 0.0f,
                                           1.0f, 0.0f));
    addChild(ModuleLightWidget::create<ButtonLight<GreenLight>>(
        Vec(15.2 + (i * 42), 322), module, Mixer4Module::SOLO_LIGHT + i));

    // mute button
    addParam(ParamWidget::create<LEDBezel>(Vec(13 + (i * 42), 342), module,
                                           Mixer4Module::MUTE_PARAM + i, 0.0f,
                                           1.0f, 0.0f));
    addChild(ModuleLightWidget::create<ButtonLight<RedLight>>(
        Vec(15.2 + (i * 42), 344), module, Mixer4Module::MUTE_LIGHT + i));

    // send left output
    addOutput(Port::create<RCJackSmallDark>(Vec(240, 10 + (i * 92)),
                                            Port::OUTPUT, module,
                                            Mixer4Module::SENDL_OUTPUT));

    // send right output
    addOutput(Port::create<RCJackSmallDark>(Vec(270, 10 + (i * 92)),
                                            Port::OUTPUT, module,
                                            Mixer4Module::SENDR_OUTPUT));

    // recv left input
    addInput(Port::create<RCJackSmallDark>(Vec(240, 37 + (i * 92)), Port::INPUT,
                                           module, Mixer4Module::RECVL_INPUT));

    // recv right input
    addInput(Port::create<RCJackSmallDark>(Vec(270, 37 + (i * 92)), Port::INPUT,
                                           module, Mixer4Module::RECVR_INPUT));

    // mix left param
    addParam(ParamWidget::create<RCKnobWhiteSmall>(
        Vec(240, 67 + (i * 92)), module, Mixer4Module::MIXL_PARAM + i, 0.0, 1.0,
        0.5));

    // mix right param
    addParam(ParamWidget::create<RCKnobWhiteSmall>(
        Vec(270, 67 + (i * 92)), module, Mixer4Module::MIXR_PARAM + i, 0.0, 1.0,
        0.5));
  }

  // left master LED
  {
    LEDWideDisplay *led = new LEDWideDisplay();
    led->value = &module->master_led_l;
    led->box.pos = Vec(180, 100);
    led->box.size = Vec(16, 110);
    addChild(led);
  }

  // right master LED
  {
    LEDWideDisplay *led = new LEDWideDisplay();
    led->value = &module->master_led_r;
    led->box.pos = Vec(206, 100);
    led->box.size = Vec(16, 110);
    addChild(led);
  }

  // send left master output
  addOutput(Port::create<RCJackSmallDark>(Vec(176, 10), Port::OUTPUT, module,
                                          Mixer4Module::MASTERL_SEND_OUTPUT));

  // send right master output
  addOutput(Port::create<RCJackSmallDark>(Vec(202, 10), Port::OUTPUT, module,
                                          Mixer4Module::MASTERR_SEND_OUTPUT));

  // recv left master input
  addInput(Port::create<RCJackSmallDark>(Vec(176, 37), Port::INPUT, module,
                                         Mixer4Module::MASTERL_RECV_INPUT));

  // recv right master input
  addInput(Port::create<RCJackSmallDark>(Vec(202, 37), Port::INPUT, module,
                                         Mixer4Module::MASTERR_RECV_INPUT));

  // mix left master param
  addParam(ParamWidget::create<RCKnobWhiteSmall>(
      Vec(176, 67), module, Mixer4Module::MASTERL_MIX_PARAM, 0.0, 1.0, 0.5));

  // mix right master param
  addParam(ParamWidget::create<RCKnobWhiteSmall>(
      Vec(202, 67), module, Mixer4Module::MASTERR_MIX_PARAM, 0.0, 1.0, 0.5));

  // left volume slider
  addParam(ParamWidget::create<RCSlider>(
      Vec(176, 214), module, Mixer4Module::MASTERL_PARAM, 0.0f, 1.2f, 0.8f));

  // right volume slider
  addParam(ParamWidget::create<RCSlider>(
      Vec(202, 214), module, Mixer4Module::MASTERR_PARAM, 0.0f, 1.2f, 0.8f));

  // left output
  addOutput(Port::create<RCJackSmallDark>(Vec(175, 314), Port::OUTPUT, module,
                                          Mixer4Module::LEFT_OUTPUT));

  // right output
  addOutput(Port::create<RCJackSmallDark>(Vec(201, 314), Port::OUTPUT, module,
                                          Mixer4Module::RIGHT_OUTPUT));

  // mute button left
  addParam(ParamWidget::create<LEDBezel>(
      Vec(176, 342), module, Mixer4Module::MUTEL_PARAM, 0.0f, 1.0f, 0.0f));
  addChild(ModuleLightWidget::create<ButtonLight<RedLight>>(
      Vec(178.2, 344), module, Mixer4Module::MUTEL_LIGHT));

  // mute button right
  addParam(ParamWidget::create<LEDBezel>(
      Vec(202, 342), module, Mixer4Module::MUTER_PARAM, 0.0f, 1.0f, 0.0f));
  addChild(ModuleLightWidget::create<ButtonLight<RedLight>>(
      Vec(204.2, 344), module, Mixer4Module::MUTER_LIGHT));

  addChild(Widget::create<ScrewSilver>(Vec(32, 0)));
  addChild(
      Widget::create<ScrewSilver>(Vec(32, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

} // namespace rack_plugin_SynthKit

using namespace rack_plugin_SynthKit;

RACK_PLUGIN_MODEL_INIT(SynthKit, Mixer4) {
   Model *modelMixer4 = Model::create<Mixer4Module, Mixer4Widget>(
      "SynthKit", "Mixer 4", "Mixer 4", MIXER_TAG);
   return modelMixer4;
}
