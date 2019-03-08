#pragma once

namespace DHE {

class Trigger {
public:
  void step() {
    auto old_state = state;
    state = state_in();
    if (state != old_state)
      on_state_change(state);
  }

protected:
  virtual auto state_in() const -> bool = 0;
  virtual void on_rise() = 0;
  virtual void on_state_change(bool state) {
    if (state) {
      on_rise();
    }
  }

private:
  bool state = false;
};
} // namespace DHE
