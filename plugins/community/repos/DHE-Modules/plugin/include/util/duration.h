#pragma once

#include <vector>

#include "display/controls.h"
#include "range.h"
#include "selector.h"
#include "sigmoid.h"

namespace DHE {

namespace Duration {
static constexpr auto short_range = Range{0.001f, 1.f};
static constexpr auto medium_range = Range{0.01f, 10.f};
static constexpr auto long_range = Range{0.1f, 100.f};

static const auto ranges =
    std::vector<Range const *>{&short_range, &medium_range, &long_range};
} // namespace Duration

inline auto duration(float rotation, const Range &range) -> float {
  auto tapered = Sigmoid::j_taper(rotation, 0.8f);
  return range.scale(tapered);
} // namespace Duration

inline auto duration(float rotation) -> float {
  return duration(rotation, Duration::medium_range);
}
} // namespace DHE
