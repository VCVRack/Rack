#pragma once
#include <ui/common.hpp>
#include <widget/OpaqueWidget.hpp>
#include <ui/Scrollbar.hpp>


namespace rack {
namespace ui {


/** Handles a container with Scrollbar */
struct ScrollWidget : widget::OpaqueWidget {
	struct Internal;
	Internal* internal;

	widget::Widget* container;
	Scrollbar* horizontalScrollbar;
	Scrollbar* verticalScrollbar;

	math::Vec offset;
	math::Rect containerBox;
	bool hideScrollbars = false;

	ScrollWidget();
	void scrollTo(math::Rect r);
	/** Returns the bound of allowed `offset` values in pixels. */
	math::Rect getContainerOffsetBound();
	/** Returns the handle position relative to the scrollbar. [0, 1]. */
	math::Vec getHandleOffset();
	/** Returns the handle size relative to the scrollbar. [0, 1]. */
	math::Vec getHandleSize();
	void draw(const DrawArgs& args) override;
	void step() override;
	void onButton(const ButtonEvent& e) override;
	void onDragStart(const DragStartEvent& e) override;
	void onDragMove(const DragMoveEvent& e) override;
	void onHoverScroll(const HoverScrollEvent& e) override;
	void onHoverKey(const HoverKeyEvent& e) override;
};


} // namespace ui
} // namespace rack
