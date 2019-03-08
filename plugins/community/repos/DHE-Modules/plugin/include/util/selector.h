#pragma once

#include <utility>

#include "util/range.h"

namespace DHE {
template <typename T> class Selector {
public:
  Selector(std::vector<T> choices, std::function<void(T)> on_selection)
      : choices{std::move(choices)}, notify{std::move(on_selection)} {}

  /**
   * Send notice that the choice at the given position has been selected.
   */
  void operator()(int position) { notify(choices[position]); }

private:
  const std::vector<T> choices;
  const std::function<void(T)> notify;
};
} // namespace DHE
