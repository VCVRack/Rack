#include <ui/OptionButton.hpp>


namespace rack {
namespace ui {


void OptionButton::draw(const DrawArgs& args) {
	BNDwidgetState state = BND_DEFAULT;
	if (quantity) {
		if (quantity->isMax())
			state = BND_ACTIVE;
	}

	std::string text = this->text;
	if (text.empty() && quantity)
		text = quantity->getLabel();
	bndOptionButton(args.vg, 0.0, 0.0, INFINITY, box.size.y, state, text.c_str());
}


} // namespace ui
} // namespace rack
