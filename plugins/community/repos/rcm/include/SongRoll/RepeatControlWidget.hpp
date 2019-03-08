#include "rack.hpp"

using namespace rack;

namespace rack_plugin_rcm {

namespace SongRoll {

  struct SongRollData;

  class RepeatControlWidget : public VirtualWidget {
  public:
    int repeats=1;
    int repeats_complete=0;
    int repeat_mode=1;

    RepeatControlWidget();

    void draw(NVGcontext* ctx) override;
    void onMouseDown(EventMouseDown& e) override;
  };

}

} // namespace rack_plugin_rcm
