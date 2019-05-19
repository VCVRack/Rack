#include "ZZC.hpp"

namespace rack_plugin_ZZC {

enum ModeIds {
  MUSICAL_MODE,
  DECIMAL_MODE,
  FREE_MODE,
  NUM_MODES
};


void writeMusicalNotation(char *output, float voltage) {
  char notes[20][20] = {"c", "ic", "d", "id", "e", "f", "if", "g", "ig", "a", "ia", "b"};
  int noteIdx = (int)(eucmod(voltage, 1.0f) / (1.0f / 12.05f));
  char *note = notes[noteIdx];
  if (voltage > 5.0f) {
    sprintf(output, "%sh", note);
  } else if (voltage < -4.0f) {
    sprintf(output, "%sl", note);
  } else {
    sprintf(output, "%s%d", note, ((int)(voltage)) + 4);
  }
}

struct VoltageDisplayWidget : BaseDisplayWidget {
  float *value;
  int *mode;
  std::shared_ptr<Font> font;

  VoltageDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/fonts/DSEG/DSEG7ClassicMini-Italic.ttf"));
  };

  void draw(NVGcontext *vg) override {
    drawBackground(vg);
    NVGcolor lcdGhostColor = nvgRGB(0x1e, 0x1f, 0x1d);
    NVGcolor lcdTextColor = nvgRGB(0xff, 0xd4, 0x2a);

    // Text (integer part)
    nvgFontSize(vg, 11);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 1.0);
    nvgTextAlign(vg, NVG_ALIGN_RIGHT);

    char text[10];
    if (*mode == MUSICAL_MODE) {
      writeMusicalNotation(text, *value);
    } else {
      sprintf(text, "%2.1f", fabsf(*value));
    }

    Vec textPos = Vec(box.size.x - 5.0f, 16.0f);

    nvgFillColor(vg, lcdGhostColor);
    nvgText(vg, textPos.x, textPos.y, *mode == MUSICAL_MODE ? "188" : "18.8", NULL);
    nvgFillColor(vg, lcdTextColor);
    nvgText(vg, textPos.x, textPos.y, text, NULL);
  }
};


struct SRC : Module {
  enum ParamIds {
    COARSE_PARAM,
    FINE_PARAM,
    ON_SWITCH_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    CV_INPUT,
    ON_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    VOLTAGE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    VOLTAGE_POS_LIGHT,
    VOLTAGE_NEG_LIGHT,
    ON_LED,
    NUM_LIGHTS
  };

  SchmittTrigger onButtonTrigger;
  SchmittTrigger externalOnTrigger;

  bool on = true;
  int mode = MUSICAL_MODE;
  bool quantizeInput = false;
  bool onHold = false;
  float voltage = 0.0f;

  void processButtons() {
    if (onHold) {
      on = (bool)params[ON_SWITCH_PARAM].value ^ (bool)inputs[ON_INPUT].value;
    } else if (onButtonTrigger.process(params[ON_SWITCH_PARAM].value || externalOnTrigger.process(inputs[ON_INPUT].value))) {
      on = !on;
    }
  }

  float quantize(float voltage) {
    if (mode == MUSICAL_MODE) {
      return roundf(voltage * 12.0f) / 12.0f;
    } else if (mode == DECIMAL_MODE) {
      return roundf(voltage * 10.0f) / 10.0f;
    }
    return voltage;
  }

  SRC() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
  void step() override;

  json_t *toJson() override {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "on", json_boolean(on));
    json_object_set_new(rootJ, "mode", json_integer(mode));
    json_object_set_new(rootJ, "quantizeInput", json_boolean(quantizeInput));
    json_object_set_new(rootJ, "onHold", json_boolean(onHold));
    return rootJ;
  }

  void fromJson(json_t *rootJ) override {
    json_t *onJ = json_object_get(rootJ, "on");
    json_t *modeJ = json_object_get(rootJ, "mode");
    json_t *quantizeInputJ = json_object_get(rootJ, "quantizeInput");
    json_t *onHoldJ = json_object_get(rootJ, "onHold");
    if (onJ) { on = json_boolean_value(onJ); }
    if (modeJ) { mode = json_integer_value(modeJ); }
    if (quantizeInputJ) { quantizeInput = json_boolean_value(quantizeInputJ); }
    if (onHoldJ) { onHold = json_boolean_value(onHoldJ); }
  }
};


void SRC::step() {
  processButtons();
  float coarse = params[COARSE_PARAM].value;
  float fine = quantize(params[FINE_PARAM].value);
  voltage = clamp(coarse + fine + (inputs[CV_INPUT].active ? (quantizeInput ? quantize(inputs[CV_INPUT].value) : inputs[CV_INPUT].value) : 0.0f), -11.0f, 11.0f);

  if (outputs[VOLTAGE_OUTPUT].active) {
    outputs[VOLTAGE_OUTPUT].value = on ? voltage : 0.0f;
  }
  lights[VOLTAGE_POS_LIGHT].setBrightness(fmaxf(0.0f, voltage / 11.0f));
  lights[VOLTAGE_NEG_LIGHT].setBrightness(fmaxf(0.0f, voltage / -11.0f));
  if (on) {
    lights[ON_LED].value = 1.1f;
  }
}


