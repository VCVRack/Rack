#pragma once
#include <app/common.hpp>
#include <widget/OpaqueWidget.hpp>
#include <ui/Tooltip.hpp>
#include <ui/Menu.hpp>
#include <engine/ParamQuantity.hpp>


namespace rack {
namespace app {


/** Manages an engine::Param on a ModuleWidget. */
struct ParamWidget : widget::OpaqueWidget {
	struct Internal;
	Internal* internal;

	engine::Module* module = NULL;
	int paramId = -1;

	ParamWidget();
	~ParamWidget();
	/** Configures ParamQuantity properties based on the type of ParamWidget.
	This seems a bit hacky, but it's easier than requiring plugin developers to set `ParamQuantity::randomizeEnabled`, etc.
	*/
	virtual void initParamQuantity() {}
	engine::ParamQuantity* getParamQuantity();
	void createTooltip();
	void destroyTooltip();

	void step() override;
	void draw(const DrawArgs& args) override;

	void onButton(const ButtonEvent& e) override;
	void onDoubleClick(const DoubleClickEvent& e) override;
	void onEnter(const EnterEvent& e) override;
	void onLeave(const LeaveEvent& e) override;

	void createContextMenu();
	/** Override to add custom menu items at the bottom of the parameter context menu.
	It is recommended to add a MenuSeparator before other menu items.

		menu->addChild(new MenuSeparator);
	*/
	virtual void appendContextMenu(ui::Menu* menu) {}
	void resetAction();
};


} // namespace app
} // namespace rack
