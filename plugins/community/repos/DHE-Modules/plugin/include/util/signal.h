#pragma once

#include "range.h"
#include "selector.h"

namespace DHE {
namespace Signal {
static constexpr auto unipolar_range = Range{0.f, 10.f};
static constexpr auto bipolar_range = Range{-5.f, 5.f};

static const auto ranges =
    std::vector<Range const *>{&bipolar_range, &unipolar_range};

inline auto range(bool is_uni) -> const Range & {
  return is_uni ? unipolar_range : bipolar_range;
}
} // namespace Signal
} // namespace DHE
