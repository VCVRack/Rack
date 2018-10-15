#pragma once
#include "common.hpp"
#include "Port.hpp"


namespace rack {


/** If you don't add these to your ModuleWidget, they will fall out of the rack... */
struct SVGScrew : FramebufferWidget {
	SVGWidget *sw;

	SVGScrew();
};


} // namespace rack
