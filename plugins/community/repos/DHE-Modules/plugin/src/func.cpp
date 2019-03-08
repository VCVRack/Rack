#include <vector>

#include "dhe-modules.h"
#include "display/controls.h"
#include "display/panel.h"
#include "util/rotation.h"
#include "util/signal.h"

namespace DHE {
static constexpr auto attenuation_range = Range{0.f, 1.f};
static constexpr auto invertible_attenuation_range = Range{-1.f, 1.f};
static constexpr auto gain_range = Range{0.f, 2.f};
static constexpr auto invertible_gain_range = Range{-2.f, 2.f};

static const auto multiplication_ranges = std::vector<Range const *>{
    &attenuation_range, &invertible_attenuation_range, &gain_range,
    &invertible_gain_range};

static constexpr auto half_bipolar_range = Range{0.f, 5.f};
static constexpr auto invertible_unipolar_range = Range{-10.f, 10.f};
static const auto addition_ranges = std::vector<Range const *>{
    &half_bipolar_range, &Signal::bipolar_range, &Signal::unipolar_range,
    &invertible_unipolar_range};

class FuncChannel {
public:
  FuncChannel(rack::Module *module, int input, int amount_knob_param,
              int output)
      : input_port{module->inputs[input]},
        amount{module->params[amount_knob_param].value},
        output{module->outputs[output].value} {}

  auto adjust(float upstream) -> float {
    auto input = input_port.active ? input_port.value : upstream;
    if (is_multiplication) {
      output = input * multiplication_range->scale(amount);
    } else {
      output = input + addition_range->scale(amount);
    }
    return output;
  }

  void set_operator(bool is_multiplication) {
    this->is_multiplication = is_multiplication;
  }

  void set_addition_range(int selection) {
    addition_range = addition_ranges[selection];
  }

  void set_multiplication_range(int selection) {
    multiplication_range = multiplication_ranges[selection];
  }

private:
  const rack::Input &input_port;
  bool is_multiplication = false;
  Range const *addition_range{&Signal::bipolar_range};
  Range const *multiplication_range{&gain_range};
  const float &amount;
  float &output;
};

class Func : public rack::Module {
public:
  Func() : Module{PARAMETER_COUNT, INPUT_COUNT, OUTPUT_COUNT} {}

  void step() override { channel.adjust(0.f); }

  enum ParameterIds {
    KNOB,
    OPERATOR_SWITCH,
    ADDITION_RANGE_SWITCH,
    MULTIPLICATION_RANGE_SWITCH,
    PARAMETER_COUNT
  };

  enum InputIds { IN, INPUT_COUNT };

  enum OutputIds { OUT, OUTPUT_COUNT };

  FuncChannel channel{this, IN, KNOB, OUT};
};

class Func6 : public rack::Module {
  static constexpr auto channel_count = 6;

public:
  Func6() : Module{PARAMETER_COUNT, INPUT_COUNT, OUTPUT_COUNT} {
    for (int i = 0; i < channel_count; i++) {
      channels.emplace_back(this, IN + i, KNOB + i, OUT + i);
    }
  }

  void step() override {
    auto upstream = 0.f;
    for (auto &channel : channels) {
      upstream = channel.adjust(upstream);
    }
  }

  enum ParameterIds {
    KNOB,
    OPERATOR_SWITCH = KNOB + channel_count,
    ADDITION_RANGE_SWITCH = OPERATOR_SWITCH + channel_count,
    MULTIPLICATION_RANGE_SWITCH = ADDITION_RANGE_SWITCH + channel_count,
    PARAMETER_COUNT = MULTIPLICATION_RANGE_SWITCH + channel_count
  };

  enum InputIds { IN, INPUT_COUNT = IN + channel_count };

  enum OutputIds { OUT, OUTPUT_COUNT = OUT + channel_count };

