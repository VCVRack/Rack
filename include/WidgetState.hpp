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
	void handleHover(math::Vec pos, math::Vec mouseDelta);
	void handleLeave();
	void handleScroll(math::Vec pos, math::Vec scrollDelta);
	void handleChar(math::Vec pos, int codepoint);
	void handleKey(math::Vec pos, int key, int scancode, int action, int mods);
	void handleDrop(math::Vec pos, std::vector<std::string> paths);
};


// TODO Move this elsewhere
extern WidgetState *gWidgetState;


} // namespace rack
