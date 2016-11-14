#include "../5V.hpp"


void MenuOverlay::onMouseDown(int button) {
	if (parent) {
		parent->removeChild(this);
	}
	// Commit sudoku
	delete this;
}
