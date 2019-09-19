#include <ui/MenuSeparator.hpp>


namespace rack {
namespace ui {


MenuSeparator::MenuSeparator() {
	box.size.y = BND_WIDGET_HEIGHT / 2;
}

void MenuSeparator::draw(const DrawArgs& args) {
	nvgBeginPath(args.vg);
	const float margin = 8.0;
	nvgMoveTo(args.vg, margin, box.size.y / 2.0);
	nvgLineTo(args.vg, box.size.x - margin, box.size.y / 2.0);
	nvgStrokeWidth(args.vg, 1.0);
	nvgStrokeColor(args.vg, color::alpha(bndGetTheme()->menuTheme.textColor, 0.25));
	nvgStroke(args.vg);
}


} // namespace ui
} // namespace rack
