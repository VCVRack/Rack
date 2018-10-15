#pragma once
#include "common.hpp"


namespace rack {


struct PluginManagerWidget : virtual Widget {
	Widget *loginWidget;
	Widget *manageWidget;
	Widget *downloadWidget;
	PluginManagerWidget();
	void step() override;
};


} // namespace rack
