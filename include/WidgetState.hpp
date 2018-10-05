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
	void handleButton(math::Vec pos, int button, int action, int mods);
};


// TODO Move this elsewhere
extern WidgetState *gWidgetState;


} // namespace rack
