#include <sstream>
#include <iomanip>


//  From AS DelayPlus.cpp https://github.com/AScustomWorks/AS
struct MsDisplayWidget : TransparentWidget {

  float *value;
  std::shared_ptr<Font> font;

  MsDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  }

  void draw(NVGcontext *vg) override {
    // Background
    // these go to...
    NVGcolor backgroundColor = nvgRGB(0x11, 0x11, 0x11);

    NVGcolor borderColor = nvgRGB(0xff, 0xff, 0xff);

    nvgBeginPath(vg);

    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 3.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);

    nvgStrokeWidth(vg, 1.0f);
    nvgStrokeColor(vg, borderColor);

    nvgStroke(vg);

    // text
    nvgFontSize(vg, 14);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);

    std::stringstream to_display;
    // to_display << std::setiosflags(std::ios::fixed) << std::right  << std::setw(5) << std::setprecision(4) << *value;
    to_display << std::setiosflags(std::ios::fixed) << std::left << std::setprecision(4) << *value;

    Vec textPos = Vec(1.0f, 19.0f);

    NVGcolor textColor = nvgRGB(0x65, 0xf6, 0x78);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};

