#include <ui/ProgressBar.hpp>


namespace rack {
namespace ui {


ProgressBar::ProgressBar() {
	box.size.y = BND_WIDGET_HEIGHT;
}

void ProgressBar::draw(const DrawArgs& args) {
	float progress = quantity ? quantity->getScaledValue() : 0.f;
	std::string text = quantity ? quantity->getString() : "";
	bndSlider(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_ALL, BND_DEFAULT, progress, text.c_str(), NULL);
}


} // namespace ui
} // namespace rack
