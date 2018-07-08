#pragma once

#include <functional>
#include <vector>
#include "util/d-flip-flop.h"

namespace rack_plugin_DHE_Modules {
struct Mode {
  void enter() { fire(entry_actions); }
  void exit() { fire(exit_actions); }
  void step() { fire(step_actions); }

  void on_entry(std::function<void()> action) {
    entry_actions.push_back(std::move(action));
  }

  void on_exit(std::function<void()> action) {
    exit_actions.push_back(std::move(action));
  }

  void on_step(std::function<void()> action) {
    step_actions.push_back(std::move(action));
  }

private:
  void fire(const std::vector<std::function<void()>> &actions) {
    for (const auto &action : actions) action();
  }

  std::vector<std::function<void()>> entry_actions;
  std::vector<std::function<void()>> exit_actions;
  std::vector<std::function<void()>> step_actions;
};

class SubmodeSwitch : public Mode {
  DFlipFlop submode_switch;
  Mode *submode;

  void enter_submode(Mode *incoming_submode) {
    submode->exit();
    submode = incoming_submode;
    submode->enter();
  }

public:
  SubmodeSwitch(std::function<float()> switch_signal, Mode *low_mode, Mode *high_mode) :
      submode_switch{std::move(switch_signal)}, submode{low_mode} {
    submode_switch.on_rise([this, high_mode] {
      enter_submode(high_mode);
    });
    submode_switch.on_fall([this, low_mode] {
      enter_submode(low_mode);
    });

    on_entry([this] {
      submode->enter();
    });
    on_step([this] {
      submode_switch.step();
      submode->step();
    });
    on_exit([this] {
      submode->exit();
    });
  }
};
}
