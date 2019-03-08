#include "rack.hpp"

using namespace rack;

namespace rack_plugin_rcm {

namespace SongRoll {

  struct SongRollData;

  class PatternControlWidget : public VirtualWidget {
  public:
    int pattern=0;

    PatternControlWidget();

    void draw(NVGcontext* ctx) override;
    void onMouseDown(EventMouseDown& e) override;
  };

}

} // namespace rack_plugin_rcm
