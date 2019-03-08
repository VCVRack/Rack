#include <vector>

namespace rack_plugin_rcm {

namespace SongRoll {


  class ChannelConfig {
  public:
    enum eRepeatMode {
      FREE,
      REPEATS,
      LIMIT
    };

    int pattern = 1;
    int repeats = 1;
    eRepeatMode repeat_mode = eRepeatMode::FREE;
    int clock_div = 1;
  };

  class Section {
  public:
    std::vector<ChannelConfig> channels;
    Section();
  };

  class SongRollData {
  public:
    std::vector<Section> sections;
    SongRollData();
    void reset();
  };

}

} // namespace rack_plugin_rcm
