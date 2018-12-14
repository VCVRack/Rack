#pragma once
#include "ui/ScrollWidget.hpp"
#include "app/common.hpp"


namespace rack {


struct RackScrollWidget : ScrollWidget {
	void step() override;
};


} // namespace rack
