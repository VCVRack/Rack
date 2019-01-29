#pragma once
#include "app/common.hpp"
#include "app/SliderKnob.hpp"
#include "widget/FramebufferWidget.hpp"
#include "widget/SVGWidget.hpp"


namespace rack {
namespace app {


/** Behaves like a knob but linearly moves an widget::SVGWidget between two points.
Can be used for horizontal or vertical linear faders.
*/
struct SVGSlider : app::SliderKnob {
	widget::FramebufferWidget *fb;
	widget::SVGWidget *background;
	widget::SVGWidget *handle;
	/** Intermediate positions will be interpolated between these positions */
	math::Vec minHandlePos, maxHandlePos;

	SVGSlider();
	void setBackgroundSVG(std::shared_ptr<SVG> backgroundSVG);
	void setHandleSVG(std::shared_ptr<SVG> handleSVG);
	void onChange(const event::Change &e) override;

	DEPRECATED void setSVGs(std::shared_ptr<SVG> backgroundSVG, std::shared_ptr<SVG> handleSVG) {
		setBackgroundSVG(backgroundSVG);
		setHandleSVG(handleSVG);
	}
};


} // namespace app
} // namespace rack
