#pragma once
#include "app/common.hpp"
#include "widgets/OpaqueWidget.hpp"
#include "ui/Quantity.hpp"


namespace rack {


struct ParamWidget : OpaqueWidget {
	Quantity *quantity = NULL;

	~ParamWidget() {
		if (quantity)
			delete quantity;
	}

	/** For legacy patch loading */
	void fromJson(json_t *rootJ);
	virtual void reset();
	virtual void randomize();
	void onButton(event::Button &e) override;
};


} // namespace rack
