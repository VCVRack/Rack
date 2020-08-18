#pragma once
#include <ui/common.hpp>
#include <widget/OpaqueWidget.hpp>
#include <ui/ScrollBar.hpp>


namespace rack {
namespace ui {


/** Handles a container with ScrollBar */
struct ScrollWidget : widget::OpaqueWidget {
	struct Internal;
	Internal* internal;

	widget::Widget* container;
	ScrollBar* horizontalScrollBar;
	ScrollBar* verticalScrollBar;

	math::Vec offset;
	math::Rect containerBox;

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
	void onButton(const event::Button& e) override;
	void onDragStart(const event::DragStart& e) override;
	void onDragMove(const event::DragMove& e) override;
	void onHoverScroll(const event::HoverScroll& e) override;
	void onHoverKey(const event::HoverKey& e) override;
};


} // namespace ui
} // namespace rack
