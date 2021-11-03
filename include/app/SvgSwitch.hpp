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
	struct Internal;
	Internal* internal;

	widget::FramebufferWidget* fb;
	CircularShadow* shadow;
	widget::SvgWidget* sw;
	std::vector<std::shared_ptr<window::Svg>> frames;

	/** Use frames 0 and 1 when the mouse is pressed and released, instead of using the param value as the frame index.
	TODO change name
	*/
	bool latch = false;

	SvgSwitch();
	~SvgSwitch();
	/** Adds an SVG file to represent the next switch position */
	void addFrame(std::shared_ptr<window::Svg> svg);

	void onDragStart(const DragStartEvent& e) override;
	void onDragEnd(const DragEndEvent& e) override;
	void onChange(const ChangeEvent& e) override;
};


DEPRECATED typedef SvgSwitch SVGSwitch;


} // namespace app
} // namespace rack
