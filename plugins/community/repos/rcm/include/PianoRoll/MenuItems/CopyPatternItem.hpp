#include "rack.hpp"

using namespace rack;

namespace rack_plugin_rcm {

struct CopyPatternItem : MenuItem {
  PianoRollWidget *widget = NULL;
  PianoRollModule *module = NULL;
  int type;
  void onAction(EventAction &e) override {
    module->patternData.copyPattern(module->transport.currentPattern());
    widget->state = PATTERNLOADED;
  }
};

} // namespace rack_plugin_rcm
