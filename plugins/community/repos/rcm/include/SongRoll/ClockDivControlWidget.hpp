#include "rack.hpp"

using namespace rack;

namespace rack_plugin_rcm {

namespace SongRoll {

  struct SongRollData;

  class ClockDivControlWidget : public VirtualWidget {
  public:
    int clock_div=1;

    ClockDivControlWidget();

    void draw(NVGcontext* ctx) override;
    void onMouseDown(EventMouseDown& e) override;
  };

}

} // namespace rack_plugin_rcm
