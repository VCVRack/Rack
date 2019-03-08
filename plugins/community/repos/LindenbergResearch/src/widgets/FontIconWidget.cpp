#include "../LRComponents.hpp"

#define ICON_FONT_GLYPHYX "res/GlyphyxOneNF.ttf"

using namespace rack;
using namespace lrt;


FontIconWidget::FontIconWidget(float fontSize, NVGcolor color) {
    iconFont = Font::load(assetPlugin(plugin, ICON_FONT_GLYPHYX));
    FontIconWidget::fontSize = fontSize;
    FontIconWidget::color = color;
}


void FontIconWidget::draw(NVGcontext *vg) {
    FramebufferWidget::draw(vg);

    nvgFontSize(vg, fontSize);
    nvgFontFaceId(vg, iconFont->handle);
    nvgTextLetterSpacing(vg, 0);


    nvgFillColor(vg, color);
    nvgTextBox(vg, parent->box.size.x / 2 - fontSize / 2, parent->box.size.y / 2 - fontSize / 2, 0, "H", nullptr);
    //  nvgTextBox(vg, 0, 0, 0, "Ldfgjsndfklgjhbsdfkljhgsdkfjgh", nullptr);
}
