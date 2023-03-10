#include <ui/Tooltip.hpp>
#include <context.hpp>
#include <app/Scene.hpp>


namespace rack {
namespace ui {


void Tooltip::step() {
	// Wrap size to contents
	nvgSave(APP->window->vg);
	nvgTextLineHeight(APP->window->vg, 1.2);
	box.size.x = bndLabelWidth(APP->window->vg, -1, text.c_str());
	box.size.y = bndLabelHeight(APP->window->vg, -1, text.c_str(), INFINITY);
	// Position near cursor. This assumes that the Tooltip is added to the root widget.
	box.pos = APP->scene->mousePos.plus(math::Vec(15, 15));
	// Fit inside parent
	assert(parent);
	box = box.nudge(parent->box.zeroPos());
	nvgRestore(APP->window->vg);

	Widget::step();
}

void Tooltip::draw(const DrawArgs& args) {
	bndTooltipBackground(args.vg, 0.0, 0.0, box.size.x, box.size.y);
	nvgTextLineHeight(args.vg, 1.2);

	// Because there is no bndThemeLabel() function, temporarily replace the menu text color with tooltip text color and draw a menu label
	BNDtheme* theme = (BNDtheme*) bndGetTheme();
	NVGcolor menuTextColor = theme->menuTheme.textColor;
	theme->menuTheme.textColor = theme->tooltipTheme.textColor;
	bndMenuLabel(args.vg, 0.0, 0.0, INFINITY, box.size.y, -1, text.c_str());
	theme->menuTheme.textColor = menuTextColor;

	Widget::draw(args);
}


} // namespace ui
} // namespace rack
