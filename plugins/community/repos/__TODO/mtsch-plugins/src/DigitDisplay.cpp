// Adapted from https://github.com/luckyxxl/vcv_luckyxxl

#include "mtsch.hpp"
#include "DigitDisplay.hpp"

#include <map>
#include <utility>


DigitDisplay::DigitDisplay(Vec position, float size, char *display)
    : position(position), size(size), display(display) {
}

static void drawSegment(NVGcontext *vg, const NVGcolor &color) {
    nvgBeginPath(vg);
    nvgMoveTo(vg, 0.f, 0.f);
    nvgLineTo(vg, .5f, .5f);
    nvgLineTo(vg, 1.5f, .5f);
    nvgLineTo(vg, 2.f, 0.f);
    nvgLineTo(vg, 1.5f, -.5f);
    nvgLineTo(vg, .5f, -.5f);
    nvgClosePath(vg);

    nvgFillColor(vg, color);
    nvgFill(vg);
}

#define BITMAP(a, b, c, d, e, f, g) (a << 0 | b << 1 | c << 2 | d << 3 | e << 4 | f << 5 | g << 6)

static const std::map<char, uint8_t> bitmaps = {
    std::make_pair('\0', BITMAP(0, 0, 0, 0, 0, 0, 0)),
    std::make_pair('0', BITMAP(1, 1, 1, 0, 1, 1, 1)),
    std::make_pair('1', BITMAP(0, 0, 1, 0, 0, 1, 0)),
    std::make_pair('2', BITMAP(1, 0, 1, 1, 1, 0, 1)),
    std::make_pair('3', BITMAP(1, 0, 1, 1, 0, 1, 1)),
    std::make_pair('4', BITMAP(0, 1, 1, 1, 0, 1, 0)),
    std::make_pair('5', BITMAP(1, 1, 0, 1, 0, 1, 1)),
    std::make_pair('6', BITMAP(1, 1, 0, 1, 1, 1, 1)),
    std::make_pair('7', BITMAP(1, 0, 1, 0, 0, 1, 0)),
    std::make_pair('8', BITMAP(1, 1, 1, 1, 1, 1, 1)),
    std::make_pair('9', BITMAP(1, 1, 1, 1, 0, 1, 1)),
    std::make_pair('#', BITMAP(0, 1, 1, 1, 1, 1, 0)),
    std::make_pair('a', BITMAP(1, 1, 1, 1, 1, 1, 0)),
    std::make_pair('b', BITMAP(0, 1, 0, 1, 1, 1, 1)),
    std::make_pair('c', BITMAP(1, 1, 0, 0, 1, 0, 1)),
    std::make_pair('d', BITMAP(0, 0, 1, 1, 1, 1, 1)),
    std::make_pair('e', BITMAP(1, 1, 0, 1, 1, 0, 1)),
    std::make_pair('f', BITMAP(1, 1, 0, 1, 1, 0, 0)),
    std::make_pair('g', BITMAP(1, 1, 1, 1, 0, 1, 1)),
};

#undef BITMAP

void DigitDisplay::draw(NVGcontext *vg) {
    nvgSave(vg);

    nvgTranslate(vg, position.x, position.y);
    nvgScale(vg, size, size);

    static const NVGcolor off_color = nvgRGB(200, 200, 200);
    static const NVGcolor on_color = nvgRGB(30, 30, 30);

    uint8_t bitmap = 0u;
    {
        auto b = bitmaps.find(*display);
        if(b != bitmaps.end()) bitmap = b->second;
    }

#define BIT(x) (bitmap & 1 << x) ? on_color : off_color

    drawSegment(vg, BIT(0));
    nvgTranslate(vg, 0.f, 2.f);
    drawSegment(vg, BIT(3));
    nvgTranslate(vg, 0.f, 2.f);
    drawSegment(vg, BIT(6));

    nvgTranslate(vg, 0.f, -4.f);
    nvgRotate(vg, NVG_PI/2.f);

    drawSegment(vg, BIT(1));
    nvgTranslate(vg, 0.f, -2.f);
    drawSegment(vg, BIT(2));

    nvgTranslate(vg, 2.f, 2.f);

    drawSegment(vg, BIT(4));
    nvgTranslate(vg, 0.f, -2.f);
    drawSegment(vg, BIT(5));

#undef BIT

    nvgRestore(vg);
}
