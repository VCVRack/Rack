#include <ui/common.hpp>
#include <settings.hpp>


namespace rack {
namespace ui {


void init() {
	refreshTheme();
}


void destroy() {
}


void setTheme(NVGcolor bg, NVGcolor fg) {
	BNDwidgetTheme w;
	w.outlineColor = color::lerp(bg, fg, 0.1);
	w.itemColor = fg;
	w.innerColor = color::lerp(bg, fg, 0.1);
	w.innerSelectedColor = color::lerp(bg, fg, 0.2);
	w.textColor = fg;
	w.textSelectedColor = fg;
	w.shadeTop = 0;
	w.shadeDown = 0;

	BNDtheme t;
	t.backgroundColor = bg;
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

	// Slider filled background
	t.sliderTheme.itemColor = color::lerp(bg, fg, 0.4);
	// Slider background
	t.sliderTheme.innerColor = color::lerp(bg, fg, 0.0);
	t.sliderTheme.innerSelectedColor = color::lerp(bg, fg, 0.1);

	// Text field background
	t.textFieldTheme.innerColor = color::lerp(bg, fg, 0.7);
	t.textFieldTheme.innerSelectedColor = color::lerp(bg, fg, 0.8);
	// Text
	t.textFieldTheme.textColor = color::lerp(bg, fg, -0.2);
	t.textFieldTheme.textSelectedColor = t.textFieldTheme.textColor;
	// Placeholder text and highlight background
	t.textFieldTheme.itemColor = color::lerp(bg, fg, 0.3);

	t.scrollBarTheme.itemColor = color::lerp(bg, fg, 0.4);
	t.scrollBarTheme.innerColor = color::lerp(bg, fg, 0.1);

	// Menu background
	t.menuTheme.innerColor = bg;
	// Menu label text
	t.menuTheme.textColor = color::lerp(bg, fg, 0.6);
	t.menuTheme.textSelectedColor = t.menuTheme.textColor;

	// Tooltip background
	t.tooltipTheme.innerColor = bg;

	bndSetTheme(t);
}


void refreshTheme() {
	if (settings::uiTheme == "light") {
		setTheme(nvgRGB(0xf0, 0xf0, 0xf0), nvgRGB(0x04, 0x04, 0x04));
	}
	else if (settings::uiTheme == "hcdark") {
		setTheme(nvgRGB(0x00, 0x00, 0x00), nvgRGB(0xff, 0xff, 0xff));
	}
	else {
		// Dark
		setTheme(nvgRGB(0x20, 0x20, 0x20), nvgRGB(0xf0, 0xf0, 0xf0));
	}
}


} // namespace ui
} // namespace rack
