#pragma once
#include "app/common.hpp"
#include "ui/ScrollWidget.hpp"


namespace rack {


struct RackScrollWidget : ScrollWidget {
	void step() override;
};


} // namespace rack
