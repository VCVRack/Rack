#pragma once
#include "app/common.hpp"
#include "app/CircularShadow.hpp"
#include "widgets/TransformWidget.hpp"


namespace rack {


/** A knob which rotates an SVG and caches it in a framebuffer */
struct SVGKnob : Knob, FramebufferWidget {
	TransformWidget *tw;
	SVGWidget *sw;
	CircularShadow *shadow;
	/** Angles in radians */
	float minAngle, maxAngle;

	SVGKnob();
	void setSVG(std::shared_ptr<SVG> svg);
	void step() override;
	void onChange(event::Change &e) override;
};


} // namespace rack
