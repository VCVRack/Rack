#pragma once
#include "widget/Widget.hpp"


namespace rack {
namespace widget {


/** A Widget that does not respond to events and does not pass events to children */
struct TransparentWidget : Widget {
	/** Override behavior to do nothing instead. */
	void onHover(const HoverEvent &e) override {}
	void onButton(const ButtonEvent &e) override {}
	void onHoverKey(const HoverKeyEvent &e) override {}
	void onHoverText(const HoverTextEvent &e) override {}
	void onHoverScroll(const HoverScrollEvent &e) override {}
	void onDragHover(const DragHoverEvent &e) override {}
	void onPathDrop(const PathDropEvent &e) override {}
};


} // namespace widget
} // namespace rack
