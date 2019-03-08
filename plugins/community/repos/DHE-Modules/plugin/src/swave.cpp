#include "dhe-modules.h"
#include "display/controls.h"
#include "display/panel.h"
#include "util/range.h"
#include "util/rotation.h"
#include "util/sigmoid.h"
#include "util/signal.h"

namespace DHE {

class Swave : public rack::Module {
public:
  Swave() : Module{PARAMETER_COUNT, INPUT_COUNT, OUTPUT_COUNT} {}

  void step() override {
    auto phase = Signal::bipolar_range.normalize(signal_in());
    auto shaped = Sigmoid::taper(phase, curve(), shape());
    auto out_voltage = Signal::bipolar_range.scale(shaped);
    send_signal(out_voltage);
  }

  enum ParameterIds { CURVE_KNOB, SHAPE_SWITCH, PARAMETER_COUNT };
  enum InputIds { CURVE_CV, MAIN_IN, INPUT_COUNT };
  enum OutputIds { MAIN_OUT, OUTPUT_COUNT };

private:
  auto curve() const -> float {
    auto rotation = params[CURVE_KNOB].value;
    auto cv = inputs[CURVE_CV].value;
    return Rotation::modulated(rotation, cv);
  }

  void send_signal(float voltage) { outputs[MAIN_OUT].value = voltage; }

  auto shape() const -> float { return params[SHAPE_SWITCH].value; }

  auto signal_in() const -> float { return inputs[MAIN_IN].value; }
};

class SwavePanel : public Panel<SwavePanel> {
public:
  explicit SwavePanel(Swave *module) : Panel{module, hp} {
    auto widget_right_edge = width();

    auto x = widget_right_edge / 2.f;

    auto y = 25.f;
    auto dy = 18.5f;

    install(x, y, toggle<2>(Swave::SHAPE_SWITCH, 1));

    y += dy;
    install(x, y, knob<LargeKnob>(Swave::CURVE_KNOB));

    y += dy;
    install(x, y, input(Swave::CURVE_CV));

    y = 82.f;
    dy = 15.f;

    y += dy;
    install(x, y, input(Swave::MAIN_IN));

    y += dy;
    install(x, y, output(Swave::MAIN_OUT));
  }

  static constexpr auto module_slug = "swave";

public:
  static constexpr auto hp = 4;
};
} // namespace DHE

RACK_PLUGIN_MODEL_INIT(DHE_Modules, Swave) {
   rack::Model *modelSwave = rack::Model::create<DHE::Swave, DHE::SwavePanel>(
      "DHE-Modules", "Swave", "Swave", rack::WAVESHAPER_TAG);
   return modelSwave;
}
