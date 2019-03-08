
namespace rack_plugin_rcm {

namespace SongRoll {

  class SongRollData;

  class Transport {
  public:
    Transport(SongRollData* data);
    void reset();

    int currentSection = 0;

  private:
    SongRollData* data;
  };

}

} // namespace rack_plugin_rcm
