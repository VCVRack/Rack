#pragma once
#include "common.hpp"
#include "MultiLightWidget.hpp"


namespace rack {


/** A MultiLightWidget that points to a module's Light or a range of lights
Will access firstLightId, firstLightId + 1, etc. for each added color
*/
struct ModuleLightWidget : MultiLightWidget {
	Module *module = NULL;
	int firstLightId;
	void step() override;
};


} // namespace rack
