#include "LRComponents.hpp"


/**
 * @brief Constructor of LCD Widget
 */
LCDWidget::LCDWidget(NVGcolor fg, unsigned char length) {
    /** load LCD ttf font */
    gLCDFont_DIG7 = Font::load(assetPlugin(plugin, LCD_FONT_DIG7));

    auto r = (unsigned char) (fg.r * 0xFF);
    auto g = (unsigned char) (fg.g * 0xFF);
    auto b = (unsigned char) (fg.b * 0xFF);

    LCDWidget::length = length;

    LCDWidget::fg = fg;
    LCDWidget::bg = nvgRGBA(r - 0x40, g - 0x40, b - 0x40, 0x40);
}


LRModuleWidget::LRModuleWidget(Module *module) : ModuleWidget(module) {
}


/**
 * @brief Draw method of custom LCD widget
 * @param vg
 */
void LCDWidget::draw(NVGcontext *vg) {
    nvgFontSize(vg, LCD_FONTSIZE);
    nvgFontFaceId(vg, gLCDFont_DIG7->handle);
    nvgTextLetterSpacing(vg, LCD_LETTER_SPACING);

    nvgFillColor(vg, bg);

    std::string s1;
    std::string s2;

    for (int i = 0; i < LCDWidget::length; ++i) {
        s1.append("8");
        s2.append(":");
    }

    nvgTextBox(vg, 0, 0, 220, s1.c_str(), nullptr);
    nvgTextBox(vg, 0, 0, 220, s2.c_str(), nullptr);

    nvgFillColor(vg, fg);
    nvgTextBox(vg, 0, 0, 220, text.c_str(), nullptr);
}


void LRRedLight::draw(NVGcontext *vg) {
    //LightWidget::draw(vg);

    float radius = box.size.x / 1.5f;
    float oradius = radius + 10.0f;

    /* color.r = clampf(color.r, 0.0f, 1.0f);
     color.g = clampf(color.g, 0.0f, 1.0f);
     color.b = clampf(color.b, 0.0f, 1.0f);
     color.a = clampf(color.a, 0.0f, 1.0f);*/

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
    icol.a *= 0.40f;
    NVGcolor ocol = color;
    ocol.a = 0.00f;
    paint = nvgRadialGradient(vg, radius, radius, radius, oradius, icol, ocol);
    nvgFillPaint(vg, paint);
    nvgFill(vg);
}


/**
 * @brief Constructor
 */
LRRedLight::LRRedLight() {
    addBaseColor(COLOR_RED);
}


/**
 * @brief Draw routine for cv indicator
 * @param vg
 */
void Indicator::draw(NVGcontext *vg) {
    NVGcolor current = normalColor;

    if (active) {
        /** underrun */
        if (cv < 0.f - OVERFLOW_THRESHOLD) {
            cv = 0.f - OVERFLOW_THRESHOLD;
            current = overflowColor;
        }

        /** overrun */
        if (cv > 1.f + OVERFLOW_THRESHOLD) {
            cv = 1.f + OVERFLOW_THRESHOLD;
            current = overflowColor;
        }


        float a = -angle + cv * angle2;
        float d = distance - 4.f;
        Vec p1, p2, p3;

        /** compute correct point of indicator on circle */
        p1.x = middle.x - sin(-a * (float) M_PI) * distance;
        p1.y = middle.y - cos(-a * (float) M_PI) * distance;

        p2.x = middle.x - sin(-(a + 0.1f) * (float) M_PI) * d;
        p2.y = middle.y - cos(-(a + 0.1f) * (float) M_PI) * d;

        p3.x = middle.x - sin(-(a - 0.1f) * (float) M_PI) * d;
        p3.y = middle.y - cos(-(a - 0.1f) * (float) M_PI) * d;

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


/**
 * @brief Draw shadow for circular knobs
 * @param vg NVGcontext
 * @param strength Alpha value of outside gradient
 * @param size Outer size
 * @param shift XY Offset shift from middle
 */
void LRShadow::drawShadow(NVGcontext *vg, float strength, float size) {
    // add shadow
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