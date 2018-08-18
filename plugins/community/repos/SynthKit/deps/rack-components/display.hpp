#pragma once

#include <cstdint>

#include "asset.hpp"
#include "rack.hpp"

using namespace rack;

#define plugin "SynthKit"

struct WaveSelect : TransparentWidget {
  uint8_t *value;
  std::shared_ptr<Font> font;

	WaveSelect ( ) {
    value = NULL;
    font = Font::load(assetPlugin(plugin, "res/digit.ttf"));
  }

  void draw (NVGcontext *vg) override {
    nvgFontSize(vg, 8);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 1);

		nvgFillColor(vg, nvgRGBA(0x00, 0xff, 0x00, 0xff));

    if (value) {
      switch (*value) {
      case 0:
        nvgText(vg, box.pos.x + 1, box.pos.y + 1, "SIN", NULL);
        break;
      case 1:
        nvgText(vg, box.pos.x + 1, box.pos.y + 1, "TRI", NULL);
        break;
      case 2:
        nvgText(vg, box.pos.x + 1, box.pos.y + 1, "SAW", NULL);
        break;
      case 3:
        nvgText(vg, box.pos.x + 1, box.pos.y + 1, "SQR", NULL);
        break;
      default:
        nvgText(vg, box.pos.x + 1, box.pos.y + 1, "ERR", NULL);
        break;
      }
    } else {
      nvgText(vg, box.pos.x + 1, box.pos.y + 1, "NUL", NULL);
    }
  }
};

struct ValueDisplay : TransparentWidget {
  float *value;
  std::shared_ptr<Font> font;

	ValueDisplay ( ) {
    value = NULL;
    font = Font::load(assetPlugin(plugin, "res/digit.ttf"));
  }

  void draw (NVGcontext *vg) override {
    char text[12];
    nvgFontSize(vg, 8);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 1);

		nvgFillColor(vg, nvgRGBA(0x00, 0xff, 0x00, 0xff));

    if (value) {
      sprintf(text, "%6.2f", *value);
    } else {
      sprintf(text, "ERROR");
    }

    nvgText(vg, box.pos.x + 1, box.pos.y + 1, text, NULL);
  }
};

struct LEDDisplay : TransparentWidget {
  float *value;

	LEDDisplay ( ) {
    value = NULL;
  }

  void draw (NVGcontext *vg) override {
    NVGcolor red = nvgRGBA(192, 0, 0, 255);
    NVGcolor yellow = nvgRGBA(255, 192, 0, 255);
    NVGcolor green = nvgRGBA(0, 192, 0, 255);
    NVGcolor grey = nvgRGBA(64, 64, 64, 255);
    float val = *value ? *value : 0.0f;

    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, 8, 8);
    if (fabs(val) >= 4.5) {
      nvgFillColor(vg, red);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);


    nvgBeginPath(vg);
    nvgRect(vg, 0, 11, 8, 8);
    if (fabs(val) >= 4.0) {
      nvgFillColor(vg, yellow);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 22, 8, 8);
    if (fabs(val) >= 3.5) {
      nvgFillColor(vg, yellow);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 33, 8, 8);
    if (fabs(val) >= 3.0) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 44, 8, 8);
    if (fabs(val) >= 2.5) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 55, 8, 8);
    if (fabs(val) >= 2.0) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 66, 8, 8);
    if (fabs(val) >= 1.5) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 77, 8, 8);
    if (fabs(val) >= 1.0) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 88, 8, 8);
    if (fabs(val) >= 0.5) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 99, 8, 8);
    if (fabs(val) > 0.0) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);
  }
};

struct LEDSmallDisplay : TransparentWidget {
  float *value;

	LEDSmallDisplay ( ) {
    value = NULL;
  }

  void draw (NVGcontext *vg) override {
    NVGcolor red = nvgRGBA(192, 0, 0, 255);
    NVGcolor yellow = nvgRGBA(255, 192, 0, 255);
    NVGcolor green = nvgRGBA(0, 192, 0, 255);
    NVGcolor grey = nvgRGBA(64, 64, 64, 255);
    float val = *value ? *value : 0.0f;

    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, 8, 6);
    if (fabs(val) >= 4.5) {
      nvgFillColor(vg, red);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);


    nvgBeginPath(vg);
    nvgRect(vg, 0, 8, 8, 6);
    if (fabs(val) >= 4.0) {
      nvgFillColor(vg, yellow);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 16, 8, 6);
    if (fabs(val) >= 3.5) {
      nvgFillColor(vg, yellow);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 24, 8, 6);
    if (fabs(val) >= 3.0) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 32, 8, 6);
    if (fabs(val) >= 2.5) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 40, 8, 6);
    if (fabs(val) >= 2.0) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 48, 8, 6);
    if (fabs(val) >= 1.5) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 56, 8, 6);
    if (fabs(val) >= 1.0) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 64, 8, 6);
    if (fabs(val) >= 0.5) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 72, 8, 6);
    if (fabs(val) > 0.0) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);
  }
};

struct LEDWideDisplay : TransparentWidget {
  float *value;

	LEDWideDisplay ( ) {
    value = NULL;
  }

  void draw (NVGcontext *vg) override {
    NVGcolor red = nvgRGBA(192, 0, 0, 255);
    NVGcolor yellow = nvgRGBA(255, 192, 0, 255);
    NVGcolor green = nvgRGBA(0, 192, 0, 255);
    NVGcolor grey = nvgRGBA(64, 64, 64, 255);
    float val = *value ? *value : 0.0f;

    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, 16, 8);
    if (fabs(val) >= 4.5) {
      nvgFillColor(vg, red);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);


    nvgBeginPath(vg);
    nvgRect(vg, 0, 11, 16, 8);
    if (fabs(val) >= 4.0) {
      nvgFillColor(vg, yellow);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 22, 16, 8);
    if (fabs(val) >= 3.5) {
      nvgFillColor(vg, yellow);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 33, 16, 8);
    if (fabs(val) >= 3.0) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 44, 16, 8);
    if (fabs(val) >= 2.5) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 55, 16, 8);
    if (fabs(val) >= 2.0) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 66, 16, 8);
    if (fabs(val) >= 1.5) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 77, 16, 8);
    if (fabs(val) >= 1.0) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 88, 16, 8);
    if (fabs(val) >= 0.5) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 99, 16, 8);
    if (fabs(val) > 0.0) {
      nvgFillColor(vg, green);
    } else {
      nvgFillColor(vg, grey);
    }
    nvgFill(vg);
  }
};

template <typename BASE>
struct ButtonLight : BASE {
	ButtonLight() {
	  //this->box.size = Vec(20.0, 20.0);
	  this->box.size = mm2px(Vec(6.0, 6.0));
	}
};
