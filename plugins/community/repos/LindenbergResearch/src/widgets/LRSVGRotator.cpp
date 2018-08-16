#include "../LRComponents.hpp"

namespace lrt {

    SVGRotator::SVGRotator() : FramebufferWidget() {
        tw = new TransformWidget();
        addChild(tw);

        sw = new SVGWidget();
        tw->addChild(sw);
    }


    /**
     * @brief Set SVG image to rotator
     * @param svg
     */
    void SVGRotator::setSVG(std::shared_ptr<SVG> svg) {
        sw->setSVG(svg);
        tw->box.size = sw->box.size;
        box.size = sw->box.size;
    }


    /**
     * @brief Rotate one step
     */
    void SVGRotator::step() {
        tw->identity();

        angle = fmodf(angle + inc, 2 * M_PI);;

        Vec center = sw->box.getCenter();
        tw->translate(center);
        tw->rotate(angle);
        tw->translate(center.neg());

        dirty = true;

        FramebufferWidget::step();
    }
}