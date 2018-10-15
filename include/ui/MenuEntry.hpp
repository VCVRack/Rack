#pragma once
#include "common.hpp"


namespace rack {


struct MenuEntry : OpaqueWidget {
	MenuEntry() {
		box.size = Vec(0, BND_WIDGET_HEIGHT);
	}
};


} // namespace rack
