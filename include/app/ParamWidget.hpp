#pragma once
#include <app/common.hpp>
#include <widget/OpaqueWidget.hpp>
#include <ui/Tooltip.hpp>
#include <engine/ParamQuantity.hpp>
#include <history.hpp>


namespace rack {
namespace app {


/** Manages an engine::Param on a ModuleWidget. */
struct ParamWidget : widget::OpaqueWidget {
	engine::ParamQuantity* paramQuantity = NULL;
	float dirtyValue = NAN;
	ui::Tooltip* tooltip = NULL;

	void step() override;
	void draw(const DrawArgs& args) override;

	void onButton(const event::Button& e) override;
	void onDoubleClick(const event::DoubleClick& e) override;
	void onEnter(const event::Enter& e) override;
	void onLeave(const event::Leave& e) override;

	/** For legacy patch loading */
	void fromJson(json_t* rootJ);
	void createContextMenu();
	void resetAction();
	virtual void reset() {}
	virtual void randomize() {}
};


} // namespace app
} // namespace rack
