#include "../LRComponents.hpp"
#include "../LRModel.hpp"

namespace lrt {

/**
 * @brief Constructor of LCD Widget
 */
LRLCDWidget::LRLCDWidget(unsigned char length, std::string format, LCDType type, float fontsize) {
    /** load LCD ttf font */
    ttfLCDDig7 = Font::load(assetPlugin(plugin, LCD_FONT_DIG7));
    LRLCDWidget::fontsize = fontsize;

    LRLCDWidget::type = type;

    LRLCDWidget::length = length;
    LRLCDWidget::format = format;

    LRLCDWidget::fg = LCD_DEFAULT_COLOR_DARK;
    LRLCDWidget::bg = nvgRGBAf(fg.r, fg.g, fg.b, 0.15f);

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
    nvgFontSize(vg, fontsize);
    nvgFontFaceId(vg, ttfLCDDig7->handle);
    nvgTextLetterSpacing(vg, LCD_LETTER_SPACING);

    auto digitsColor = nvgRGBAf(fg.r, fg.b, fg.b, 0.1);

    nvgFillColor(vg, digitsColor);
    std::string str;

    nvgTextBox(vg, 0, 0, 220, s1.c_str(), nullptr);
    nvgTextBox(vg, 0, 0, 220, s2.c_str(), nullptr);

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

        str = stringf(format.c_str(), items[index].c_str());
    }

    nvgFillColor(vg, fg);
    nvgTextBox(vg, 0, 0, 220, str.c_str(), nullptr);
}


void LRLCDWidget::onGestaltChange(LREventGestaltChange &e) {
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

}