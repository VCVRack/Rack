#include "../LRComponents.hpp"

namespace lrt {

    /**
     * @brief Extention for panel background
     * @param vg
     */
    void LRPanel::draw(NVGcontext *vg) {
        FramebufferWidget::draw(vg);

        nvgBeginPath(vg);
        nvgRect(vg, -MARGIN, -MARGIN, box.size.x + MARGIN * 2, box.size.y + MARGIN * 2);

        NVGpaint paint = nvgLinearGradient(vg, offset.x, offset.y, box.size.x, box.size.y, inner, outer);
        nvgFillPaint(vg, paint);
        nvgFill(vg);
    }


    void LRPanel::setInner(const NVGcolor &inner) {
        LRPanel::inner = inner;
    }


    void LRPanel::setOuter(const NVGcolor &outer) {
        LRPanel::outer = outer;
    }


    LRPanel::LRPanel() {}


    const NVGcolor &LRPanel::getInner() const {
        return inner;
    }


    const NVGcolor &LRPanel::getOuter() const {
        return outer;
    }

}