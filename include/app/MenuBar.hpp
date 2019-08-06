#pragma once
#include <app/common.hpp>
#include <widget/OpaqueWidget.hpp>


namespace rack {
namespace app {


struct MenuBar : widget::OpaqueWidget {
	void draw(const DrawArgs& args) override;
};


MenuBar* createMenuBar();


} // namespace app
} // namespace rack