struct SRCWidget : ModuleWidget {
  SRCWidget(SRC *module);
  void appendContextMenu(Menu *menu) override;
};

SRCWidget::SRCWidget(SRC *module) : ModuleWidget(module) {
  setPanel(SVG::load(assetPlugin(plugin, "res/panels/SRC.svg")));

  addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(25.0f, 42.5f), module, SRC::VOLTAGE_POS_LIGHT));

  VoltageDisplayWidget *display = new VoltageDisplayWidget();
  display->box.pos = Vec(6.0f, 60.0f);
  display->box.size = Vec(33.0f, 21.0f);
  display->value = &module->voltage;
  display->mode = &module->mode;
  addChild(display);

  addParam(ParamWidget::create<ZZC_SelectKnob>(Vec(9, 105), module, SRC::COARSE_PARAM, -10.0f, 10.0f, 0.0f));
  addParam(ParamWidget::create<ZZC_Knob25>(Vec(10, 156), module, SRC::FINE_PARAM, -1.0f, 1.0f, 0.0f));

  addInput(Port::create<ZZC_PJ_Port>(Vec(10.5, 200), Port::INPUT, module, SRC::CV_INPUT));
  addInput(Port::create<ZZC_PJ_Port>(Vec(10.5, 242), Port::INPUT, module, SRC::ON_INPUT));

  addParam(ParamWidget::create<ZZC_LEDBezelDark>(Vec(11.3f, 276.0f), module, SRC::ON_SWITCH_PARAM, 0.0f, 1.0f, 0.0f));
  addChild(ModuleLightWidget::create<LedLight<ZZC_YellowLight>>(Vec(13.1f, 277.7f), module, SRC::ON_LED));

  addOutput(Port::create<ZZC_PJ_Port>(Vec(10.5, 320), Port::OUTPUT, module, SRC::VOLTAGE_OUTPUT));

  addChild(Widget::create<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  addChild(Widget::create<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}


struct SRCMusicalItem : MenuItem {
  SRC *src;
  void onAction(EventAction &e) override {
    src->mode = MUSICAL_MODE;
  }
  void step() override {
    rightText = CHECKMARK(src->mode == MUSICAL_MODE);
  }
};
struct SRCDecimalItem : MenuItem {
  SRC *src;
  void onAction(EventAction &e) override {
    src->mode = DECIMAL_MODE;
  }
  void step() override {
    rightText = CHECKMARK(src->mode == DECIMAL_MODE);
  }
};
struct SRCFreeItem : MenuItem {
  SRC *src;
  void onAction(EventAction &e) override {
    src->mode = FREE_MODE;
  }
  void step() override {
    rightText = CHECKMARK(src->mode == FREE_MODE);
  }
};
struct SRCOnToggleItem : MenuItem {
  SRC *src;
  void onAction(EventAction &e) override {
    src->onHold = false;
  }
  void step() override {
    rightText = CHECKMARK(!src->onHold);
  }
};
struct SRCOnHoldItem : MenuItem {
  SRC *src;
  void onAction(EventAction &e) override {
    src->onHold = true;
  }
  void step() override {
    rightText = CHECKMARK(src->onHold);
  }
};
struct SRCQuantizeItem : MenuItem {
  SRC *src;
  void onAction(EventAction &e) override {
    src->quantizeInput ^= true;
  }
  void step() override {
    rightText = CHECKMARK(src->quantizeInput);
  }
};

void SRCWidget::appendContextMenu(Menu *menu) {
  menu->addChild(new MenuSeparator());

  SRC *src = dynamic_cast<SRC*>(module);
  assert(src);

  SRCMusicalItem *musicalItem = MenuItem::create<SRCMusicalItem>("Fine: Snap to 1/12V");
  SRCDecimalItem *decimalItem = MenuItem::create<SRCDecimalItem>("Fine: Snap to 1/10V");
  SRCFreeItem *freeItem = MenuItem::create<SRCFreeItem>("Fine: Don't snap");
  SRCOnToggleItem *onToggleItem = MenuItem::create<SRCOnToggleItem>("ON: Toggle");
  SRCOnHoldItem *onHoldItem = MenuItem::create<SRCOnHoldItem>("ON: Hold");
  SRCQuantizeItem *quantizeItem = MenuItem::create<SRCQuantizeItem>("Quantize CV like Fine knob");
  musicalItem->src = src;
  decimalItem->src = src;
  freeItem->src = src;
  onToggleItem->src = src;
  onHoldItem->src = src;
  quantizeItem->src = src;
  menu->addChild(musicalItem);
  menu->addChild(decimalItem);
  menu->addChild(freeItem);
  menu->addChild(new MenuSeparator());
  menu->addChild(onToggleItem);
  menu->addChild(onHoldItem);
  menu->addChild(new MenuSeparator());
  menu->addChild(quantizeItem);
}

} // namespace rack_plugin_ZZC

using namespace rack_plugin_ZZC;

RACK_PLUGIN_MODEL_INIT(ZZC, SRC) {
   Model *modelSRC = Model::create<SRC, SRCWidget>("ZZC", "SRC", "Voltage Source", UTILITY_TAG, QUANTIZER_TAG);
   return modelSRC;
}
