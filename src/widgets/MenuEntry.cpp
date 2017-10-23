#include "widgets.hpp"
#include "gui.hpp"


namespace rack {


void MenuEntry::step() {
	// Add 10 more pixels because Retina measurements are sometimes too small
	const float rightPadding = 10.0;
	// HACK use gVg from the gui.
	// All this does is inspect the font, so it shouldn't modify gVg and should work when called from a FramebufferWidget for example.
	box.size.x = bndLabelWidth(gVg, -1, text.c_str()) + bndLabelWidth(gVg, -1, rightText.c_str()) + rightPadding;
	Widget::step();
}


} // namespace rack
