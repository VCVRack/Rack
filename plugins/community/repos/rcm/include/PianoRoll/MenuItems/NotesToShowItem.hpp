#include "rack.hpp"

using namespace rack;

namespace rack_plugin_rcm {

struct NotesToShowItem : MenuItem {
  char buffer[100];
  PianoRollWidget* module;
  int value;
  NotesToShowItem(PianoRollWidget* module, int value) {
    this->module = module;
    this->value = value;

    snprintf(buffer, 10, "%d", value);
    text = buffer;
    if (value == module->rollAreaWidget->state.notesToShow) {
      rightText = "✓";
    }
  }
  void onAction(EventAction &e) override {
    module->rollAreaWidget->state.notesToShow = value;
    module->rollAreaWidget->state.dirty = true;
  }
};

} // namespace rack_plugin_rcm
