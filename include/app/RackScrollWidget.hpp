#pragma once
#include <app/common.hpp>
#include <ui/ScrollWidget.hpp>
#include <widget/ZoomWidget.hpp>
#include <app/RackWidget.hpp>


namespace rack {
namespace app {


struct RackScrollWidget : ui::ScrollWidget {
	widget::ZoomWidget *zoomWidget;
	RackWidget *rackWidget;
	/** The pivot point for zooming */
	math::Vec zoomPos;
	math::Vec oldOffset;

	RackScrollWidget();
	void step() override;
	void draw(const DrawArgs &args) override;
	void onHoverKey(const event::HoverKey &e) override;
	void onHoverScroll(const event::HoverScroll &e) override;
	void reset();
};


} // namespace app
} // namespace rack
