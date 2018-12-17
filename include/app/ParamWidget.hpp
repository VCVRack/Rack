#pragma once
#include "widgets/OpaqueWidget.hpp"
#include "app/ParamQuantity.hpp"
#include "app/common.hpp"


namespace rack {


/** Controls a ParamQuantity */
struct ParamWidget : OpaqueWidget {
	ParamQuantity *quantity;

	ParamWidget() {
		quantity = new ParamQuantity;
	}

	~ParamWidget() {
		delete quantity;
	}

	/** For legacy patch loading */
	void fromJson(json_t *rootJ);
	virtual void reset();
	virtual void randomize();
	void onButton(event::Button &e) override;
};


} // namespace rack
