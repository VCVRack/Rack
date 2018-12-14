#pragma once
#include "app/common.hpp"
#include "engine.hpp"


namespace rack {


/** A Component with a default (up) and active (down) state when clicked.
Does not modify a Param, simply calls onAction() of a subclass.
*/
struct SVGButton : FramebufferWidget {
	std::shared_ptr<SVG> defaultSVG;
	std::shared_ptr<SVG> activeSVG;
	SVGWidget *sw;
	SVGButton();
	/** If `activeSVG` is NULL, `defaultSVG` is used as the active state instead. */
	void setSVGs(std::shared_ptr<SVG> defaultSVG, std::shared_ptr<SVG> activeSVG);
	void onDragStart(event::DragStart &e) override;
	void onDragEnd(event::DragEnd &e) override;
};


} // namespace rack
