#include "../../include/SongRoll/PatternControlWidget.hpp"
#include "../../include/Consts.hpp"

namespace rack_plugin_rcm {

namespace SongRoll {

  PatternControlWidget::PatternControlWidget() {} //int pattern) : pattern(pattern) {}

  void PatternControlWidget::draw(NVGcontext* ctx) {
    float y = 1;

    nvgBeginPath(ctx);
    nvgFontSize(ctx, 10);
    nvgTextAlign(ctx, NVG_ALIGN_CENTER + NVG_ALIGN_TOP);
    nvgFillColor(ctx, NV_YELLOW);
    nvgText(ctx, box.size.x * 0.5, y, "-- Pattern --", NULL);

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

    char patternBuffer[100];
    sprintf(patternBuffer, "%d", pattern + 1);

    nvgBeginPath(ctx);
    nvgFontSize(ctx, 14);
    nvgTextAlign(ctx, NVG_ALIGN_CENTER + NVG_ALIGN_TOP);
    nvgFillColor(ctx, NV_YELLOW);
    nvgText(ctx, box.size.x * 0.5, y, patternBuffer, NULL);

    y += 14;

    box.size.y = y;
  }

  void PatternControlWidget::onMouseDown(EventMouseDown& e) {
    Rect minus(Vec(0,10), Vec(box.size.x*0.40,14));
    Rect plus(Vec(box.size.x*0.60,10), Vec(box.size.x*0.40,14));

    if (minus.contains(e.pos)) {
      pattern = clamp(pattern - 1, 0, 63);
    } else if (plus.contains(e.pos)) {
      pattern = clamp(pattern + 1, 0, 63);
    }
  }

}

} // namespace rack_plugin_rcm
