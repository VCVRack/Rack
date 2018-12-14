#pragma once
#include "widgets/Widget.hpp"
#include "app/common.hpp"


namespace rack {


struct PluginManagerWidget : VirtualWidget {
	Widget *loginWidget;
	Widget *manageWidget;
	Widget *downloadWidget;
	PluginManagerWidget();
	void step() override;
};


} // namespace rack
