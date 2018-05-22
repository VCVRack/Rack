#include "app.hpp"
#include "engine.hpp"


namespace rack {


json_t *ParamWidget::toJson() {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "paramId", json_integer(paramId));

	// Infinite params should serialize to 0
	float v = (isfinite(minValue) && isfinite(maxValue)) ? value : 0.f;
	json_object_set_new(rootJ, "value", json_real(v));
	return rootJ;
}

void ParamWidget::fromJson(json_t *rootJ) {
	json_t *valueJ = json_object_get(rootJ, "value");
	if (valueJ)
		setValue(json_number_value(valueJ));
}

void ParamWidget::reset() {
	// Infinite params should not be reset
	if (isfinite(minValue) && isfinite(maxValue)) {
		setValue(defaultValue);
	}
}

void ParamWidget::randomize() {
	// Infinite params should not be randomized
	if (randomizable && isfinite(minValue) && isfinite(maxValue)) {
		setValue(rescale(randomUniform(), 0.f, 1.f, minValue, maxValue));
	}
}

void ParamWidget::onMouseDown(EventMouseDown &e) {
	if (e.button == 1) {
		reset();
	}
	e.consumed = true;
	e.target = this;
}

void ParamWidget::onChange(EventChange &e) {
	if (!module)
		return;

	if (smooth)
		engineSetParamSmooth(module, paramId, value);
	else
		engineSetParam(module, paramId, value);
}


} // namespace rack
