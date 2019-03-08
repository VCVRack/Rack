#include "whatnote-module-widget.hh"
#include "whatnote-module.hh"

namespace rack_plugin_Skylights {

whatnote_module_widget::whatnote_module_widget(Module* module) : ModuleWidget(module) {
  font = Font::load(assetPlugin(plugin, "res/LEDCalculator.ttf"));
  
  setPanel(SVG::load(assetPlugin(plugin, "res/WhatNote.svg")));

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<PJ301MPort>(Vec(57.5, 273), Port::INPUT, module, whatnote_module::AUDIO_INPUT + 0));
}

void whatnote_module_widget::draw(NVGcontext* vg) {
  whatnote_module* mod = reinterpret_cast<whatnote_module*>(module);

  static const char* semitone_labels[12] =
    { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
  
  ModuleWidget::draw(vg);
  if (!mod) return;

  char buffer[128];
  
  nvgFillColor(vg, nvgRGBA(0x00, 0x00, 0x00, 0xFF));
  nvgFontFaceId(vg, font->handle);
  nvgTextLetterSpacing(vg, 0);
  nvgTextAlign(vg, NVG_ALIGN_CENTER);

  // the big font
  nvgFontSize(vg, 20);

  if (mod->octave >= -10) {
    snprintf
      (reinterpret_cast<char*>(&buffer),
       128,
       "%s%d",
       semitone_labels[mod->semitone],
       mod->octave);
  
    nvgTextBox(vg, 25, 164, 85, buffer, 0);

    // the little fonts
    nvgFontSize(vg, 14);

    if (mod->cents > 0) {
       snprintf
	  (reinterpret_cast<char*>(&buffer),
	   128,
	   "+%d",
	   mod->cents);
    } else {
       snprintf
	  (reinterpret_cast<char*>(&buffer),
	   128,
	   "%d",
	   mod->cents);
    }
  
    nvgTextBox(vg, 25, 182, 85, buffer, 0);
  } else {
    nvgTextBox(vg, 25, 164, 85, "--", 0);
    
    // the little fonts
    nvgFontSize(vg, 14);
  }

  snprintf
    (reinterpret_cast<char*>(&buffer),
     128,
     "%.2f V",
     mod->voltage);
  
  nvgTextBox(vg, 25, 198, 85, buffer, 0);
}

} // namespace rack_plugin_Skylights

using namespace rack_plugin_Skylights;

RACK_PLUGIN_MODEL_INIT(Skylights, whatnote_model) {
   Model *whatnote_model = Model::create<whatnote_module, whatnote_module_widget>("Skylights", "SkWhatnoteCV", "SK What Note? (CV Tuner)", TUNER_TAG, UTILITY_TAG);
   return whatnote_model;
}
