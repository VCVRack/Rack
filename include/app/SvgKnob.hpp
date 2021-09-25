#pragma once
#include <app/common.hpp>
#include <app/Knob.hpp>
#include <widget/FramebufferWidget.hpp>
#include <widget/TransformWidget.hpp>
#include <widget/SvgWidget.hpp>
#include <app/CircularShadow.hpp>


namespace rack {
namespace app {


/** A knob which rotates an SVG and caches it in a framebuffer */
struct SvgKnob : Knob {
	widget::FramebufferWidget* fb;
	CircularShadow* shadow;
	widget::TransformWidget* tw;
	widget::SvgWidget* sw;

	SvgKnob();
	void setSvg(std::shared_ptr<window::Svg> svg);
	DEPRECATED void setSVG(std::shared_ptr<window::Svg> svg) {
		setSvg(svg);
	}
	void onChange(const ChangeEvent& e) override;
};


DEPRECATED typedef SvgKnob SVGKnob;


} // namespace app
} // namespace rack
