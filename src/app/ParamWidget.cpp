#include "app.hpp"
#include "engine.hpp"
#include "random.hpp"


namespace rack {


json_t *ParamWidget::toJson() {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "paramId", json_integer(paramId));

	// Infinite params should serialize to 0
	float v = (std::isfinite(minValue) && std::isfinite(maxValue)) ? value : 0.f;
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
	if (std::isfinite(minValue) && std::isfinite(maxValue)) {
		setValue(defaultValue);
	}
}

void ParamWidget::randomize() {
	// Infinite params should not be randomized
	if (randomizable && std::isfinite(minValue) && std::isfinite(maxValue)) {
		setValue(math::rescale(random::uniform(), 0.f, 1.f, minValue, maxValue));
	}
}

void ParamWidget::onButton(event::Button &e) {
	OpaqueWidget::onButton(e);
	if (e.target == this) {
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
			reset();
		}
	}
}

void ParamWidget::onChange(event::Change &e) {
	if (!module)
		return;

	if (smooth)
		engineSetParamSmooth(module, paramId, value);
	else
		engineSetParam(module, paramId, value);
}


} // namespace rack
