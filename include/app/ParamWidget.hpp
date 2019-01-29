#pragma once
#include "app/common.hpp"
#include "widget/OpaqueWidget.hpp"
#include "ui/Tooltip.hpp"
#include "app/ParamQuantity.hpp"
#include "history.hpp"


namespace rack {
namespace app {


struct ParamWidget : widget::OpaqueWidget {
	ParamQuantity *paramQuantity = NULL;
	float dirtyValue = NAN;
	ui::Tooltip *tooltip = NULL;

	~ParamWidget();
	void step() override;
	void draw(const widget::DrawContext &ctx) override;
	void onButton(const event::Button &e) override;
	void onEnter(const event::Enter &e) override;
	void onLeave(const event::Leave &e) override;

	/** For legacy patch loading */
	void fromJson(json_t *rootJ);
	void createContextMenu();
	void resetAction();
};


} // namespace app
} // namespace rack
