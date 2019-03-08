#include "rack.hpp"

using namespace rack;

namespace rack_plugin_rcm {

namespace SongRoll {

  class PatternHeaderWidget : public VirtualWidget {
  public:
    int repeats;
    int repeats_completed;
    int pattern;
    bool active;

    PatternHeaderWidget(int repeats, int repeats_completed, int pattern);

    void draw(NVGcontext* ctx) override;
  };

}

} // namespace rack_plugin_rcm
