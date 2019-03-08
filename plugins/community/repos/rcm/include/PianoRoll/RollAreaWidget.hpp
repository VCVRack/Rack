#include "rack.hpp"

namespace rack_plugin_rcm {

class Auditioner;
class PatternData;
class Transport;
class PianoRollDragType;

struct Key {
  rack::Vec pos;
  rack::Vec size;
  bool sharp;
  int num;
  int oct;

  Key() : pos(rack::Vec(0,0)), size(rack::Vec(0,0)), sharp(false), num(0), oct(0) {}
  Key(rack::Vec p, rack::Vec s, bool sh, int n, int o) : pos(p), size(s), sharp(sh), num(n), oct(o) {}

  int pitch() {
    return num + (12 * oct);
  }
};

struct BeatDiv {
  rack::Vec pos;
  rack::Vec size;
  int num;
  bool beat;
  bool triplet;

  BeatDiv() : pos(rack::Vec(0,0)), size(rack::Vec(0,0)), num(0), beat(false), triplet(false) {}
};

struct WidgetState {
  rack::Rect box;
  int notesToShow = 18;
  int lowestDisplayNote = 4 * 12;
  int currentMeasure = 0;
  int lastDrawnStep = -1;
  float displayVelocityHigh = -1;
  float displayVelocityLow = -1;
  double measureLockPressTime = 0.f;

  bool dirty = true;
  bool consumeDirty();
};

class UnderlyingRollAreaWidget : public rack::VirtualWidget {
public:
  UnderlyingRollAreaWidget();
  ~UnderlyingRollAreaWidget();

  int fonthandle;

  WidgetState* state;
  PatternData* patternData;
  Transport* transport;
  Auditioner* auditioner;
  float topMargins = 15;

  std::vector<Key> getKeys(const rack::Rect& keysArea);
  std::vector<BeatDiv> getBeatDivs(const rack::Rect &roll);
  rack::Rect reserveKeysArea(rack::Rect& roll);
  std::tuple<bool, int> findMeasure(rack::Vec pos);
  std::tuple<bool, bool> findOctaveSwitch(rack::Vec pos);
  std::tuple<bool, BeatDiv, Key> findCell(rack::Vec pos);

  void drawKeys(NVGcontext *ctx, const std::vector<Key> &keys);
  void drawSwimLanes(NVGcontext *ctx, const rack::Rect &roll, const std::vector<Key> &keys);
  void drawBeats(NVGcontext *ctx, const std::vector<BeatDiv> &beatDivs);
  void drawNotes(NVGcontext *ctx, const std::vector<Key> &keys, const std::vector<BeatDiv> &beatDivs);
  void drawMeasures(NVGcontext *ctx);
  void drawPlayPosition(NVGcontext *ctx);
  void drawVelocityInfo(NVGcontext *ctx);

  void draw(NVGcontext* ctx) override;

	void onMouseDown(rack::EventMouseDown& e) override;
	void onDragStart(rack::EventDragStart& e) override;
  void onDragMove(rack::EventDragMove& e) override;
  void onDragEnd(rack::EventDragEnd& e) override;

  rack::Vec lastMouseDown;
  PianoRollDragType* currentDragType = NULL;
};

class RollAreaWidget : public rack::FramebufferWidget {
public:

  WidgetState state;
  UnderlyingRollAreaWidget* underlyingRollAreaWidget;

  RollAreaWidget(PatternData* patternData, Transport* transport, Auditioner* auditioner);

  void step() override;

private:
  PatternData* patternData;
  Transport* transport;
  Auditioner* auditioner;
};

} // namespace rack_plugin_rcm
