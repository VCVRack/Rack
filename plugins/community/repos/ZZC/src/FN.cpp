#include "ZZC.hpp"
// // #include "window.hpp"

namespace rack_plugin_ZZC {

const float M_PI_X2 = M_PI * 2.0f;

inline float fn3Sin(float phase) {
  return (sinf(phase * M_PI_X2 - M_PI_2) + 1.0f) * 0.5f;
}

inline float fn3Sqr(float phase) {
  return phase < 0.5f ? 1.0f : 0.0f;
}

inline float fn3Tri(float phase) {
  return phase < 0.5f ? phase * 2.0f : (1.0f - phase) * 2.0f;
}

inline float applyPW(float phase, float pw) {
  if (pw == 0.0f) {
    return 0.5f + phase / 2.0f;
  }
  if (phase > pw) {
    return (phase - pw) / (1.0f - pw) / 2.0f + 0.5f;
  } else {
    return phase / pw / 2.0f;
  }
}

struct FN3TextDisplayWidget : TransparentWidget {
  float *hook;
  float prevHook = 0.0f;
  float *text;
  bool displayText = false;
  double textUpdatedAt = 0.0;

  std::shared_ptr<Font> font;

  FN3TextDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/fonts/Nunito/Nunito-Black.ttf"));
  };

  void draw(NVGcontext *vg) override {
    NVGcolor lcdColor = nvgRGB(0x12, 0x12, 0x12);
    NVGcolor lcdTextColor = nvgRGB(0xff, 0xd4, 0x2a);

    if (prevHook == *hook) {
      double curTime = (lglw_time_get_millisec(rack::global_ui->window.lglw) / 1000.0);
      if ((curTime - textUpdatedAt) > 2.0) {
        return;
      }
    } else {
      textUpdatedAt = (lglw_time_get_millisec(rack::global_ui->window.lglw) / 1000.0);
      prevHook = *hook;
    }

    // LCD
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 0.0);
    nvgFillColor(vg, lcdColor);
    nvgFill(vg);

    char textString[10];
    snprintf(textString, sizeof(textString), "%3.1f", *text > 0.04 ? *text - 0.04 : *text + 0.04);

    // Text
    nvgFontSize(vg, 8.5);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 0.1);
    nvgTextAlign(vg, NVG_ALIGN_CENTER);
    Vec textPos = Vec(box.size.x / 2.0f, box.size.y * 0.7f);
    nvgFillColor(vg, lcdTextColor);
    nvgText(vg, textPos.x, textPos.y, textString, NULL);
  }

};

struct FN3DisplayWidget : BaseDisplayWidget {
  float *wave;
  float *pw;
  float *shift;

  void draw(NVGcontext *vg) override {
    drawBackground(vg);

    NVGcolor graphColor = nvgRGB(0xff, 0xd4, 0x2a);

    nvgBeginPath(vg);
    float firstCoord = true;
    for (float i = 0.0f; i < 1.00f; i = i + 0.01f) {
      float x, y, value, phase;
      value = 0.0f;
      x = 2.0f + (box.size.x - 4.0f) * i;
      phase = applyPW(eucmod(i + *shift, 1.0f), *pw);
      if (*wave == 0.0f) {
        value = fn3Sin(phase);
      } else if (*wave == 1.0f) {
        value = fn3Tri(phase);
      } else if (*wave == 2.0f) {
        value = fn3Sqr(phase);
      }
      y = (1.0f - value) * (box.size.y / 4.0f) + (0.375f * box.size.y);

      if (firstCoord) {
        nvgMoveTo(vg, x, y);
        firstCoord = false;
        continue;
      }
      nvgLineTo(vg, x, y);
    }

    nvgStrokeColor(vg, graphColor);
    nvgLineCap(vg, NVG_ROUND);
    nvgMiterLimit(vg, 2.0f);
    nvgStrokeWidth(vg, 1.0f);
    nvgStroke(vg);
  }
};


struct FN3 : Module {
  enum ParamIds {
    PW_PARAM,
    WAVE_PARAM,
    OFFSET_PARAM,
    SHIFT_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    PW_INPUT,
    SHIFT_INPUT,
    PHASE_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    WAVE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  float phase;
  float pw;
  float shift;
  float wave;

  float pwParam;
  float lastPwParam;
  float pwDisplay = 50.0f;

  float shiftParam;
  float lastShiftParam;
  float shiftDisplay;

  inline float snap(float value) {
    if (value > 0.33f && value < 0.34f) {
      return 1.0f / 3.0f;
    } else if (value > 0.66f && value < 0.67f) {
      return 1.0f / 1.5f;
    } else if (value < -0.33f && value > -0.34f) {
      return -1.0f / 3.0f;
    } else if (value < -0.66f && value > -0.67f) {
      return -1.0f / 1.5f;
    } else {
      return roundf(value * 100.0f) / 100.0f;
    }
  }

