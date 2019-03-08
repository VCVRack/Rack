#include "rack.hpp"

using namespace rack;

namespace rack_plugin_rcm {

struct ClockBufferItem : MenuItem {
  char buffer[100];
  PianoRollModule* module;
  int value;
  ClockBufferItem(PianoRollModule* module, int value) {
    this->module = module;
    this->value = value;

    snprintf(buffer, 10, "%d", value);
    text = buffer;
    if (value == module->clockDelay) {
      rightText = "✓";
    }
  }
  void onAction(EventAction &e) override {
    module->clockDelay = value;
  }
};

} // namespace rack_plugin_rcm
