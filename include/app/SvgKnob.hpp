#pragma once
#include "app/common.hpp"
#include "app/Knob.hpp"
#include "widget/FramebufferWidget.hpp"
#include "widget/TransformWidget.hpp"
#include "widget/SvgWidget.hpp"
#include "app/CircularShadow.hpp"


namespace rack {
namespace app {


/** A knob which rotates an SVG and caches it in a framebuffer */
struct SvgKnob : Knob {
	widget::FramebufferWidget *fb;
	CircularShadow *shadow;
	widget::TransformWidget *tw;
	widget::SvgWidget *sw;
	/** Angles in radians */
	float minAngle = 0.f;
	float maxAngle = M_PI;

	SvgKnob();
	void setSvg(std::shared_ptr<Svg> svg);
	DEPRECATED void setSVG(std::shared_ptr<Svg> svg) {setSvg(svg);}
	void onChange(const event::Change &e) override;
};


DEPRECATED typedef SvgKnob SVGKnob;


} // namespace app
} // namespace rack
