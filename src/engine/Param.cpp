#include "engine/Param.hpp"
#include "random.hpp"
#include "math.hpp"


namespace rack {


json_t *Param::toJson() {
	json_t *rootJ = json_object();

	float v = 0.f;
	// Infinite params should serialize to 0
	if (std::isfinite(minValue) && std::isfinite(maxValue))
		v = value;
	json_object_set_new(rootJ, "value", json_real(v));

	return rootJ;
}

void Param::fromJson(json_t *rootJ) {
	json_t *valueJ = json_object_get(rootJ, "value");
	if (valueJ)
		value = json_number_value(valueJ);
}

void Param::reset() {
	if (std::isfinite(minValue) && std::isfinite(maxValue)) {
		value = defaultValue;
	}
}

void Param::randomize() {
	if (std::isfinite(minValue) && std::isfinite(maxValue)) {
		value = math::rescale(random::uniform(), 0.f, 1.f, minValue, maxValue);
	}
}


} // namespace rack
