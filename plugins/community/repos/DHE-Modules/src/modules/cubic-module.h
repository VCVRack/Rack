#pragma once

#include "module.h"

namespace rack_plugin_DHE_Modules {

struct CubicModule : Module {
  CubicModule() : Module{PARAMETER_COUNT, INPUT_COUNT, OUTPUT_COUNT} {
  }

  static const Range &gain_range() {
    static constexpr auto gain_range = Range{0.f, 2.f};
    return gain_range;
  }

  static const Range &coefficient_range() {
    static constexpr auto coefficient_range = Range{-2.f, 2.f};
    return coefficient_range;
  }

  float gain(int knob, int cv) const {
    return gain_range().scale(modulated(knob, cv));
  }

  float coefficient(int knob, int cv) const {
    return coefficient_range().scale(modulated(knob, cv));
  }

  float a() const { return coefficient(X3_KNOB, X3_CV); }
  float b() const { return coefficient(X2_KNOB, X2_CV); }
  float c() const { return coefficient(X1_KNOB, X1_CV); }
  float d() const { return coefficient(X0_KNOB, X0_CV); }
  float input_gain() const { return gain(INPUT_GAIN_KNOB, INPUT_GAIN_CV); }
  float output_gain() const { return gain(OUTPUT_GAIN_KNOB, OUTPUT_GAIN_CV); }

  void step() override {
    auto x = input_gain()*input(IN)/5.f;
    auto x2 = x*x;
    auto x3 = x2*x;

    auto y = output_gain()*(a()*x3 + b()*x2 + c()*x + d());

    outputs[OUT].value = 5.f*y;
  }

  enum ParameterIds {
    X3_KNOB,
    X2_KNOB,
    X1_KNOB,
    X0_KNOB,
    INPUT_GAIN_KNOB,
    OUTPUT_GAIN_KNOB,
    PARAMETER_COUNT
  };
  enum InputIds {
    IN,
    X3_CV,
    X2_CV,
    X1_CV,
    X0_CV,
    INPUT_GAIN_CV,
    OUTPUT_GAIN_CV,
    INPUT_COUNT
  };
  enum OutputIds {
    OUT,
    OUTPUT_COUNT
  };
};

}
