#pragma once
#include "app/common.hpp"
#include "widgets/Widget.hpp"


namespace rack {


struct PluginManagerWidget : VirtualWidget {
	Widget *loginWidget;
	Widget *manageWidget;
	Widget *downloadWidget;
	PluginManagerWidget();
	void step() override;
};


} // namespace rack
