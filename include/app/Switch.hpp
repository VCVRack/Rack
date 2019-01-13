#pragma once
#include "app/common.hpp"
#include "app/ParamWidget.hpp"


namespace rack {


/** A ParamWidget that controls  */
struct Switch : ParamWidget {
	/** Return to original position when released */
	bool momentary = false;

	void onDragStart(const event::DragStart &e) override;
	void onDragEnd(const event::DragEnd &e) override;
};


} // namespace rack