  FN3() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
  }
  void step() override;
};


void FN3::step() {
  if (params[PW_PARAM].value != lastPwParam) {
    pwParam = snap(params[PW_PARAM].value);
    lastPwParam = params[PW_PARAM].value;
  }
  pw = clamp(pwParam + (inputs[PW_INPUT].active ? inputs[PW_INPUT].value / 10.0f : 0.0f), 0.0f, 1.0f);
  pwDisplay = pw * 100.0f;

  if (params[SHIFT_PARAM].value != lastShiftParam) {
    shiftParam = snap(params[SHIFT_PARAM].value);
    lastShiftParam = params[SHIFT_PARAM].value;
  }
  shift = shiftParam + (inputs[SHIFT_INPUT].active ? inputs[SHIFT_INPUT].value / -5.0f : 0.0f);
  shiftDisplay = shift * -100.0f;

  phase = applyPW(eucmod((inputs[PHASE_INPUT].active ? inputs[PHASE_INPUT].value / 10.0f : 0.0f) + shift, 1.0f), pw);
  wave = params[WAVE_PARAM].value;

  if (wave == 0.0f) {
    outputs[WAVE_OUTPUT].value = fn3Sin(phase) * 10.0f - (params[OFFSET_PARAM].value == 1.0f ? 5.0f : 0.0f);
  } else if (wave == 1.0f) {
    outputs[WAVE_OUTPUT].value = fn3Tri(phase) * 10.0f - (params[OFFSET_PARAM].value == 1.0f ? 5.0f : 0.0f);
  } else {
    outputs[WAVE_OUTPUT].value = fn3Sqr(phase) * 10.0f - (params[OFFSET_PARAM].value == 1.0f ? 5.0f : 0.0f);
  }
}


struct FN3Widget : ModuleWidget {
  FN3Widget(FN3 *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/panels/FN-3.svg")));

    addParam(ParamWidget::create<ZZC_Knob27>(Vec(9, 58), module, FN3::PW_PARAM, 0.0f, 1.0f, 0.5f));
    addInput(Port::create<ZZC_PJ_Port>(Vec(10.5, 93), Port::INPUT, module, FN3::PW_INPUT));

    FN3DisplayWidget *display = new FN3DisplayWidget();
    display->box.pos = Vec(8.0f, 126.0f);
    display->box.size = Vec(29.0f, 49.0f);
    display->wave = &module->wave;
    display->pw = &module->pw;
    display->shift = &module->shift;
    addChild(display);
    addParam(ParamWidget::create<ZZC_FN3WaveSwitch>(Vec(8, 126), module, FN3::WAVE_PARAM, 0.0f, 2.0f, 0.0f));
    addParam(ParamWidget::create<ZZC_FN3UniBiSwitch>(Vec(8, 152), module, FN3::OFFSET_PARAM, 0.0f, 1.0f, 0.0f));

    FN3TextDisplayWidget *pwDisplay = new FN3TextDisplayWidget();
    pwDisplay->box.pos = Vec(11.0f, 129.0f);
    pwDisplay->box.size = Vec(23.0f, 13.0f);
    pwDisplay->hook = &module->pwParam;
    pwDisplay->text = &module->pwDisplay;
    addChild(pwDisplay);

    FN3TextDisplayWidget *shiftDisplay = new FN3TextDisplayWidget();
    shiftDisplay->box.pos = Vec(11.0f, 129.0f);
    shiftDisplay->box.size = Vec(23.0f, 13.0f);
    shiftDisplay->hook = &module->shiftParam;
    shiftDisplay->text = &module->shiftDisplay;
    addChild(shiftDisplay);

    addInput(Port::create<ZZC_PJ_Port>(Vec(10.5, 194), Port::INPUT, module, FN3::SHIFT_INPUT));
    addParam(ParamWidget::create<ZZC_Knob25>(Vec(10, 229), module, FN3::SHIFT_PARAM, 1.0f, -1.0f, 0.0f));

    addInput(Port::create<ZZC_PJ_Port>(Vec(10.5, 275), Port::INPUT, module, FN3::PHASE_INPUT));
    addOutput(Port::create<ZZC_PJ_Port>(Vec(10.5, 320), Port::OUTPUT, module, FN3::WAVE_OUTPUT));

    addChild(Widget::create<ZZC_Screw>(Vec(box.size.x / 2 - RACK_GRID_WIDTH / 2, 0)));
    addChild(Widget::create<ZZC_Screw>(Vec(box.size.x / 2 - RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  }
};

} // namespace rack_plugin_ZZC

using namespace rack_plugin_ZZC;

RACK_PLUGIN_MODEL_INIT(ZZC, FN3) {
   Model *modelFN3 = Model::create<FN3, FN3Widget>("ZZC", "FN-3", "FN-3 Function Generator", FUNCTION_GENERATOR_TAG);
   return modelFN3;
}
