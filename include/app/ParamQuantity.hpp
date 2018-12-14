#pragma once
#include "ui/Quantity.hpp"
#include "engine.hpp"


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
	std::string getLabel() override {
		return getParam()->label;
	}
	std::string getUnit() override {
		return getParam()->unit;
	}
	int getPrecision() override {
		return getParam()->precision;
	}
};


} // namespace rack
