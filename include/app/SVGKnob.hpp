#pragma once
#include "app/common.hpp"
#include "app/Knob.hpp"
#include "widget/FramebufferWidget.hpp"
#include "widget/TransformWidget.hpp"
#include "widget/SVGWidget.hpp"
#include "app/CircularShadow.hpp"


namespace rack {
namespace app {


/** A knob which rotates an SVG and caches it in a framebuffer */
struct SVGKnob : Knob {
	widget::FramebufferWidget *fb;
	widget::TransformWidget *tw;
	widget::SVGWidget *sw;
	CircularShadow *shadow;
	/** Angles in radians */
	float minAngle, maxAngle;

	SVGKnob();
	void setSVG(std::shared_ptr<SVG> svg);
	void onChange(const event::Change &e) override;
};


} // namespace app
} // namespace rack
