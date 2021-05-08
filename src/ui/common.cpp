#include <ui/common.hpp>


namespace rack {
namespace ui {


void init() {
	setTheme(nvgRGB(0x33, 0x33, 0x33), nvgRGB(0xf0, 0xf0, 0xf0));
}

void destroy() {
}

void setTheme(NVGcolor bg, NVGcolor fg) {
	// Assume dark background and light foreground

	BNDwidgetTheme w;
	w.outlineColor = bg;
	w.itemColor = fg;
	w.innerColor = bg;
	w.innerSelectedColor = color::plus(bg, nvgRGB(0x30, 0x30, 0x30));
	w.textColor = fg;
	w.textSelectedColor = fg;
	w.shadeTop = 0;
	w.shadeDown = 0;

	BNDtheme t;
	t.backgroundColor = color::plus(bg, nvgRGB(0x30, 0x30, 0x30));
	t.regularTheme = w;
	t.toolTheme = w;
	t.radioTheme = w;
	t.textFieldTheme = w;
	t.optionTheme = w;
	t.choiceTheme = w;
	t.numberFieldTheme = w;
	t.sliderTheme = w;
	t.scrollBarTheme = w;
	t.tooltipTheme = w;
	t.menuTheme = w;
	t.menuItemTheme = w;

	t.sliderTheme.itemColor = bg;
	t.sliderTheme.innerColor = color::plus(bg, nvgRGB(0x50, 0x50, 0x50));
	t.sliderTheme.innerSelectedColor = color::plus(bg, nvgRGB(0x60, 0x60, 0x60));

	t.textFieldTheme = t.sliderTheme;
	t.textFieldTheme.textColor = color::minus(bg, nvgRGB(0x20, 0x20, 0x20));
	t.textFieldTheme.textSelectedColor = t.textFieldTheme.textColor;
	t.textFieldTheme.itemColor = color::plus(bg, nvgRGB(0x30, 0x30, 0x30));

	t.scrollBarTheme.itemColor = color::plus(bg, nvgRGB(0x50, 0x50, 0x50));
	t.scrollBarTheme.innerColor = bg;

	t.menuTheme.innerColor = color::minus(bg, nvgRGB(0x10, 0x10, 0x10));
	t.menuTheme.textColor = color::minus(fg, nvgRGB(0x50, 0x50, 0x50));
	t.menuTheme.textSelectedColor = t.menuTheme.textColor;

	bndSetTheme(t);
}


} // namespace ui
} // namespace rack
