#include "engine/Param.hpp"


namespace rack {


json_t *Param::toJson() {
	json_t *rootJ = json_object();

	// Infinite params should serialize to 0
	float v = (std::isfinite(minValue) && std::isfinite(maxValue)) ? value : 0.f;
	json_object_set_new(rootJ, "value", json_real(v));

	return rootJ;
}

void Param::fromJson(json_t *rootJ) {
	json_t *valueJ = json_object_get(rootJ, "value");
	if (valueJ)
		value = json_number_value(valueJ);
}


} // namespace rack
