#include "dhe-modules.h"
#include "display/controls.h"
#include "display/panel.h"
#include "util/rotation.h"
#include "util/signal.h"

namespace DHE {

class Cubic : public rack::Module {
public:
  Cubic() : Module{PARAMETER_COUNT, INPUT_COUNT, OUTPUT_COUNT} {}

  void step() override {
    auto a = coefficient(A_KNOB, A_CV);
    auto b = coefficient(B_KNOB, B_CV);
    auto c = coefficient(C_KNOB, C_CV);
    auto d = coefficient(D_KNOB, D_CV);
    auto input_gain = gain(INPUT_GAIN_KNOB, INPUT_GAIN_CV);
    auto output_gain = gain(OUTPUT_GAIN_KNOB, OUTPUT_GAIN_CV);

    auto x = input_gain * main_in() * 0.2f;
    auto x2 = x * x;
    auto x3 = x2 * x;
    auto y = a * x3 + b * x2 + c * x + d;
    auto output_voltage = output_gain * y * 5.f;
    send_main_out(output_voltage);
  }
  enum ParameterIds {
    A_KNOB,
    B_KNOB,
    C_KNOB,
    D_KNOB,
    INPUT_GAIN_KNOB,
    OUTPUT_GAIN_KNOB,
    PARAMETER_COUNT
  };

  enum InputIds {
    MAIN_IN,
    A_CV,
    B_CV,
    C_CV,
    D_CV,
    INPUT_GAIN_CV,
    OUTPUT_GAIN_CV,
    INPUT_COUNT
  };

  enum OutputIds { MAIN_OUT, OUTPUT_COUNT };

private:
  auto coefficient(ParameterIds knob_param, InputIds cv_param) const -> float {
    static auto constexpr coefficient_range = Range{-2.0f, 2.0f};
    return coefficient_range.scale(modulated(knob_param, cv_param));
  }

  auto gain(const ParameterIds knob_param, const InputIds cv_input) const
      -> float {
    return Rotation::gain_multiplier(modulated(knob_param, cv_input));
  }

  auto main_in() const -> float { return inputs[MAIN_IN].value; }

  auto modulated(ParameterIds knob_param, InputIds cv_input) const -> float {
    auto rotation = params[knob_param].value;
    auto cv = inputs[cv_input].value;
    return Rotation::modulated(rotation, cv);
  }

  void send_main_out(float voltage) { outputs[MAIN_OUT].value = voltage; }
};

class CubicPanel : public Panel<CubicPanel> {
public:
  explicit CubicPanel(Cubic *module) : Panel{module, hp} {
    auto widget_right_edge = width();

    auto column_1 = width() / 4.f + 0.333333f;
    auto column_2 = widget_right_edge - column_1;

    auto y = 20.f;
    auto dy = 15.f;

    install(column_1, y, input(Cubic::A_CV));
    install(column_2, y, knob<SmallKnob>(Cubic::A_KNOB));

    y += dy;
    install(column_1, y, input(Cubic::B_CV));
    install(column_2, y, knob<SmallKnob>(Cubic::B_KNOB));

    y += dy;
    install(column_1, y, input(Cubic::C_CV));
    install(column_2, y, knob<SmallKnob>(Cubic::C_KNOB));

    y += dy;
    install(column_1, y, input(Cubic::D_CV));
    install(column_2, y, knob<SmallKnob>(Cubic::D_KNOB));

    y = 82.f;
    install(column_1, y, knob<SmallKnob>(Cubic::INPUT_GAIN_KNOB));
    install(column_2, y, knob<SmallKnob>(Cubic::OUTPUT_GAIN_KNOB));

    y += dy;
    install(column_1, y, input(Cubic::INPUT_GAIN_CV));
    install(column_2, y, input(Cubic::OUTPUT_GAIN_CV));

    y += dy;
    install(column_1, y, input(Cubic::MAIN_IN));
    install(column_2, y, output(Cubic::MAIN_OUT));
  }

  static constexpr auto module_slug = "cubic";

private:
  static constexpr auto hp = 5;
};
} // namespace DHE

RACK_PLUGIN_MODEL_INIT(DHE_Modules, Cubic) {
   rack::Model *modelCubic = rack::Model::create<DHE::Cubic, DHE::CubicPanel>(
      "DHE-Modules", "Cubic", "Cubic", rack::FUNCTION_GENERATOR_TAG);
   return modelCubic;
}
