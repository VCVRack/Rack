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
	engine::Module* module = NULL;
	int firstLightId;

	ui::Tooltip* tooltip = NULL;

	~ModuleLightWidget();
	engine::Light* getLight(int colorId);
	engine::LightInfo* getLightInfo();
	void createTooltip();
	void destroyTooltip();

	void step() override;
	void onHover(const event::Hover& e) override;
	void onEnter(const event::Enter& e) override;
	void onLeave(const event::Leave& e) override;
};


} // namespace app
} // namespace rack
