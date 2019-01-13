#pragma once
#include "app/common.hpp"
#include "widgets/Widget.hpp"
#include "widgets/FramebufferWidget.hpp"
#include "widgets/SVGWidget.hpp"
#include "app/Switch.hpp"


namespace rack {


/** A ParamWidget with multiple frames corresponding to its value */
struct SVGSwitch : Switch {
	FramebufferWidget *fb;
	SVGWidget *sw;
	std::vector<std::shared_ptr<SVG>> frames;

	SVGSwitch();
	/** Adds an SVG file to represent the next switch position */
	void addFrame(std::shared_ptr<SVG> svg);
	void onChange(const event::Change &e) override;
};


} // namespace rack
