#pragma once
#include "event.hpp"
#include "widgets/Widget.hpp"


namespace rack {



struct WidgetState {
	Widget *rootWidget = NULL;
	Widget *hoveredWidget = NULL;
	Widget *draggedWidget = NULL;
	Widget *dragHoveredWidget = NULL;
	Widget *selectedWidget = NULL;
	/** For middle-click dragging */
	Widget *scrollWidget = NULL;

	void handleButton(math::Vec pos, int button, int action, int mods);
	void handleHover(math::Vec pos, math::Vec mouseDelta);
	void handleLeave();
	void handleScroll(math::Vec pos, math::Vec scrollDelta);
	void handleText(math::Vec pos, int codepoint);
	void handleKey(math::Vec pos, int key, int scancode, int action, int mods);
	void handleDrop(math::Vec pos, std::vector<std::string> paths);
	void handleZoom();
	/** Prepares a widget for deletion */
	void finalizeWidget(Widget *w);
};


// TODO Move this elsewhere
extern WidgetState *gWidgetState;


} // namespace rack
