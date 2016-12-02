#include "Rack.hpp"


namespace rack {

void MenuOverlay::onMouseDown(int button) {
	if (parent) {
		parent->removeChild(this);
	}
	// Commit sudoku
	delete this;
}


} // namespace rack
