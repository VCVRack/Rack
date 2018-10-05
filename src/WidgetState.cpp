#include "WidgetState.hpp"

namespace rack {


WidgetState::WidgetState() {
	rootWidget = NULL;
	hoveredWidget = NULL;
	draggedWidget = NULL;
	dragHoveredWidget = NULL;
	selectedWidget = NULL;
}


// TODO Move this elsewhere
WidgetState *gWidgetState = NULL;


} // namespace rack
