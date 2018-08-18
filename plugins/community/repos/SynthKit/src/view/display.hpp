#pragma once

#include <cstdint>

#include "asset.hpp"
#include "rack.hpp"

using namespace rack;

#define plugin "SynthKit"

struct FloatDisplay : TransparentWidget {
  float *value;
  std::shared_ptr<Font> font;

  FloatDisplay() {
    value = NULL;
    font = Font::load(assetPlugin(plugin, "res/font/OpenSans-Regular.ttf"));
  }

  void draw(NVGcontext *vg) override {
    char text[12];
    nvgFontSize(vg, 11);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 1);

    nvgFillColor(vg, nvgRGBA(235, 160, 0, 0xff));

    if (value) {
      sprintf(text, "%2.7f", *value);
    } else {
      sprintf(text, "ERROR");
    }

    nvgText(vg, box.pos.x + 1, box.pos.y + 1, text, NULL);
  }
};

struct IntDisplay : TransparentWidget {
  uint32_t *value;
  std::shared_ptr<Font> font;

  IntDisplay() {
    value = NULL;
    font = Font::load(assetPlugin(plugin, "res/font/OpenSans-Regular.ttf"));
  }

  void draw(NVGcontext *vg) override {
    char text[12];
    nvgFontSize(vg, 11);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 1);

    nvgFillColor(vg, nvgRGBA(235, 160, 0, 0xff));

    if (value) {
      sprintf(text, "%d", *value);
    } else {
      sprintf(text, "ERROR");
    }

    nvgText(vg, box.pos.x + 1, box.pos.y + 1, text, NULL);
  }
};
