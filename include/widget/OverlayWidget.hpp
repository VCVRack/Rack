#pragma once
#include "widget/OpaqueWidget.hpp"


namespace rack {
namespace widget {


/** Like OpaqueWidget but consumes even more events. */
struct OverlayWidget : OpaqueWidget {
	void onHoverScroll(const HoverScrollEvent &e) override {
		Widget::onHoverScroll(e);
		if (!e.getConsumed())
			e.consume(this);
	}
	void onPathDrop(const PathDropEvent &e) override {
		Widget::onPathDrop(e);
		if (!e.getConsumed())
			e.consume(this);
	}
};


} // namespace widget
} // namespace rack
