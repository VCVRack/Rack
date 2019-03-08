#include "rack.hpp"
using namespace rack;

namespace rack_plugin_rcm {

namespace SongRoll {

  struct SongRollData;
  struct PatternHeaderWidget;
  struct PatternControlWidget;
  struct RepeatControlWidget;
  struct ClockDivControlWidget;

  class PatternControllerSlice : public VirtualWidget {
  public:
    const int channel;
    SongRollData& data;
    SequentialLayout* layout;
    PatternHeaderWidget* header;
    PatternControlWidget* pattern;
    RepeatControlWidget* repeats;
    ClockDivControlWidget* clock_div;

    PatternControllerSlice(int channel, SongRollData& data, int section);
    void draw(NVGcontext* ctx) override;
    void step() override;
    void onMouseDown(EventMouseDown& e) override;
    void setSection(int section);

  private:
      int section = 0;
      bool sectionChanged = true;
  };

}

} // namespace rack_plugin_rcm
