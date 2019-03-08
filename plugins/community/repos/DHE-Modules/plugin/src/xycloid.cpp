#include <array>
#include <math.h>

#include "dhe-modules.h"
#include "display/controls.h"
#include "display/panel.h"
#include "util/rotation.h"
#include "util/sigmoid.h"
#include "util/signal.h"

namespace DHE {

class XycloidRotor {
public:
  void advance(float delta, float offset = 0.f) {
    this->offset = offset;
    phase += delta;
    phase -= std::trunc(phase);
  }

  auto x() const -> float { return std::cos(two_pi * (phase + offset)); }
  auto y() const -> float { return std::sin(two_pi * (phase + offset)); }

private:
  float const two_pi{2.f * std::acos(-1.f)};
  float phase{0.f};
  float offset{0.f};
};

class Xycloid : public rack::Module {
public:
  Xycloid() : Module{PARAMETER_COUNT, INPUT_COUNT, OUTPUT_COUNT} {}

  void set_musical_wobble_ratios(bool is_musical) {
    wobble_ratio_offset = is_musical ? 0.f : 1.f;
  }

  auto is_musical_wobble_ratios() const -> bool {
    return wobble_ratio_offset == 0.f;
  }

  void step() override {
    auto wobble_ratio = this->wobble_ratio();
    auto wobble_phase_offset = wobble_phase_in();
    if (wobble_ratio < 0.f)
      wobble_phase_offset *= -1.f;

    auto throb_speed = this->throb_speed();
    auto wobble_speed = wobble_ratio * throb_speed;
    auto wobble_depth = this->wobble_depth();
    auto throb_depth = 1.f - wobble_depth;

    throbber.advance(throb_speed);
    wobbler.advance(wobble_speed, wobble_phase_offset);
    auto x = throb_depth * throbber.x() + wobble_depth * wobbler.x();
    auto y = throb_depth * throbber.y() + wobble_depth * wobbler.y();

    outputs[X_OUT].value = 5.f * x_gain_in() * (x + x_offset());
    outputs[Y_OUT].value = 5.f * y_gain_in() * (y + y_offset());
  }

  enum ParameterIds {
    WOBBLE_RATIO_KNOB,
    WOBBLE_RATIO_AV,
    WOBBLE_RANGE_SWITCH,
    WOBBLE_DEPTH_KNOB,
    WOBBLE_DEPTH_AV,
    THROB_SPEED_KNOB,
    THROB_SPEED_AV,
    X_GAIN_KNOB,
    Y_GAIN_KNOB,
    X_RANGE_SWITCH,
    Y_RANGE_SWITCH,
    WOBBLE_RATIO_FREEDOM_SWITCH,
    WOBBLE_PHASE_KNOB,
    PARAMETER_COUNT
  };
  enum InputIds {
    WOBBLE_RATIO_CV,
    WOBBLE_DEPTH_CV,
    THROB_SPEED_CV,
    X_GAIN_CV,
    Y_GAIN_CV,
    INPUT_COUNT
  };
  enum OutputIds { X_OUT, Y_OUT, OUTPUT_COUNT };

private:
  auto is_wobble_ratio_free() const -> bool {
    return params[WOBBLE_RATIO_FREEDOM_SWITCH].value > 0.1f;
  }

  auto modulated(const ParameterIds &knob_param, const InputIds &cv_input) const
      -> float {
    auto rotation = params[knob_param].value;
    auto cv = inputs[cv_input].value;
    return Rotation::modulated(rotation, cv);
  }

  auto modulated(const ParameterIds &knob_param, const InputIds &cv_input,
                 const ParameterIds &av_param) const -> float {
    auto rotation = params[knob_param].value;
    auto cv = inputs[cv_input].value;
    auto av = params[av_param].value;
    return Rotation::modulated(rotation, cv, av);
  }

  auto offset(int param) const -> float {
    auto is_uni = params[param].value > 0.5f;
    return is_uni ? 1.f : 0.f;
  }

  auto throb_speed() const -> float {
    constexpr auto speed_taper_curvature = 0.8f;
    auto rotation = modulated(THROB_SPEED_KNOB, THROB_SPEED_CV, THROB_SPEED_AV);
    auto scaled = throb_speed_knob_range.scale(rotation);
    auto tapered = Sigmoid::inverse(scaled, speed_taper_curvature);
    return -10.f * tapered * rack::engineGetSampleTime();
  }

  auto wobble_depth() const -> float {
    auto rotation =
        modulated(WOBBLE_DEPTH_KNOB, WOBBLE_DEPTH_CV, WOBBLE_DEPTH_AV);
    return wobble_depth_range.clamp(rotation);
  }

  auto wobble_phase_in() const -> float {
    auto rotation = params[WOBBLE_PHASE_KNOB].value;
    return rotation - 0.5f;
  }

  auto wobble_ratio_range() const -> const Range & {
    static constexpr auto wobble_ratio_max = 16.f;
    static constexpr auto inward_wobble_ratio_range =
        Range{0.f, wobble_ratio_max};
    static constexpr auto outward_wobble_ratio_range =
        Range{0.f, -wobble_ratio_max};
    static constexpr auto bidirectional_wobble_ratio_range =
        Range{wobble_ratio_max, -wobble_ratio_max};
    static constexpr std::array<Range, 3> wobble_ratio_ranges{
        inward_wobble_ratio_range, bidirectional_wobble_ratio_range,
        outward_wobble_ratio_range};

    const auto param = params[WOBBLE_RANGE_SWITCH].value;
    const auto selection = static_cast<int>(param);

    return wobble_ratio_ranges[selection];
  }

