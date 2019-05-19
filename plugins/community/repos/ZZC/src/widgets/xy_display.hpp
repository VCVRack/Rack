#include "rack.hpp"
#include "window.hpp"

#ifndef DISPLAYS_H
#define DISPLAYS_H
#include "displays.hpp"
#endif

using namespace rack;

namespace rack_plugin_ZZC {

#ifndef KNOB_SENSITIVITY_CONST
#define KNOB_SENSITIVITY_CONST
static const float KNOB_SENSITIVITY = 0.0015f;
#endif

struct XYDisplayViewWidget : BaseDisplayWidget {
  float *x = nullptr;
  float *y = nullptr;
  float drawnX = 0.0f;
  float drawnY = 0.0f;
  float padding = 2.0f;
  double lastDrawnAt = 0.0;

  NVGcolor lcdGhostColor = nvgRGB(0x1e, 0x1f, 0x1d);
  NVGcolor posColor = nvgRGB(0x9c, 0xd7, 0x43);
  NVGcolor negColor = nvgRGB(0xe7, 0x34, 0x2d);

  float scaleValue(float value, float area) {
    return padding + ((value + 1.0f) / 2.0f) * (area - padding * 2.0f);
  }

  void drawCross(NVGcontext *vg, Vec position) {
    if (*y == 0.0f) {
      nvgStrokeColor(vg, lcdGhostColor);
    } else if (*y < 0.0f) {
      nvgStrokeColor(vg, negColor);
    } else {
      nvgStrokeColor(vg, posColor);
    }
    nvgStrokeWidth(vg, 1.0f);

    nvgBeginPath(vg);
    nvgMoveTo(vg, position.x - 5.5f, position.y);
    nvgLineTo(vg, position.x + 5.5f, position.y);
    nvgStroke(vg);

    nvgBeginPath(vg);
    nvgMoveTo(vg, position.x, position.y - 5.5f);
    nvgLineTo(vg, position.x, position.y + 5.5f);
    nvgStroke(vg);
  }

  void draw(NVGcontext *vg) override {
    drawBackground(vg);
    nvgScissor(vg, padding, padding, box.size.x - padding * 2.0f, box.size.y - padding * 2.0f);
    drawCross(vg, Vec(
      scaleValue(x ? *x : 0.0f, box.size.x),
      scaleValue(y ? -*y : 0.0f, box.size.y)
    ));
    nvgResetScissor(vg);
    this->drawnX = *x;
    this->drawnY = *y;
    this->lastDrawnAt = (lglw_time_get_millisec(rack::global_ui->window.lglw) / 1000.0);
  }

  bool shouldUpdate(float *xPtr, float *yPtr) {
    if ((lglw_time_get_millisec(rack::global_ui->window.lglw) / 1000.0) - this->lastDrawnAt < 0.016) { return false; } // ~60 FPS
    if (*xPtr == this->drawnX && *yPtr == this->drawnY) { return false; }
    if (*xPtr == 0.0f || *yPtr == 0.0f) { return true; }
    return fabsf(this->drawnX - *xPtr) > 0.05 || fabsf(this->drawnY - *yPtr) > 0.05;
  }
};

struct XYDisplayWidget : ParamWidget, FramebufferWidget {
  float *x = nullptr;
  float *y = nullptr;
  float lastX = 0.0f;
  float lastY = 0.0f;
  float dragDelta;
  XYDisplayViewWidget *disp;
  float speed = 4.0;

  XYDisplayWidget() {
    disp = new XYDisplayViewWidget();
  }

  void setupSize() {
    disp->box.pos = Vec(0, 0);
    disp->box.size = this->box.size;
    addChild(disp);
  }

  void setupPtrs() {
    this->disp->x = this->x;
    this->disp->y = this->y;
  }

  void onDragStart(EventDragStart &e) override {
    windowCursorLock();
    dragDelta = 0.0;
  }

  virtual void onInput(float x, float y) = 0;
  virtual void onReset() = 0;

  void onDragMove(EventDragMove &e) override {
    float deltaX = KNOB_SENSITIVITY * e.mouseRel.x * speed;
    float deltaY = KNOB_SENSITIVITY * -e.mouseRel.y * speed;

    if (windowIsModPressed()) {
      deltaX /= 16.f;
      deltaY /= 16.f;
    }

    onInput(deltaX, deltaY);

    dirty = true;
  }

  void onDragEnd(EventDragEnd &e) override {
    windowCursorUnlock();
  }
  void reset() override {
    this->onReset();
    this->dirty = true;
  }

  void step() override {
    if (x && y && this->disp->shouldUpdate(x, y)) {
      dirty = true;
      lastX = *x;
      lastY = *y;
    }
    FramebufferWidget::step();
  }

  void draw(NVGcontext *vg) override {
    // Bypass framebuffer rendering entirely
    Widget::draw(vg);
  }
};

} // namespace rack_plugin_ZZC
