#include "../../include/SongRoll/RepeatControlWidget.hpp"
#include "../../include/SongRoll/SongRollData.hpp"
#include "../../include/Consts.hpp"

namespace rack_plugin_rcm {

namespace SongRoll {

  RepeatControlWidget::RepeatControlWidget() {}

  void RepeatControlWidget::draw(NVGcontext* ctx) {
    float y = 1;

    nvgBeginPath(ctx);
    nvgFontSize(ctx, 10);
    nvgTextAlign(ctx, NVG_ALIGN_CENTER + NVG_ALIGN_TOP);
    nvgFillColor(ctx, NV_YELLOW);
    nvgText(ctx, box.size.x * 0.5, y, "-- Repeats --", NULL);

    y += 10;

    nvgBeginPath(ctx);
    nvgFontSize(ctx, 14);
    nvgTextAlign(ctx, NVG_ALIGN_CENTER + NVG_ALIGN_TOP);
    nvgFillColor(ctx, NV_YELLOW);
    nvgText(ctx, box.size.x * 0.25, y, "−", NULL);

    nvgBeginPath(ctx);
    nvgFontSize(ctx, 14);
    nvgTextAlign(ctx, NVG_ALIGN_CENTER + NVG_ALIGN_TOP);
    nvgFillColor(ctx, NV_YELLOW);
    nvgText(ctx, box.size.x * 0.75, y, "+", NULL);

    char repeatBuffer[100];
    sprintf(repeatBuffer, "%d", repeats);

    nvgBeginPath(ctx);
    nvgFontSize(ctx, 14);
    nvgTextAlign(ctx, NVG_ALIGN_CENTER + NVG_ALIGN_TOP);
    nvgFillColor(ctx, NV_YELLOW);
    nvgText(ctx, box.size.x * 0.5, y, repeatBuffer, NULL);

    y += 16;

    nvgBeginPath(ctx);
    nvgFontSize(ctx, 12);
    nvgTextAlign(ctx, NVG_ALIGN_CENTER + NVG_ALIGN_TOP);
    nvgFillColor(ctx, NV_YELLOW);
    switch (repeat_mode) {
      case ChannelConfig::FREE:
        nvgText(ctx, box.size.x * 0.5, y, "no limit", NULL);
        break;
      case ChannelConfig::REPEATS:
        nvgText(ctx, box.size.x * 0.5, y, "at least", NULL);
        break;
      case ChannelConfig::LIMIT:
        nvgText(ctx, box.size.x * 0.5, y, "at most", NULL);
        break;
    }

    nvgBeginPath(ctx);
    nvgStrokeColor(ctx, NV_YELLOW_H);
    nvgRoundedRect(ctx, 8, y, box.size.x - 16, 12, 2);
    nvgStroke(ctx);

    y += 14;
    box.size.y = y;
  }

  void RepeatControlWidget::onMouseDown(EventMouseDown& e) {
    Rect minus(Vec(0,10), Vec(box.size.x*0.40,14));
    Rect plus(Vec(box.size.x*0.60,10), Vec(box.size.x*0.40,14));
    Rect mode(Vec(0, 26), Vec(box.size.x, 26));

    if (minus.contains(e.pos)) {
      repeats = clamp(repeats - 1, 1, 64);
    } else if (plus.contains(e.pos)) {
      repeats = clamp(repeats + 1, 1, 64);
    } else if (mode.contains(e.pos)) {
      repeat_mode = (repeat_mode + 1) % 3;
    }
  }

}

} // namespace rack_plugin_rcm
