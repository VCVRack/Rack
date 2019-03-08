#include "../LRComponents.hpp"


namespace lrt {


/**
 * @brief Init indicator
 * @param distance Radius viewed from the middle
 * @param angle Angle of active knob area
 */
LRCVIndicator::LRCVIndicator(float distance, float angle) {
    LRCVIndicator::distance = distance;
    LRCVIndicator::angle = angle;

    /** for optimization */
    angle2 = 2 * angle;
}


/**
 * @brief Draw routine for cv indicator
 * @param vg
 */
void LRCVIndicator::draw(NVGcontext *vg) {
    NVGcolor current = lightMode ? normalColorLight : normalColor;

    if (active) {
        /** underrun */
        if (cv < 0.f - OVERFLOW_THRESHOLD) {
            cv = 0.f - OVERFLOW_THRESHOLD;
            current = lightMode ? overflowColorLight : overflowColor;
        }

        /** overrun */
        if (cv > 1.f + OVERFLOW_THRESHOLD) {
            cv = 1.f + OVERFLOW_THRESHOLD;
            current = lightMode ? overflowColorLight : overflowColor;
        }


        float a = -angle + cv * angle2;
        float d = distance - d1;
        Vec p1, p2, p3;

        /** compute correct point of indicator on circle */
        p1.x = middle.x - sin(-a * (float) M_PI) * distance;
        p1.y = middle.y - cos(-a * (float) M_PI) * distance;

        p2.x = middle.x - sin(-(a + d2) * (float) M_PI) * d;
        p2.y = middle.y - cos(-(a + d2) * (float) M_PI) * d;

        p3.x = middle.x - sin(-(a - d2) * (float) M_PI) * d;
        p3.y = middle.y - cos(-(a - d2) * (float) M_PI) * d;

        nvgBeginPath(vg);
        nvgMoveTo(vg, p1.x, p1.y);
        nvgLineTo(vg, p2.x, p2.y);
        nvgLineTo(vg, p3.x, p3.y);
        nvgLineTo(vg, p1.x, p1.y);
        nvgClosePath(vg);

        nvgFillColor(vg, current);
        nvgFill(vg);
    }
}


void LRCVIndicator::setDistances(float d1, float d2) {
    LRCVIndicator::d1 = d1;
    LRCVIndicator::d2 = d2;
}


}