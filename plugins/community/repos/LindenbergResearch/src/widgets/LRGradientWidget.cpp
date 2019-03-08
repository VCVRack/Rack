#include "../LRComponents.hpp"

namespace lrt {

/**
 * @brief Draw routines for gradient effect
 * @param vg
 */
void LRGradientWidget::draw(NVGcontext *vg) {
    nvgBeginPath(vg);
    nvgRect(vg, -margin, -margin, box.size.x + margin * 2, box.size.y + margin * 2);
    NVGpaint paint = nvgLinearGradient(vg, v1.x, v1.y, v2.x, v2.y, innerColor, outerColor);
    nvgFillPaint(vg, paint);
    nvgFill(vg);

    nvgRestore(vg);
}


const NVGcolor &LRGradientWidget::getInnerColor() const {
    return innerColor;
}


void LRGradientWidget::setInnerColor(const NVGcolor &innerColor) {
    LRGradientWidget::innerColor = innerColor;
}


const NVGcolor &LRGradientWidget::getOuterColor() const {
    return outerColor;
}


void LRGradientWidget::setOuterColor(const NVGcolor &outerColor) {
    LRGradientWidget::outerColor = outerColor;
}

}