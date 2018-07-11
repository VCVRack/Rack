#include <stdint.h>

#include "../deps/SynthDevKit/src/CV.hpp"
#include "DrumKit.hpp"
#include "DrumModule.hpp"
#include "snare.h"

namespace rack_plugin_DrumKit {

struct SnareModule : DrumModule {
  void setupSamples( ) override;
};

void SnareModule::setupSamples( ) {
  samples[0] = { (float *)snare1, snare1_len };
  samples[1] = { (float *)snare2, snare2_len };
  samples[2] = { (float *)snare3, snare3_len };
  samples[3] = { (float *)snare4, snare4_len };
  samples[4] = { (float *)snare5, snare5_len };
  samples[5] = { (float *)snare6, snare6_len };
  samples[6] = { (float *)snare7, snare7_len };
  samples[7] = { (float *)snare8, snare8_len };
  samples[8] = { (float *)snare9, snare9_len };
  samples[9] = { (float *)snare10, snare10_len };
  samples[10] = { (float *)snare11, snare11_len };
  samples[11] = { (float *)snare12, snare12_len };
  samples[12] = { (float *)snare13, snare13_len };
  samples[13] = { (float *)snare14, snare14_len };
  samples[14] = { (float *)snare15, snare15_len };
  samples[15] = { (float *)snare16, snare16_len };

  numSamples = 16;
}

struct SnareWidget : ModuleWidget {
    SnareWidget(SnareModule *module);
};

SnareWidget::SnareWidget(SnareModule *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel( );
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/Snare.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(
      Port::create<PJ301MPort>(Vec(10, 45), Port::INPUT, module, SnareModule::CLOCK1_INPUT));
  addParam(ParamWidget::create<RoundBlackSnapKnob>(
      Vec(8, 92), module, SnareModule::DRUM1_PARAM, 1.0, 16.0, 8.0));

  addOutput(Port::create<PJ301MPort>(Vec(10, 149), Port::OUTPUT, module,
                                     SnareModule::AUDIO1_OUTPUT));

  addInput(
      Port::create<PJ301MPort>(Vec(10, 205), Port::INPUT, module, SnareModule::CLOCK2_INPUT));
  addParam(ParamWidget::create<RoundBlackSnapKnob>(
      Vec(8, 252), module, SnareModule::DRUM2_PARAM, 1.0, 16.0, 8.0));

  addOutput(Port::create<PJ301MPort>(Vec(10, 308), Port::OUTPUT, module,
                                     SnareModule::AUDIO2_OUTPUT));
}

} // namespace rack_plugin_DrumKit

using namespace rack_plugin_DrumKit;

RACK_PLUGIN_MODEL_INIT(DrumKit, Snare) {
   Model *modelSnare = Model::create<SnareModule, SnareWidget>("DrumKit", "Snare Drum N", "Snare Drum N", DRUM_TAG);
   return modelSnare;
}
