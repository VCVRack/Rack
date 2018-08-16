#include "../LRComponents.hpp"

namespace lrt {


    /**
     * @brief Draw shadow for circular knobs
     * @param vg NVGcontext
     * @param strength Alpha value of outside gradient
     * @param size Outer size
     * @param shift XY Offset shift from middle
     */
    void LRShadow::drawShadow(NVGcontext *vg, float strength, float size) {
        // add shader
        nvgBeginPath(vg);
        nvgRect(vg, -20, -20, box.size.x + 40, box.size.y + 40);

        NVGcolor icol = nvgRGBAf(0.0f, 0.0f, 0.0f, strength);
        NVGcolor ocol = nvgRGBAf(0.0f, 0.0f, 0.0f, 0.f);;

        NVGpaint paint = nvgRadialGradient(vg, box.size.x / 2 + shadowPos.x, box.size.y / 2 + shadowPos.y,
                                           box.size.x * 0.3f, box.size.x * size, icol, ocol);
        nvgFillPaint(vg, paint);
        nvgFill(vg);
    }


    /**
     * @brief Hook into widget draw routine to simulate shadow
     * @param vg
     */
    void LRShadow::draw(NVGcontext *vg) {
        drawShadow(vg, strength, size);
    }


    /**
     * @brief Setter for box dimensions
     * @param box
     */
    void LRShadow::setBox(const Rect &box) {
        LRShadow::box = box;
    }


    /**
     * @brief Setter for outer radius size
     * @param size
     */
    void LRShadow::setSize(float size) {
        LRShadow::size = size;
    }


    /**
     * @brief Setter for draw strength of shadow
     * @param strength
     */
    void LRShadow::setStrength(float strength) {
        LRShadow::strength = strength;
    }


    LRShadow::LRShadow() {

    }

}

