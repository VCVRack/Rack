#pragma once
#include "app/common.hpp"
#include "app/ParamWidget.hpp"


namespace rack {
namespace app {


/** A ParamWidget that controls  */
struct Switch : ParamWidget {
	/** Return to original position when released */
	bool momentary = false;
	/** Hysteresis state for momentary switch */
	bool momentaryPressed = false;
	bool momentaryReleased = false;

	void step() override;
	void onDoubleClick(const event::DoubleClick &e) override;
	void onDragStart(const event::DragStart &e) override;
	void onDragEnd(const event::DragEnd &e) override;
	void reset() override;
	void randomize() override;
};


} // namespace app
} // namespace rack
