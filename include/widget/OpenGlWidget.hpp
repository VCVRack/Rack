#pragma once
#include <widget/FramebufferWidget.hpp>


namespace rack {
namespace widget {


/** A FramebufferWidget that can be drawn on with OpenGL commands */
struct OpenGlWidget : FramebufferWidget {
	/** Draws every frame by default
	Override this and call `FramebufferWidget::step()` to restore the default behavior of FramebufferWidget.
	*/
	void step() override;
	/** Draws to the framebuffer.
	Override to initialize, draw, and flush the OpenGL state.
	*/
	void drawFramebuffer() override;
};


} // namespace widget
} // namespace rack
