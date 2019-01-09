#pragma once
#include "common.hpp"
#include <jansson.h>


namespace rack {


struct Param {
	float value = 0.f;
	float minValue = 0.f;
	float maxValue = 1.f;
	float defaultValue = 0.f;

	void config(float minValue, float maxValue, float defaultValue) {
		this->value = defaultValue;
		this->minValue = minValue;
		this->maxValue = maxValue;
		this->defaultValue = defaultValue;
	}

	json_t *toJson();
	void fromJson(json_t *rootJ);
	void reset();
	void randomize();
};


} // namespace rack
