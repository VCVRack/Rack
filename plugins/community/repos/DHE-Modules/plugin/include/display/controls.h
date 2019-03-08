#pragma once

#include <utility>

#include <app.hpp>
#include <componentlibrary.hpp>

#include "display/controls.h"
#include "display/menus.h"

namespace DHE {
template <typename T> class Control {
public:
  std::function<void(T)> notify{[](T) {}};
};

template <typename P>
class Knob : public Control<float>, public rack::RoundKnob {
public:
  explicit Knob(const std::string &size) {
    static const auto prefix = std::string{"knob-"};
    setSVG(P::svg(prefix + size));
    shadow->opacity = 0.f;
  }

  void onChange(rack::EventChange &e) override {
    rack::RoundKnob::onChange(e);
    notify(this->value);
  }
};

template <typename P> class LargeKnob : public Knob<P> {
public:
  LargeKnob() : Knob<P>("large") {}
};

template <typename P> class MediumKnob : public Knob<P> {
public:
  MediumKnob() : Knob<P>("medium") {}
};

template <typename P> class SmallKnob : public Knob<P> {
public:
  SmallKnob() : Knob<P>("small") {}
};

template <typename P> class TinyKnob : public Knob<P> {
public:
  TinyKnob() : Knob<P>("tiny") {}
};

template <typename P>
class Button : public Control<bool>,
               public rack::SVGSwitch,
               public rack::MomentarySwitch {
public:
  explicit Button(const std::string &name = "button") {
    addFrame(P::svg(name + "-1"));
    addFrame(P::svg(name + "-2"));
  }

  void onChange(rack::EventChange &e) override {
    rack::SVGSwitch::onChange(e);
    notify(this->value > 0.5f);
  }
};

template <typename P> class ReverseButton : public Button<P> {
public:
  ReverseButton() : Button<P>("button-reversed") {}
};

template <typename P, int N>
class Toggle : public Control<int>,
               public rack::SVGSwitch,
               public rack::ToggleSwitch {
public:
  explicit Toggle(const std::string &name = "toggle-" + std::to_string(N)) {
    auto base = name + "-";
    for (int position = 1; position <= size; position++) {
      addFrame(P::svg(base + std::to_string(position)));
    }
  }

  void onChange(rack::EventChange &e) override {
    rack::SVGSwitch::onChange(e);
    notify(static_cast<int>(this->value));
  }

  static constexpr auto size = N;
};
} // namespace DHE
