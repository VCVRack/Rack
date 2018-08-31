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
    // NVGcolor borderColor = nvgRGBAf(0.01, 0.03, 0.09, 0.9);
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
    box.size = Vec(7.5f, 7.5f);

    color = LED_DEFAULT_COLOR;
    addBaseColor(color);
    borderColor = nvgRGBAf(color.r / 100, color.g / 100, color.b / 100, 0.9);
    bgColor = nvgRGBAf(color.r, color.g, color.b, 0.3);
    }

}