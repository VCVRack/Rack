#include "ui/ProgressBar.hpp"


namespace rack {


ProgressBar::ProgressBar() {
	box.size.y = BND_WIDGET_HEIGHT;
}

ProgressBar::~ProgressBar() {
	if (quantity)
		delete quantity;
}

void ProgressBar::draw(const DrawContext &ctx) {
	float progress = quantity ? quantity->getScaledValue() : 0.f;
	std::string text = quantity ? quantity->getString() : "";
	bndSlider(ctx.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_ALL, BND_DEFAULT, progress, text.c_str(), NULL);
}


} // namespace rack
