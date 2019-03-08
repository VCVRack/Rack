#include "rack.hpp"

using namespace rack;

namespace rack_plugin_rcm {

namespace SongRoll {

  class SongRollData;

  class RollArea : public VirtualWidget {
  public:
    SongRollData& data;
    RollArea(Rect box, SongRollData& data);
  };
}

} // namespace rack_plugin_rcm
