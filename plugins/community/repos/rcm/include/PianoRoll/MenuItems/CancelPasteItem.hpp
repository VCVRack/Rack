#include "rack.hpp"

using namespace rack;

namespace rack_plugin_rcm {

struct CancelPasteItem : MenuItem {
  PianoRollWidget *widget = NULL;
  void onAction(EventAction &e) override {
    widget->state = COPYREADY;
  }
};

} // namespace rack_plugin_rcm
