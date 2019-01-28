#pragma once
#include "ui/common.hpp"
#include "widgets/OpaqueWidget.hpp"
#include "ui/ScrollBar.hpp"


namespace rack {


/** Handles a container with ScrollBar */
struct ScrollWidget : OpaqueWidget {
	Widget *container;
	ScrollBar *horizontalScrollBar;
	ScrollBar *verticalScrollBar;
	math::Vec offset;

	ScrollWidget();
	void scrollTo(math::Rect r);
	void draw(const DrawContext &ctx) override;
	void step() override;
	void onHover(const event::Hover &e) override;
	void onHoverScroll(const event::HoverScroll &e) override;
};


} // namespace rack
