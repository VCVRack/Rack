#pragma once
#include "common.hpp"
#include "engine/Port.hpp"


namespace rack {


struct Input : Port {
	/** Returns the value if a wire is plugged in, otherwise returns the given default value */
	float normalize(float normalVoltage, int index = 0) {
		return active ? getVoltage(index) : normalVoltage;
	}
};


} // namespace rack
