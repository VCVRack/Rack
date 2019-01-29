#include "ui/IconButton.hpp"


namespace rack {
namespace ui {


IconButton::IconButton() {
	box.size.x = BND_TOOL_WIDTH;

	fw = new widget::FramebufferWidget;
	fw->oversample = 2;
	addChild(fw);

	sw = new widget::SVGWidget;
	sw->box.pos = math::Vec(2, 2);
	fw->addChild(sw);
}

void IconButton::setSVG(std::shared_ptr<SVG> svg) {
	sw->setSVG(svg);
	fw->dirty = true;
}


} // namespace ui
} // namespace rack
