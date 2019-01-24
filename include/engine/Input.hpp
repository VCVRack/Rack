#pragma once
#include "common.hpp"
#include "engine/Port.hpp"


namespace rack {


struct Input : Port {
	/** Returns the value if a cable is plugged in, otherwise returns the given default value */
	float normalize(float normalVoltage, int channel = 0) {
		return isActive() ? getVoltage(channel) : normalVoltage;
	}
};


} // namespace rack
