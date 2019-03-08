#include "../../include/SongRoll/RollArea.hpp"
#include "../../include/SongRoll/SongRollData.hpp"
#include "../../include/SongRoll/PatternControllerSlice.hpp"

namespace rack_plugin_rcm {

namespace SongRoll {

  RollArea::RollArea(Rect box, SongRollData& data) : data(data) {
    this->box = box;
    const int numChannels = (int)data.sections[0].channels.size();
    const float channelWidth = box.size.x / data.sections[0].channels.size();
    for (int i = 0; i < numChannels; i++) {
      auto *slice = new PatternControllerSlice(i, data, 0);
      slice->box.pos.x = (channelWidth * i);
      slice->box.pos.y = 0;
      slice->box.size.x = channelWidth;
      slice->box.size.y = box.size.y;

      addChild(slice);
    }
  }

  
}

} // namespace rack_plugin_rcm
