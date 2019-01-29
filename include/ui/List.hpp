#pragma once
#include "widget/OpaqueWidget.hpp"
#include "ui/common.hpp"


namespace rack {
namespace ui {


struct List : widget::OpaqueWidget {
	void step() override;
};


} // namespace ui
} // namespace rack
