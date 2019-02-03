#pragma once
#include "widget/FramebufferWidget.hpp"


namespace rack {
namespace widget {


struct OpenGlWidget : FramebufferWidget {
	/** Draws every frame by default
	Override this to restore the default behavior of FramebufferWidget.
	*/
	void step() override;
	void drawFramebuffer() override;
};


} // namespace widget
} // namespace rack
