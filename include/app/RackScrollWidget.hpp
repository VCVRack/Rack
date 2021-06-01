#pragma once
#include <app/common.hpp>
#include <ui/ScrollWidget.hpp>
#include <widget/ZoomWidget.hpp>
#include <app/RackWidget.hpp>


namespace rack {
namespace app {


struct RackScrollWidget : ui::ScrollWidget {
	widget::ZoomWidget* zoomWidget;
	RackWidget* rackWidget;
	/** The pivot point for zooming */
	math::Vec zoomPos;
	math::Vec oldOffset;

	RackScrollWidget();
	void reset();
	void step() override;
	void draw(const DrawArgs& args) override;
	void onHoverKey(const HoverKeyEvent& e) override;
	void onHoverScroll(const HoverScrollEvent& e) override;
	void onHover(const HoverEvent& e) override;
	void onButton(const ButtonEvent& e) override;
};


} // namespace app
} // namespace rack
