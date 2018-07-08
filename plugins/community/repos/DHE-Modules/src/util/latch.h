#pragma once

#include <functional>
#include <vector>

namespace rack_plugin_DHE_Modules {

class Latch {

public:
  bool is_high() const { return state==State::HIGH; }

  /**
   * Suspends firing events.
   */
  void disable() {
    enabled = false;
  }

  /**
   * Resumes firing events.
   */
  void enable() {
    enabled = true;
  }

  /**
   * Registers an action to be called on each rising edge.
   * @param action called on each rising edge
   */
  void on_rise(std::function<void()> action) {
    rise_actions.push_back(std::move(action));
  }

  /**
   * Registers an action to be called on each falling edge.
   * @param action called on each falling edge
   */
  void on_fall(std::function<void()> action) {
    fall_actions.push_back(std::move(action));
  }

protected:
  enum class State {
    UNKNOWN, LOW, HIGH
  } state = State::UNKNOWN;

  void set_state(State incoming_state) {
    if (state==incoming_state) return;
    state = incoming_state;
    fire(state==State::HIGH ? rise_actions : fall_actions);
  }

private:
  bool enabled = true;
  std::vector<std::function<void()>> rise_actions;
  std::vector<std::function<void()>> fall_actions;

  void fire(const std::vector<std::function<void()>> &actions) const {
    if (!enabled)
      return;
    for (const auto &action : actions)
      action();
  }
};
}
