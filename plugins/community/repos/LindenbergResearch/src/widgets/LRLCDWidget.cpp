#include "../LRComponents.hpp"
#include "../LRModel.hpp"

namespace lrt {

/**
 * @brief Constructor of LCD Widget
 */
LRLCDWidget::LRLCDWidget(unsigned char length, std::string format, LCDType type, float fontsize) {
    tw = new TransformWidget();
    addChild(tw);

    sw = new SVGWidget();
    tw->addChild(sw);

    /** load LCD ttf font */
    ttfLCDDIG7 = Font::load(assetPlugin(plugin, LCD_FONT_DIG7));
    LRLCDWidget::fontsize = fontsize;
    LRLCDWidget::type = type;
    LRLCDWidget::length = length;
    LRLCDWidget::format = format;
    LRLCDWidget::fg = LCD_DEFAULT_COLOR_DARK;

    addSVGVariant(LRGestalt::DARK, SVG::load(assetPlugin(plugin, "res/elements/LCDFrameDark.svg")));
    addSVGVariant(LRGestalt::LIGHT, SVG::load(assetPlugin(plugin, "res/elements/LCDFrameLight.svg")));
    addSVGVariant(LRGestalt::AGED, SVG::load(assetPlugin(plugin, "res/elements/LCDFrameAged.svg")));

    for (int i = 0; i < LRLCDWidget::length; ++i) {
        s1.append("O");
        s2.append("X");
    }
}


/**
 * @brief Draw method of custom LCD widget
 * @param vg
 */
void LRLCDWidget::draw(NVGcontext *vg) {
    FramebufferWidget::draw(vg);

    nvgFontSize(vg, fontsize);
    nvgFontFaceId(vg, ttfLCDDIG7->handle);
    nvgTextLetterSpacing(vg, LCD_LETTER_SPACING);
    //nvgTextAlign(vg, NVG_ALIGN_B | NVG_ALIGN_LEFT);

    float bounds[4];
    nvgTextBoxBounds(vg, 0, 0, 120, s2.c_str(), nullptr, bounds);

    auto mx = (bounds[2] - bounds[0]) * LCD_MARGIN_HORIZONTAL;
    auto my = (bounds[3] - bounds[1]) * LCD_MARGIN_VERTICAL;

    // size of frame not proper setup
    if (!sw->box.size.isEqual(Vec(mx, my))) {
        doResize(Vec(mx, my));
    }

    /**
     * @brief Remark: Due to inconsistent baseline shift on changing the
     * font size this is set rather manual and may do not work with other
     * fonts or sizes.
     *
     */
    float xoffs = (mx - bounds[2] + 0.6f - bounds[0]) / 2.f;
    float yoffs = (my - bounds[3] - 1.f - bounds[1]) / 2.f;


    /*if (++foo % 100 == 0)
        debug("bounds: (%f %f %f %f) box: %f x %f font: %f offs: (%f : %f)", bounds[0], bounds[1], bounds[2], bounds[3], box.size.x, box
                .size.y,
              fontsize, xoffs,
              yoffs);*/

    /*nvgFillPaint(vg, nvgBoxGradient(vg, 0 - 50, 0 - 50, mx + 50, my + 50, 10, 10,
                                    nvgRGBAf(1, 0, 0, 0.25),
                                    nvgRGBAf(0, 0, 0, .0)));
    nvgFill(vg);*/

    nvgFillColor(vg, nvgRGBAf(fg.r, fg.g, fg.b, 0.23));
    std::string str;

    nvgTextBox(vg, xoffs, yoffs, 120, s1.c_str(), nullptr);
    nvgTextBox(vg, xoffs, yoffs, 120, s2.c_str(), nullptr);

    /** if set to inactive just draw the background segments */
    if (!active) return;


    // if set to numeric, do some formatting
    if (type == NUMERIC) {
        str = stringf(format.c_str(), value);

        // quick and dirty, urgs
        if (value < 10)
            text = "0" + text;
    }

    // on text mode just format
    if (type == TEXT) {
        str = stringf(format.c_str(), text.c_str());
    }

    // on list mode get current item out of the current value
    if (type == LIST) {
        unsigned long index;
        long current = lround(value);

        if (current < 0) {
            index = 0;
        } else if ((unsigned long) current >= items.size()) {
            index = items.size() - 1;
        } else {
            index = (unsigned long) current;
        }

        value = index;
        text = items[index];
        str = stringf(format.c_str(), items[index].c_str());
    }

    nvgFillColor(vg, fg);
    nvgTextBox(vg, xoffs, yoffs, 120, str.c_str(), nullptr);
}


void LRLCDWidget::onGestaltChange(LREventGestaltChange &e) {
    auto svg = getSVGVariant(*gestalt);

    if (svg != nullptr) {
        tw->identity();

        sw->setSVG(svg);
        sw->wrap();

        dirty = true;
    }

    LRGestaltChangeAction::onGestaltChange(e);

    switch (*gestalt) {
        case DARK:
            fg = LCD_DEFAULT_COLOR_DARK;
            break;
        case LIGHT:
            fg = LCD_DEFAULT_COLOR_LIGHT;
            break;
        case AGED:
            fg = LCD_DEFAULT_COLOR_AGED;
            break;
        default:
            fg = LCD_DEFAULT_COLOR_DARK;
    }

    e.consumed = true;
}


void LRLCDWidget::doResize(Vec v) {
    auto factor = Vec(v.x / sw->box.size.x, v.y / sw->box.size.y);

    // tw->identity();
    tw->scale(factor);

    sw->box.size = v;
    tw->box.size = sw->box.size;
    box.size = sw->box.size;

    dirty = true;
}


void LRLCDWidget::onMouseDown(EventMouseDown &e) {
    Widget::onMouseDown(e);

    if (type == LIST) {
        if (value < items.size() - 1) value++;
        else value = 0;

        e.consumed = true;
    }
}


void LRLCDWidget::step() {


    FramebufferWidget::step();
}

}