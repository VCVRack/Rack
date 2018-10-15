#pragma once
#include "math.hpp"
#include <vector>


namespace rack {


struct Widget;


struct WidgetState {
	Widget *rootWidget = NULL;
	Widget *hoveredWidget = NULL;
	Widget *draggedWidget = NULL;
	Widget *dragHoveredWidget = NULL;
	Widget *selectedWidget = NULL;
	/** For middle-click dragging */
	Widget *scrollWidget = NULL;

	void handleButton(Vec pos, int button, int action, int mods);
	void handleHover(Vec pos, Vec mouseDelta);
	void handleLeave();
	void handleScroll(Vec pos, Vec scrollDelta);
	void handleText(Vec pos, int codepoint);
	void handleKey(Vec pos, int key, int scancode, int action, int mods);
	void handleDrop(Vec pos, std::vector<std::string> paths);
	void handleZoom();
	/** Prepares a widget for deletion */
	void finalizeWidget(Widget *w);
};


// TODO Move this elsewhere
extern WidgetState *gWidgetState;


} // namespace rack
