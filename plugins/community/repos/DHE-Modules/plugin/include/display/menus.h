#pragma once

#include <functional>

#include <events.hpp>
#include <ui.hpp>

namespace DHE {

struct BooleanOption : rack::MenuItem {
  template <typename Setter, typename Getter>
  BooleanOption(const std::string &name, const Setter &setter,
                const Getter &getter)
      : set{setter}, is_on{getter} {
    text = name;
  }
  void onAction(rack::EventAction &e) override { set(!is_on()); }

  void step() override {
    rightText = is_on() ? "✔" : "";
    rack::MenuItem::step();
  }

  const std::function<void(bool)> set;
  const std::function<bool()> is_on;
};
} // namespace DHE
