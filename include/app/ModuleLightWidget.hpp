#pragma once
#include <app/common.hpp>
#include <app/MultiLightWidget.hpp>
#include <engine/Module.hpp>


namespace rack {
namespace app {


/** A MultiLightWidget that points to a module's Light or a range of lights
Will access firstLightId, firstLightId + 1, etc. for each added color
*/
struct ModuleLightWidget : MultiLightWidget {
	engine::Module *module = NULL;
	int firstLightId;

	void step() override;
};


} // namespace app
} // namespace rack
