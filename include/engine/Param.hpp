#pragma once
#include <common.hpp>
#include <math.hpp>


namespace rack {
namespace engine {


struct Param {
	/** Unstable API. Use setValue() and getValue() instead. */
	float value = 0.f;

	float getValue() {
		return value;
	}

	void setValue(float value) {
		this->value = value;
	}
};


} // namespace engine
} // namespace rack
