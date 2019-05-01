#pragma once
#include "app/common.hpp"
#include "widget/OpaqueWidget.hpp"


namespace rack {
namespace app {


struct MenuBar : widget::OpaqueWidget {
	MenuBar();
	void draw(const DrawArgs &args) override;
};


} // namespace app
} // namespace rack
