#include "rack.hpp"
#include "window.hpp"

using namespace rack;

namespace rack_plugin_ZZC {

#ifndef KNOB_SENSITIVITY_CONST
#define KNOB_SENSITIVITY_CONST
static const float KNOB_SENSITIVITY = 0.0015f;
#endif

struct ZZC_DirectKnobDisplay : TransparentWidget {
  NVGcolor backdropColor = nvgRGB(0x63, 0x63, 0x55);
  NVGcolor valueColor = nvgRGB(0xff, 0xd4, 0x2a);
  NVGcolor posColor = nvgRGB(0x9c, 0xd7, 0x43);
  NVGcolor negColor = nvgRGB(0xe7, 0x34, 0x2d);

  float *value = nullptr;
  float drawnValue = 0.0f;
  double lastDrawnAt = 0.0;
  float minVal;
  float maxVal;
  float center = 0.75;
  float girth = 0.4;
  float strokeWidth = 3.0;
  float startFrom = center;
  float range = girth;
  bool colored = false;
  float visibleDelta = 0.0f;
  float defaultValue = 0.0f;

  void setLimits(float low, float high) {
    this->visibleDelta = (high - low) / 50.0f;
    minVal = low;
    maxVal = high;
    startFrom = minVal == 0.0 ? center - girth : center;
    range = minVal == 0.0 ? girth * 2.0 : girth;
  }

  void enableColor() {
    colored = true;
  }

  void draw(NVGcontext *vg) override {
    if (!value) {
      return;
    }
    lastDrawnAt = (lglw_time_get_millisec(rack::global_ui->window.lglw) / 1000.0);
    drawnValue = *value;
    // return;
    nvgLineCap(vg, NSVG_CAP_ROUND);
    nvgStrokeWidth(vg, strokeWidth);

    nvgStrokeColor(vg, backdropColor);
    nvgBeginPath(vg);
    nvgArc(
      vg,
      box.size.x / 2.0, box.size.y / 2.0, box.size.x / 2.0 - strokeWidth / 2.0,
      (center + girth) * 2 * M_PI,
      (center - girth) * 2 * M_PI,
      1
    );
    nvgStroke(vg);

    if (*value == 0.0) {
      return;
    }
    if (colored) {
      nvgStrokeColor(vg, *value > 0.0 ? posColor : negColor);
    } else {
      nvgStrokeColor(vg, valueColor);
    }
    nvgBeginPath(vg);
    nvgArc(
      vg,
      box.size.x / 2.0, box.size.y / 2.0, box.size.x / 2.0 - strokeWidth / 2.0,
      startFrom * 2 * M_PI,
      (startFrom + (*value / maxVal) * range) * 2 * M_PI,
      *value >= 0 ? 2 : 1
    );
    nvgStroke(vg);
  }

  bool shouldUpdate(float *newValue) {
    if (*newValue == this->drawnValue) { return false; }
    if ((lglw_time_get_millisec(rack::global_ui->window.lglw) / 1000.0) - this->lastDrawnAt < 0.016) { return false; } // ~60FPS
    return fabsf(*newValue - this->drawnValue) > this->visibleDelta || *newValue == this->defaultValue;
  }
};

struct CachingTransformWidget : TransformWidget {
};

struct ZZC_CallbackKnob : ParamWidget, FramebufferWidget {
  SVGWidget *sw = nullptr;
  CachingTransformWidget *tw = nullptr;
  CircularShadow *shadow = nullptr;
  ZZC_DirectKnobDisplay *disp = nullptr;
  float *value = nullptr;
  bool randomizable = true;
  float speed = 2.0;
  float rotation = 0.0f;
  float lastRotation = 0.0f;
  float minAngle = -1.0 * M_PI;
  float maxAngle = 1.0 * M_PI;
  float strokeWidth = 3.0;
  float padding = strokeWidth + 2.0;
  float rotationMult = 1.0;
  float deltaMult = 1.0f;

  ZZC_CallbackKnob() {
  }

  void attachValue(float *valuePointer, float limitLow, float limitHigh, float defaultValue) {
    value = valuePointer;
    deltaMult = (limitHigh - limitLow) * 0.5f;
    if (disp) {
      disp->value = value;
      disp->setLimits(limitLow, limitHigh);
      disp->defaultValue = defaultValue;
    }
  }

  void setSVG(std::shared_ptr<SVG> svg, bool showDisplay) {
    if (showDisplay) {
      disp = new ZZC_DirectKnobDisplay();
      addChild(disp);
      disp->box.pos = Vec(0, 0);
    }

    shadow = new CircularShadow();
    addChild(shadow);
    shadow->box.size = Vec();

    tw = new CachingTransformWidget();
    addChild(tw);

    sw = new SVGWidget();
    tw->addChild(sw);

    padding = strokeWidth + 2.0;

    if (disp) {
      disp->strokeWidth = strokeWidth;
    }

    sw->setSVG(svg);
    sw->box.pos = showDisplay ? Vec(padding, padding) : Vec(0, 0);
    tw->box.size = sw->box.size;
    box.size = showDisplay ? Vec(padding * 2, padding * 2).plus(sw->box.size) : sw->box.size;
    shadow->box.size = sw->box.size;
    shadow->box.pos = showDisplay ? Vec(padding, padding).plus(Vec(0, sw->box.size.y * 0.2)) : Vec(0, sw->box.size.y * 0.2);
    if (disp) {
      disp->box.size = box.size;
    }
  }

  void onDragStart(EventDragStart &e) override {
    windowCursorLock();
    randomizable = false;
  }

  virtual void onInput(float factor) = 0;
  virtual void onReset() = 0;

  void onDragMove(EventDragMove &e) override {
    float delta = KNOB_SENSITIVITY * -e.mouseRel.y * speed;
    if (windowIsModPressed()) { delta /= 16.f; }
    rotation += delta * rotationMult;
    this->onInput(delta * deltaMult);
    dirty = true;
  }

  void onDragEnd(EventDragEnd &e) override {
    windowCursorUnlock();
    randomizable = true;
  }

  void reset() override {
    this->onReset();
    this->dirty = true;
  }

  void step() override {
    if (disp && value && disp->shouldUpdate(value)) {
      dirty = true;
    }
    if (dirty && (rotation != lastRotation)) {
      float angle;
      angle = rescale(rotation, -1.0, 1.0, minAngle, maxAngle);
      angle = fmodf(angle, 2*M_PI);
      tw->identity();
      // Rotate SVG
      Vec center = sw->box.getCenter();
      tw->translate(center);
      tw->rotate(angle);
      tw->translate(center.neg());
      lastRotation = rotation;
    }
    FramebufferWidget::step();
  }

  void enableColor() {
    if (disp) {
      disp->enableColor();
    }
  }

  void draw(NVGcontext *vg) override {
    // Bypass framebuffer rendering entirely
    Widget::draw(vg);
  }
};

} // namespace rack_plugin_ZZC
