#pragma once
#include "ui/Quantity.hpp"
#include "engine/Module.hpp"
#include "engine/Param.hpp"


namespace rack {


/** A Quantity that wraps an engine Param */
struct ParamQuantity : Quantity {
	Module *module = NULL;
	int paramId = 0;
	/** Use engine smoothing of Param values */
	bool smooth = false;
	/** Snap to the nearest integer */
	bool snap = false;
	float snapValue = 0.f;

	Param *getParam();
	void commitSnap();

	void setValue(float value) override;
	float getValue() override;
	float getMinValue() override;
	float getMaxValue() override;
	float getDefaultValue() override;
	float getDisplayValue() override;
	void setDisplayValue(float displayValue) override;
	int getDisplayPrecision() override;
	std::string getLabel() override;
	std::string getUnit() override;
};


} // namespace rack
