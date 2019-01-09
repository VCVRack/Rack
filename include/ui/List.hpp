#pragma once
#include "widgets/OpaqueWidget.hpp"
#include "ui/common.hpp"


namespace rack {


struct List : OpaqueWidget {
	void step() override;
};


} // namespace rack
