#include "ui.hpp"
#include "window.hpp"
#include "util/color.hpp"


namespace rack {


MenuSeparator::MenuSeparator() {
	box.size.y = BND_WIDGET_HEIGHT / 2;
}

void MenuSeparator::draw(NVGcontext *vg) {
	nvgBeginPath(vg);
	const float margin = 8.0;
	nvgMoveTo(vg, margin, box.size.y / 2.0);
	nvgLineTo(vg, box.size.x - margin, box.size.y / 2.0);
	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, colorAlpha(bndGetTheme()->menuTheme.textColor, 0.25));
	nvgStroke(vg);
}


} // namespace rack
