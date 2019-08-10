#pragma once
#include <ui/common.hpp>
#include <widget/OpaqueWidget.hpp>
#include <ui/ScrollBar.hpp>


namespace rack {
namespace ui {


/** Handles a container with ScrollBar */
struct ScrollWidget : widget::OpaqueWidget {
	widget::Widget* container;
	ScrollBar* horizontalScrollBar;
	ScrollBar* verticalScrollBar;
	math::Vec offset;

	ScrollWidget();
	void scrollTo(math::Rect r);
	void draw(const DrawArgs& args) override;
	void step() override;
	void onButton(const event::Button& e) override;
	void onDragStart(const event::DragStart& e) override;
	void onDragMove(const event::DragMove& e) override;
	void onHoverScroll(const event::HoverScroll& e) override;
};


} // namespace ui
} // namespace rack
