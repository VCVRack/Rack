#pragma once

#include "trigger.h"

namespace DHE {

class Gate : public Trigger {
protected:
  virtual void on_fall() = 0;
  void on_state_change(bool state) override {
    if (state) {
      on_rise();
    } else {
      on_fall();
    }
  }
};
} // namespace DHE
