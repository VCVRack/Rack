#include "../../include/SongRoll/ClockDivControlWidget.hpp"
#include "../../include/Consts.hpp"

namespace rack_plugin_rcm {

namespace SongRoll {

  ClockDivControlWidget::ClockDivControlWidget() {}

  void ClockDivControlWidget::draw(NVGcontext* ctx) {
    float y = 1;

    nvgBeginPath(ctx);
    nvgFontSize(ctx, 10);
    nvgTextAlign(ctx, NVG_ALIGN_CENTER + NVG_ALIGN_TOP);
    nvgFillColor(ctx, NV_YELLOW);
    nvgText(ctx, box.size.x * 0.5, y, "-- Clock Div --", NULL);

    y += 10;

    float clockDivHeight = 48;
    float hcenter = box.size.x * 0.5;

    nvgBeginPath(ctx);
    nvgStrokeColor(ctx, NV_YELLOW_H);
    nvgStrokeWidth(ctx, 1.0f);
    nvgRoundedRect(ctx, 8, y, box.size.x - 16, clockDivHeight, 4);
    nvgStroke(ctx);

    nvgBeginPath(ctx);
    nvgStrokeColor(ctx, NV_YELLOW_H);
    nvgStrokeWidth(ctx, 2.0f);
    nvgFillColor(ctx, NV_BLACK);
    nvgRect(ctx, 9, y + (clockDivHeight / 2) - 10, box.size.x - 18, 20);
    nvgFill(ctx);

    nvgBeginPath(ctx);
    nvgStrokeColor(ctx, NV_YELLOW_H);
    nvgStrokeWidth(ctx, 1.0f);
    nvgMoveTo(ctx, 8, y + (clockDivHeight / 2) - 10);
    nvgLineTo(ctx, 8 + box.size.x - 16, y + (clockDivHeight / 2) - 10);
    nvgStroke(ctx);

    nvgBeginPath(ctx);
    nvgStrokeColor(ctx, NV_YELLOW_H);
    nvgStrokeWidth(ctx, 1.0f);
    nvgMoveTo(ctx, 8, y + (clockDivHeight / 2) - 10 + 20);
    nvgLineTo(ctx, 8 + box.size.x - 16, y + (clockDivHeight / 2) - 10 + 20);
    nvgStroke(ctx);

    nvgBeginPath(ctx);
    nvgStrokeColor(ctx, NV_YELLOW_H);
    nvgStrokeWidth(ctx, 3.0f);
    nvgMoveTo(ctx, hcenter - 8, y + 10);
    nvgLineTo(ctx, hcenter, y + 4);
    nvgLineTo(ctx, hcenter + 8, y + 10);
    nvgStroke(ctx);

    nvgBeginPath(ctx);
    nvgStrokeColor(ctx, NV_YELLOW_H);
    nvgStrokeWidth(ctx, 3.0f);
    nvgMoveTo(ctx, hcenter - 8, y + clockDivHeight - 10);
    nvgLineTo(ctx, hcenter, y + clockDivHeight - 4);
    nvgLineTo(ctx, hcenter + 8, y + clockDivHeight - 10);
    nvgStroke(ctx);


    char clockDivBuffer[100];
    if (clock_div > 0) {
      sprintf(clockDivBuffer, "/ %d", clock_div);
    } else {
      sprintf(clockDivBuffer, "off");
    }

    nvgBeginPath(ctx);
    nvgFontSize(ctx, 12);
    nvgTextAlign(ctx, NVG_ALIGN_CENTER + NVG_ALIGN_MIDDLE);
    nvgFillColor(ctx, NV_YELLOW);
    nvgText(ctx, hcenter, y + (clockDivHeight / 2), clockDivBuffer, NULL);

    y += clockDivHeight;

    box.size.y = y;
  }

  void ClockDivControlWidget::onMouseDown(EventMouseDown& e) {
    Rect minus(Vec(0,44), Vec(box.size.x,20));
    Rect plus(Vec(0,10), Vec(box.size.x,20));

    if (minus.contains(e.pos)) {
      clock_div = clamp(clock_div - 1, 0, 64);
    } else if (plus.contains(e.pos)) {
      clock_div = clamp(clock_div + 1, 0, 64);
    }
  }

}

} // namespace rack_plugin_rcm
