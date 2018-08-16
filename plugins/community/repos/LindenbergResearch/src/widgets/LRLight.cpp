#include "../LRComponents.hpp"

namespace lrt {

    void LRLight::draw(NVGcontext *vg) {
        float radius = box.size.x / 1.5f;
        float oradius = radius + 14.0f;

        // Solid
        nvgBeginPath(vg);
        nvgCircle(vg, radius, radius, radius);
        nvgFillColor(vg, bgColor);
        nvgFill(vg);

        // Border
        nvgStrokeWidth(vg, 1.0f);
        NVGcolor borderColor = bgColor;
        borderColor.a *= 0.5f;
        nvgStrokeColor(vg, borderColor);
        nvgStroke(vg);

        // Inner glow
        nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
        nvgFillColor(vg, color);
        nvgFill(vg);

        // Outer glow
        nvgBeginPath(vg);
        nvgRect(vg, radius - oradius, radius - oradius, 2 * oradius, 2 * oradius);
        NVGpaint paint;
        NVGcolor icol = color;
        icol.a *= 0.30f;
        NVGcolor ocol = color;
        ocol.a = 0.00f;
        paint = nvgRadialGradient(vg, radius, radius, radius, oradius, icol, ocol);
        nvgFillPaint(vg, paint);
        nvgFill(vg);
    }


    /**
     * @brief Constructor
     */
    LRLight::LRLight() {
        addBaseColor(nvgRGBAf(0.1, 0.3, 0.9, 0.99));
    }

}