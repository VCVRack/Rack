#include "rack.hpp"

using namespace rack;

namespace rack_plugin_rcm {

struct PastePatternItem : MenuItem {
  PianoRollWidget *widget = NULL;
  PianoRollModule *module = NULL;
  void onAction(EventAction &e) override {
    module->patternData.pastePattern(module->transport.currentPattern());
  }
};

} // namespace rack_plugin_rcm
