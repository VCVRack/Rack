#pragma once

#include "skylights.hh"

namespace rack_plugin_Skylights {

struct whatnote_module_widget : ModuleWidget {
  std::shared_ptr<Font> font;
  
  whatnote_module_widget(Module* module);

  void draw(NVGcontext *vg) override;

};

} // namespace rack_plugin_Skylights
