#include "rack.hpp"

using namespace rack;

namespace rack_plugin_rcm {

struct ClearNotesItem : MenuItem {
  PianoRollModule *module = NULL;

  ClearNotesItem(PianoRollModule* module) {
    this->module = module;
    text = "Clear Notes";
  }

  void onAction(EventAction &e) override {
    module->patternData.clearPatternSteps(module->transport.currentPattern());
  }
};

} // namespace rack_plugin_rcm
