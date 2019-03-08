#include "../../include/SongRoll/PatternHeaderWidget.hpp"
#include "../../include/Consts.hpp"

namespace rack_plugin_rcm {

namespace SongRoll {
  PatternHeaderWidget::PatternHeaderWidget(int repeats, int repeats_completed, int pattern) : repeats(repeats), repeats_completed(repeats_completed), pattern(pattern), active(true) {}

  void drawSegmentedCircle(NVGcontext* ctx, float bottomGapRadians, int segmentCount, int litSegments, float strokeWidth, NVGcolor litColour, NVGcolor unlitColour, float x, float y, float radius) {
    float arcLength = TAU - bottomGapRadians;
    float arcSegmentLength = arcLength / segmentCount;
    float arcStart = (TAU * 0.25) + (bottomGapRadians / 2);
    float segmentGap = arcSegmentLength * (segmentCount > 16 ? 0.4 : segmentCount > 8 ? 0.2 : 0.1);
    for (float j = 0; j < segmentCount; j++) {
      nvgBeginPath(ctx);

      nvgStrokeWidth(ctx, strokeWidth);
      if (j < litSegments) {
        nvgStrokeColor(ctx, litColour);
      } else {
        nvgStrokeColor(ctx, unlitColour);
      }

      float segmentStart = arcStart + j * arcSegmentLength;
      float segmentEnd = arcStart + ((j+1) * arcSegmentLength);

      if (j != 0) {
        segmentStart += segmentGap / 2;
      }
      if (j != segmentCount-1) {
        segmentEnd -= segmentGap / 2;
      }
      nvgArc(ctx, x, y, radius, segmentStart, segmentEnd, NVG_CW);

      nvgStroke(ctx);
    }
  }

  void PatternHeaderWidget::draw(NVGcontext* ctx) {
    box.size.y = box.size.x * 0.7;

    Widget::draw(ctx);

    drawSegmentedCircle(ctx, TAU * 0.3, active ? repeats : 1, active ? repeats_completed : 0, 6.f, NV_YELLOW, NV_YELLOW_H, box.size.x * 0.5, box.size.x * 0.5, box.size.x * 0.5 * 0.7);

    char patternNumberBuffer[10];
    if (active) {
      sprintf(patternNumberBuffer, "%02d", pattern + 1);
    } else {
      sprintf(patternNumberBuffer, "−−");
    }
    nvgBeginPath(ctx);
    nvgFontSize(ctx, 24);
    nvgTextAlign(ctx, NVG_ALIGN_CENTER + NVG_ALIGN_MIDDLE);
    nvgFillColor(ctx, NV_YELLOW);
    nvgText(ctx, box.size.x * 0.5, box.size.x * 0.5, patternNumberBuffer, NULL);
  }
}

} // namespace rack_plugin_rcm
