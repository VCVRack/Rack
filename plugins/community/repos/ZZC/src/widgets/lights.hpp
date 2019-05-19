#include "rack.hpp"

using namespace rack;

namespace rack_plugin_ZZC {

static const NVGcolor COLOR_ZZC_YELLOW = nvgRGB(0xff, 0xd4, 0x2a);
static const NVGcolor COLOR_NEG = nvgRGB(0xe7, 0x34, 0x2d);

template <typename BASE>
struct LedLight : BASE {
  LedLight() {
    this->box.size = mm2px(Vec(6.3f, 6.3f));
  }
};


struct ZZC_BaseLight : GrayModuleLightWidget {
  float values[2] = { 0.0f, 0.0f };
  double lastStepAt = 0.0;

  ZZC_BaseLight() {
  }
  void drawHalo(NVGcontext *vg) override {
    float radius = box.size.x / 2.0;
    float oradius = radius + 15.0;

    nvgBeginPath(vg);
    nvgRect(vg, radius - oradius, radius - oradius, 2*oradius, 2*oradius);

    NVGpaint paint;
    NVGcolor icol = colorMult(color, 0.04);
    NVGcolor ocol = nvgRGB(0, 0, 0);
    paint = nvgRadialGradient(vg, radius, radius, radius, oradius, icol, ocol);
    nvgFillPaint(vg, paint);
    nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
    nvgFill(vg);
  }
  void step() override {
    assert(module);
    assert(module->lights.size() >= firstLightId + baseColors.size());

    double now = (lglw_time_get_millisec(rack::global_ui->window.lglw) / 1000.0);
    double timeDelta = now - lastStepAt;
    for (size_t i = 0; i < baseColors.size(); i++) {
      float value = module->lights[firstLightId + i].value;
      if (value == 1.0f) {
        values[i] = 1.0f;
      } else if (value == 1.1f) {
        module->lights[firstLightId + i].value = 0.0f;
        values[i] = 1.0f;
      } else if (value > 0.0f) {
        values[i] = value;
      } else if (values[i] > 0.0f){
        values[i] = fmaxf(0.0f, values[i] - values[i] * 8.0f * timeDelta);
      }
    }
    lastStepAt = now;
    setValues(values);
  }
  void setValues(float values[2]) {
    color = nvgRGBAf(0, 0, 0, 0);
    for (size_t i = 0; i < baseColors.size(); i++) {
      NVGcolor c = baseColors[i];
      c.a *= clamp(values[i], 0.f, 1.f);
      color = colorScreen(color, c);
    }
    color = colorClip(color);
  }
};

struct ZZC_YellowLight : ZZC_BaseLight {
  ZZC_YellowLight() {
    addBaseColor(COLOR_ZZC_YELLOW);
  }
};

struct ZZC_RedLight : ZZC_BaseLight {
  ZZC_RedLight() {
    addBaseColor(COLOR_NEG);
  }
};

} // namespace rack_plugin_ZZC
