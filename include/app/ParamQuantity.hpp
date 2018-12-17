#pragma once
#include "ui/Quantity.hpp"
#include "engine/Module.hpp"


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

	Param *getParam() {
		assert(module);
		return &module->params[paramId];
	}

	void commitSnap() {
		// TODO
	}

	void setValue(float value) override {
		// TODO Smooth
		// TODO Snap
		getParam()->value = value;
	}

	float getValue() override {
		return getParam()->value;
	}

	float getMinValue() override {
		return getParam()->minValue;
	}

	float getMaxValue() override {
		return getParam()->maxValue;
	}

	float getDefaultValue() override {
		return getParam()->defaultValue;
	}

	float getDisplayValue() override {
		if (getParam()->displayBase == 0.f) {
			// Linear
			return getParam()->value * getParam()->displayMultiplier;
		}
		else {
			// Exponential
			return std::pow(getParam()->displayBase, getParam()->value) * getParam()->displayMultiplier;
		}
	}

	void setDisplayValue(float displayValue) override {
		if (getParam()->displayBase == 0.f) {
			// Linear
			getParam()->value = displayValue / getParam()->displayMultiplier;
		}
		else {
			// Exponential
			getParam()->value = std::log(displayValue / getParam()->displayMultiplier) / std::log(getParam()->displayBase);
		}
	}

	int getDisplayPrecision() override {
		return getParam()->displayPrecision;
	}

	std::string getLabel() override {
		return getParam()->label;
	}

	std::string getUnit() override {
		return getParam()->unit;
	}
};


} // namespace rack
