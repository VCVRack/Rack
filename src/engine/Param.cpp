#include "engine/Param.hpp"
#include "random.hpp"
#include "math.hpp"


namespace rack {


bool Param::isBounded() {
	return std::isfinite(minValue) && std::isfinite(maxValue);
}

json_t *Param::toJson() {
	json_t *rootJ = json_object();

	// Infinite params should serialize to 0
	if (isBounded()) {
		json_object_set_new(rootJ, "value", json_real(value));
	}

	return rootJ;
}

void Param::fromJson(json_t *rootJ) {
	if (isBounded()) {
		json_t *valueJ = json_object_get(rootJ, "value");
		if (valueJ)
			value = json_number_value(valueJ);
	}
}

void Param::reset() {
	if (isBounded()) {
		value = defaultValue;
	}
}

void Param::randomize() {
	if (isBounded()) {
		value = math::rescale(random::uniform(), 0.f, 1.f, minValue, maxValue);
	}
}


} // namespace rack
