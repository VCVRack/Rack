#pragma once
#include "app/common.hpp"
#include "widgets/OpaqueWidget.hpp"
#include "ui/Tooltip.hpp"
#include "app/ParamQuantity.hpp"
#include "history.hpp"


namespace rack {


struct ParamWidget : OpaqueWidget {
	ParamQuantity *paramQuantity = NULL;
	float dirtyValue = NAN;
	Tooltip *tooltip = NULL;
	bool snap = false;

	~ParamWidget();
	void step() override;
	/** For legacy patch loading */
	void fromJson(json_t *rootJ);
	void onButton(const event::Button &e) override;
	void onEnter(const event::Enter &e) override;
	void onLeave(const event::Leave &e) override;
};


} // namespace rack
