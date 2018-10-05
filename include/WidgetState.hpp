#pragma once
#include "event.hpp"


namespace rack {



struct WidgetState {
	Widget *rootWidget;
	Widget *hoveredWidget;
	Widget *draggedWidget;
	Widget *dragHoveredWidget;
	Widget *selectedWidget;

	WidgetState();

};


// TODO Move this elsewhere
extern WidgetState *gWidgetState;


} // namespace rack
