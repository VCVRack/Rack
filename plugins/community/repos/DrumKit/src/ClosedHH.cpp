#include <stdint.h>

#include "../deps/SynthDevKit/src/CV.hpp"
#include "DrumKit.hpp"
#include "DrumModule.hpp"
#include "closedhh.h"

namespace rack_plugin_DrumKit {

struct ClosedHHModule : DrumModule {
  void setupSamples( ) override;
};

void ClosedHHModule::setupSamples( ) {
  samples[0] = { (float *)closedhh1, closedhh1_len };
  samples[1] = { (float *)closedhh2, closedhh2_len };
  samples[2] = { (float *)closedhh3, closedhh3_len };
  samples[3] = { (float *)closedhh4, closedhh4_len };
  samples[4] = { (float *)closedhh5, closedhh5_len };
  samples[5] = { (float *)closedhh6, closedhh6_len };
  samples[6] = { (float *)closedhh7, closedhh7_len };
  samples[7] = { (float *)closedhh8, closedhh8_len };
  samples[8] = { (float *)closedhh9, closedhh9_len };
  samples[9] = { (float *)closedhh10, closedhh10_len };
  samples[10] = { (float *)closedhh11, closedhh11_len };
  samples[11] = { (float *)closedhh12, closedhh12_len };
  samples[12] = { (float *)closedhh13, closedhh13_len };
  samples[13] = { (float *)closedhh14, closedhh14_len };
  samples[14] = { (float *)closedhh15, closedhh15_len };

  numSamples = 15;
}

struct ClosedHHWidget : ModuleWidget {
    ClosedHHWidget(ClosedHHModule *module);
};

ClosedHHWidget::ClosedHHWidget(ClosedHHModule *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel( );
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/ClosedHH.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<PJ301MPort>(Vec(10, 45), Port::INPUT, module,
                                   ClosedHHModule::CLOCK1_INPUT));
  addParam(ParamWidget::create<RoundBlackSnapKnob>(
      Vec(8, 92), module, ClosedHHModule::DRUM1_PARAM, 1.0, 15.0, 8.0));

  addOutput(Port::create<PJ301MPort>(Vec(10, 149), Port::OUTPUT, module,
                                     ClosedHHModule::AUDIO1_OUTPUT));

  addInput(Port::create<PJ301MPort>(Vec(10, 205), Port::INPUT, module,
                                   ClosedHHModule::CLOCK2_INPUT));
  addParam(ParamWidget::create<RoundBlackSnapKnob>(
      Vec(8, 252), module, ClosedHHModule::DRUM2_PARAM, 1.0, 15.0, 8.0));

  addOutput(Port::create<PJ301MPort>(Vec(10, 308), Port::OUTPUT, module,
                                     ClosedHHModule::AUDIO2_OUTPUT));
}

} // namespace rack_plugin_DrumKit

using namespace rack_plugin_DrumKit;

RACK_PLUGIN_MODEL_INIT(DrumKit, ClosedHH) {
   Model *modelClosedHH = Model::create<ClosedHHModule, ClosedHHWidget>("DrumKit", "Closed HiHat", "Closed HiHat", DRUM_TAG);
   return modelClosedHH;
}
