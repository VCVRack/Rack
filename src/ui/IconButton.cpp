#include "ui/IconButton.hpp"


namespace rack {
namespace ui {


IconButton::IconButton() {
	box.size.x = BND_TOOL_WIDTH;

	fw = new widget::FramebufferWidget;
	fw->oversample = 2;
	addChild(fw);

	sw = new widget::SvgWidget;
	sw->box.pos = math::Vec(2, 2);
	fw->addChild(sw);
}

void IconButton::setSvg(std::shared_ptr<Svg> svg) {
	sw->setSvg(svg);
	fw->dirty = true;
}


} // namespace ui
} // namespace rack
