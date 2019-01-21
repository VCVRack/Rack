#pragma once
#include "widgets/FramebufferWidget.hpp"


namespace rack {


struct GLWidget : FramebufferWidget {
	void step() override;
	void drawFramebuffer() override;
};


} // namespace rack
