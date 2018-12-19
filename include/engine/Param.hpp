#pragma once
#include "common.hpp"
#include <jansson.h>


namespace rack {


struct Param {
	float value = 0.f;
	float minValue = 0.f;
	float maxValue = 1.f;
	float defaultValue = 0.f;

	// For formatting/displaying the value
	/** Set to 0 for linear, nonzero for exponential */
	float displayBase = 0.f;
	float displayMultiplier = 1.f;
	int displayPrecision = 2;
	std::string label;
	std::string unit;

	void setup(float minValue, float maxValue, float defaultValue, std::string label = "", std::string unit = "", int displayPrecision = 2, float displayBase = 0.f, float displayMultiplier = 1.f) {
		this->value = defaultValue;
		this->minValue = minValue;
		this->maxValue = maxValue;
		this->defaultValue = defaultValue;
		this->label = label;
		this->unit = unit;
		this->displayPrecision = displayPrecision;
		this->displayBase = displayBase;
		this->displayMultiplier = displayMultiplier;
	}

	json_t *toJson();
	void fromJson(json_t *rootJ);
};


} // namespace rack
