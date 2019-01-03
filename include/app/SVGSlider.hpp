#pragma once
#include "app/common.hpp"
#include "app/Knob.hpp"
#include "widgets/FramebufferWidget.hpp"
#include "widgets/SVGWidget.hpp"


namespace rack {


/** Behaves like a knob but linearly moves an SVGWidget between two points.
Can be used for horizontal or vertical linear faders.
*/
struct SVGSlider : Knob, FramebufferWidget {
	SVGWidget *background;
	SVGWidget *handle;
	/** Intermediate positions will be interpolated between these positions */
	math::Vec minHandlePos, maxHandlePos;

	SVGSlider();
	void setSVGs(std::shared_ptr<SVG> backgroundSVG, std::shared_ptr<SVG> handleSVG);
	void step() override;
	void onChange(const event::Change &e) override;
};


} // namespace rack
