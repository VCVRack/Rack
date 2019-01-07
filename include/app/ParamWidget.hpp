#pragma once
#include "app/common.hpp"
#include "widgets/OpaqueWidget.hpp"
#include "ui/Tooltip.hpp"
#include "ui/Quantity.hpp"


namespace rack {


struct ParamWidget : OpaqueWidget {
	Quantity *quantity = NULL;
	float dirtyValue = NAN;
	Tooltip *tooltip = NULL;

	~ParamWidget();
	void step() override;
	/** For legacy patch loading */
	void fromJson(json_t *rootJ);
	void onButton(const event::Button &e) override;
	void onEnter(const event::Enter &e) override;
	void onLeave(const event::Leave &e) override;
};


} // namespace rack
