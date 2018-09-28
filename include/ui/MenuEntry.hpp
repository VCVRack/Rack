#pragma once

#include "widgets.hpp"


namespace rack {


struct MenuEntry : OpaqueWidget {
	MenuEntry() {
		box.size = math::Vec(0, BND_WIDGET_HEIGHT);
	}
};


} // namespace rack
