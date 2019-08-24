#pragma once
#include <app/common.hpp>
#include <widget/FramebufferWidget.hpp>
#include <widget/SvgWidget.hpp>
#include <app/CircularShadow.hpp>
#include <app/Switch.hpp>


namespace rack {
namespace app {


/** A ParamWidget with multiple frames corresponding to its value */
struct SvgSwitch : Switch {
	widget::FramebufferWidget* fb;
	CircularShadow* shadow;
	widget::SvgWidget* sw;
	std::vector<std::shared_ptr<Svg>> frames;

	SvgSwitch();
	/** Adds an SVG file to represent the next switch position */
	void addFrame(std::shared_ptr<Svg> svg);
	void onChange(const event::Change& e) override;
};


DEPRECATED typedef SvgSwitch SVGSwitch;


} // namespace app
} // namespace rack