  auto wobble_ratio() const -> float {
    auto wobble_ratio_amount =
        modulated(WOBBLE_RATIO_KNOB, WOBBLE_RATIO_CV, WOBBLE_RATIO_AV);
    auto wobble_ratio =
        wobble_ratio_range().scale(wobble_ratio_amount) + wobble_ratio_offset;
    return is_wobble_ratio_free() ? wobble_ratio : std::round(wobble_ratio);
  }

  auto x_offset() const -> float { return offset(X_RANGE_SWITCH); }

  auto x_gain_in() const -> float {
    return Rotation::gain_multiplier(modulated(X_GAIN_KNOB, X_GAIN_CV));
  }

  auto y_gain_in() const -> float {
    return Rotation::gain_multiplier(modulated(Y_GAIN_KNOB, Y_GAIN_CV));
  }

  auto y_offset() const -> float { return offset(Y_RANGE_SWITCH); }

  json_t *toJson() override {
    json_t *configuration = json_object();
    json_object_set_new(configuration, "musical_wobble_ratios",
                        json_boolean(is_musical_wobble_ratios()));
    return configuration;
  }

  void fromJson(json_t *configuration) override {
    json_t *musical_wobble_ratios =
        json_object_get(configuration, "musical_wobble_ratios");
    set_musical_wobble_ratios(json_is_true(musical_wobble_ratios));
  }

  static constexpr auto throb_speed_knob_range = Range{-1.f, 1.f};
  static constexpr auto wobble_depth_range = Range{0.f, 1.f};

  float wobble_ratio_offset{0.f};
  XycloidRotor wobbler{};

  XycloidRotor throbber{};
};

class XycloidPanel : public Panel<XycloidPanel> {
public:
  explicit XycloidPanel(Xycloid *module) : Panel{module, hp} {
    auto widget_right_edge = width();

    auto column_1 = widget_right_edge / 7.f;
    auto column_4 = widget_right_edge - column_1;
    auto column_2 = (column_4 - column_1) / 3.f + column_1;
    auto column_3 = widget_right_edge - column_2;

    auto y = 30.f;
    auto dy = 22.f;

    install(column_1, y, input(Xycloid::WOBBLE_RATIO_CV));
    install(column_2, y, knob<TinyKnob>(Xycloid::WOBBLE_RATIO_AV));
    install(column_3, y, knob<LargeKnob>(Xycloid::WOBBLE_RATIO_KNOB));
    install(column_4, y, toggle<2>(Xycloid::WOBBLE_RATIO_FREEDOM_SWITCH, 1));

    y += dy;
    install(column_1, y, input(Xycloid::WOBBLE_DEPTH_CV));
    install(column_2, y, knob<TinyKnob>(Xycloid::WOBBLE_DEPTH_AV));
    install(column_3, y, knob<LargeKnob>(Xycloid::WOBBLE_DEPTH_KNOB));
    install(column_4, y, toggle<3>(Xycloid::WOBBLE_RANGE_SWITCH, 2));

    y += dy;
    install(column_1, y, input(Xycloid::THROB_SPEED_CV));
    install(column_2, y, knob<TinyKnob>(Xycloid::THROB_SPEED_AV));
    install(column_3, y, knob<LargeKnob>(Xycloid::THROB_SPEED_KNOB, 0.65f));
    install(column_4, y, knob<SmallKnob>(Xycloid::WOBBLE_PHASE_KNOB));

    y = 82.f;
    dy = 15.f;
    const auto output_port_offset = 1.25;

    auto default_gain = Rotation::gain_range.normalize(1.f);

    y += dy;
    install(column_1, y, input(Xycloid::X_GAIN_CV));
    install(column_2, y, knob<SmallKnob>(Xycloid::X_GAIN_KNOB, default_gain));
    install(column_3, y, toggle<2>(Xycloid::X_RANGE_SWITCH, 0));
    install(column_4, y + output_port_offset, output(Xycloid::X_OUT));

    y += dy;
    install(column_1, y, input(Xycloid::Y_GAIN_CV));
    install(column_2, y, knob<SmallKnob>(Xycloid::Y_GAIN_KNOB, default_gain));
    install(column_3, y, toggle<2>(Xycloid::Y_RANGE_SWITCH, 0));
    install(column_4, y + output_port_offset, output(Xycloid::Y_OUT));
  }

  void appendContextMenu(rack::Menu *menu) override {
    auto xycloid = dynamic_cast<Xycloid *>(module);
    assert(xycloid);

    menu->addChild(rack::construct<rack::MenuLabel>());
    menu->addChild(
        rack::construct<rack::MenuLabel>(&rack::MenuLabel::text, "Options"));
    menu->addChild(new BooleanOption(
        "Musical Ratios",
        [xycloid](bool setting) {
          xycloid->set_musical_wobble_ratios(setting);
        },
        [xycloid] { return xycloid->is_musical_wobble_ratios(); }));
  }

  static constexpr auto module_slug = "xycloid";

private:
  static constexpr auto hp = 11;
};

} // namespace DHE

RACK_PLUGIN_MODEL_INIT(DHE_Modules, Xycloid) {
   rack::Model *modelXycloid =
      rack::Model::create<DHE::Xycloid, DHE::XycloidPanel>(
         "DHE-Modules", "Xycloid", "Xycloid", rack::FUNCTION_GENERATOR_TAG,
         rack::LFO_TAG);
   return modelXycloid;
}
