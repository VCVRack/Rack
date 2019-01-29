#include "ui/MenuSeparator.hpp"


namespace rack {
namespace ui {


MenuSeparator::MenuSeparator() {
	box.size.y = BND_WIDGET_HEIGHT / 2;
}

void MenuSeparator::draw(const widget::DrawContext &ctx) {
	nvgBeginPath(ctx.vg);
	const float margin = 8.0;
	nvgMoveTo(ctx.vg, margin, box.size.y / 2.0);
	nvgLineTo(ctx.vg, box.size.x - margin, box.size.y / 2.0);
	nvgStrokeWidth(ctx.vg, 1.0);
	nvgStrokeColor(ctx.vg, color::alpha(bndGetTheme()->menuTheme.textColor, 0.25));
	nvgStroke(ctx.vg);
}


} // namespace ui
} // namespace rack