  std::vector<FuncChannel> channels{};
};

template <typename P> class MultiplicationRangeSwitch : public Toggle<P, 4> {
public:
  MultiplicationRangeSwitch() : Toggle<P, 4>("counter-mult") {}
};

template <typename P> class AdditionRangeSwitch : public Toggle<P, 4> {
public:
  AdditionRangeSwitch() : Toggle<P, 4>("counter-add") {}
};

class FuncPanel : public Panel<FuncPanel> {
public:
  explicit FuncPanel(Func *func) : Panel{func, hp} {

    auto widget_right_edge = width();

    auto x = widget_right_edge / 2.f;

    auto top = 23.f;
    auto bottom = 108.f;
    auto row_count = 6;
    auto row_spacing = (bottom - top) / (row_count - 1);
    auto port_offset = 1.25f;

    auto row_1 = top + port_offset;
    auto row_2 = top + row_spacing;
    auto row_3 = top + row_spacing * 2;
    auto row_4 = top + row_spacing * 3;
    auto row_6 = top + row_spacing * 5 + port_offset;

    auto channel = &func->channel;
    auto operator_switch_index = Func::OPERATOR_SWITCH;
    auto addition_range_switch_index = Func::ADDITION_RANGE_SWITCH;
    auto multiplication_range_switch_index = Func::MULTIPLICATION_RANGE_SWITCH;

    auto addition_range_selector = [channel](int selection) {
      channel->set_addition_range(selection);
    };

    auto multiplication_range_selector = [channel](int selection) {
      channel->set_multiplication_range(selection);
    };

    auto multiplication_range_switch = toggle<MultiplicationRangeSwitch>(
        multiplication_range_switch_index, 2, multiplication_range_selector);

    auto addition_range_switch = toggle<AdditionRangeSwitch>(
        addition_range_switch_index, 1, addition_range_selector);

    auto select_operator = [channel, addition_range_switch,
                            multiplication_range_switch](int position) {
      auto is_multiplication = position == 1;
      channel->set_operator(is_multiplication);
      multiplication_range_switch->visible = is_multiplication;
      addition_range_switch->visible = !is_multiplication;
    };

    auto operator_switch = toggle<2>(operator_switch_index, 0, select_operator);

    install(x, row_1, input(Func::IN));
    install(x, row_2, operator_switch);
    install(x, row_4, addition_range_switch);
    install(x, row_4, multiplication_range_switch);
    install(x, row_3, knob<LargeKnob>(Func::KNOB));
    install(x, row_6, output(Func::OUT));
  }

  static constexpr auto module_slug = "func";

private:
  static constexpr auto hp = 3;
};

class Func6Panel : public Panel<Func6Panel> {
public:
  explicit Func6Panel(Func6 *func6) : Panel{func6, hp} {
    auto widget_right_edge = width();

    auto column_3 = widget_right_edge / 2.f;
    auto column_1 = widget_right_edge / 7.f;
    auto column_5 = widget_right_edge - column_1;
    auto column_2 = (column_3 - column_1) / 2.f + column_1;
    auto column_4 = widget_right_edge - column_2;

    auto top = 23.f;
    auto bottom = 108.f;
    auto row_count = 6;
    auto row_spacing = (bottom - top) / (row_count - 1);
    auto port_offset = 1.25f;

    for (auto row = 0; row < row_count; row++) {
      auto y = top + row * row_spacing;
      auto port_y = y + port_offset;

      auto channel = &func6->channels[row];
      auto operator_switch_index = Func6::OPERATOR_SWITCH + row;
      auto addition_range_switch_index = Func6::ADDITION_RANGE_SWITCH + row;
      auto multiplication_range_switch_index =
          Func6::MULTIPLICATION_RANGE_SWITCH + row;

      auto multiplication_range_selector = [channel](int selection) {
        channel->set_multiplication_range(selection);
      };
      auto multiplication_range_switch = toggle<MultiplicationRangeSwitch>(
          multiplication_range_switch_index, 2, multiplication_range_selector);

      auto addition_range_selector = [channel](int selection) {
        channel->set_addition_range(selection);
      };
      auto addition_range_switch = toggle<AdditionRangeSwitch>(
          addition_range_switch_index, 1, addition_range_selector);

      auto select_operator = [channel, addition_range_switch,
                              multiplication_range_switch](int position) {
        auto is_multiplication = position == 1;
        channel->set_operator(is_multiplication);
        multiplication_range_switch->visible = is_multiplication;
        addition_range_switch->visible = !is_multiplication;
      };

      auto operator_switch =
          toggle<2>(operator_switch_index, 0, select_operator);

      install(column_1, port_y, input(Func6::IN + row));
      install(column_2, y, operator_switch);
      install(column_3, y, knob<LargeKnob>(Func6::KNOB + row));
      install(column_4, y, multiplication_range_switch);
      install(column_4, y, addition_range_switch);
      install(column_5, port_y, output(Func6::OUT + row));
    }
  }

  static constexpr auto module_slug = "func-6";

private:
  static constexpr auto hp = 12;
};

} // namespace DHE

RACK_PLUGIN_MODEL_INIT(DHE_Modules, Func) {
   rack::Model *modelFunc = rack::Model::create<DHE::Func, DHE::FuncPanel>(
      "DHE-Modules", "Func", "Func", rack::UTILITY_TAG);
   return modelFunc;
}

RACK_PLUGIN_MODEL_INIT(DHE_Modules, Func6) {
   rack::Model *modelFunc6 = rack::Model::create<DHE::Func6, DHE::Func6Panel>(
      "DHE-Modules", "Func6", "Func 6", rack::UTILITY_TAG);
   return modelFunc6;
}
