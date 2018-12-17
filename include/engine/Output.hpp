#pragma once
#include "common.hpp"
#include "engine/Light.hpp"


namespace rack {


struct Output {
	/** Voltage of the port. Write-only by Module */
	float value = 0.f;
	/** Whether a wire is plugged in */
	bool active = false;
	Light plugLights[2];
};


} // namespace rack
