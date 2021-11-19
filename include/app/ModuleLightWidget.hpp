#pragma once
#include <app/common.hpp>
#include <app/MultiLightWidget.hpp>
#include <ui/Tooltip.hpp>
#include <engine/Module.hpp>


namespace rack {
namespace app {


/** A MultiLightWidget that points to a module's Light or a range of lights
Will access firstLightId, firstLightId + 1, etc. for each added color
*/
struct ModuleLightWidget : MultiLightWidget {
	struct Internal;
	Internal* internal;

	engine::Module* module = NULL;
	int firstLightId = -1;

	ModuleLightWidget();
	~ModuleLightWidget();
	engine::Light* getLight(int colorId);
	engine::LightInfo* getLightInfo();
	void createTooltip();
	void destroyTooltip();

	void step() override;
	void onHover(const HoverEvent& e) override;
	void onEnter(const EnterEvent& e) override;
	void onLeave(const LeaveEvent& e) override;
};


} // namespace app
} // namespace rack
