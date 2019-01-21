#pragma once
#include "widgets/FramebufferWidget.hpp"


namespace rack {


struct GLWidget : FramebufferWidget {
	/** Draws every frame by default
	Override this to restore the default behavior of FramebufferWidget.
	*/
	void step() override;
	void drawFramebuffer() override;
};


} // namespace rack
