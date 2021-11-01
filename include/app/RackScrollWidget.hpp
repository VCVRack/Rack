#pragma once
#include <app/common.hpp>
#include <ui/ScrollWidget.hpp>
#include <widget/ZoomWidget.hpp>
#include <app/RackWidget.hpp>


namespace rack {
namespace app {


struct RackScrollWidget : ui::ScrollWidget {
	struct Internal;
	Internal* internal;

	widget::ZoomWidget* zoomWidget;
	RackWidget* rackWidget;

	PRIVATE RackScrollWidget();
	PRIVATE ~RackScrollWidget();

	void reset();
	/** Gets the top-left scroll offset in grid coordinates.
	*/
	math::Vec getGridOffset();
	void setGridOffset(math::Vec gridOffset);
	float getZoom();
	/** Sets the zoom level, with a pivot at the center of the scroll viewport.
	*/
	void setZoom(float zoom);
	void setZoom(float zoom, math::Vec pivot);

	void step() override;
	void draw(const DrawArgs& args) override;
	void onHoverKey(const HoverKeyEvent& e) override;
	void onHoverScroll(const HoverScrollEvent& e) override;
	void onHover(const HoverEvent& e) override;
	void onButton(const ButtonEvent& e) override;
};


} // namespace app
} // namespace rack
