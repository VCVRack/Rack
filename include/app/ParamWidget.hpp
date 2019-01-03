#pragma once
#include "app/common.hpp"
#include "widgets/OpaqueWidget.hpp"
#include "ui/Quantity.hpp"


namespace rack {


struct ParamWidget : OpaqueWidget {
	Quantity *quantity = NULL;
	float dirtyValue = NAN;

	~ParamWidget() {
		if (quantity)
			delete quantity;
	}

	void step() override;
	/** For legacy patch loading */
	void fromJson(json_t *rootJ);
	void onButton(event::Button &e) override;
	void onDragMove(event::DragMove &e) override;
};


} // namespace rack
