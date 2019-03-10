#pragma once
#include "app/common.hpp"
#include "widget/OpaqueWidget.hpp"
#include "ui/Tooltip.hpp"
#include "app/ParamQuantity.hpp"
#include "history.hpp"


namespace rack {
namespace app {


/** Manages an engine::Param on a ModuleWidget. */
struct ParamWidget : widget::OpaqueWidget {
	ParamQuantity *paramQuantity = NULL;
	float dirtyValue = NAN;
	ui::Tooltip *tooltip = NULL;

	~ParamWidget();
	void step() override;
	void draw(const DrawArgs &args) override;
	void onButton(const widget::ButtonEvent &e) override;
	void onDoubleClick(const widget::DoubleClickEvent &e) override;
	void onEnter(const widget::EnterEvent &e) override;
	void onLeave(const widget::LeaveEvent &e) override;

	/** For legacy patch loading */
	void fromJson(json_t *rootJ);
	void createContextMenu();
	void resetAction();
	virtual void reset() {}
	virtual void randomize() {}
};


} // namespace app
} // namespace rack
