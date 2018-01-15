#include "app.hpp"
#include "engine.hpp"


namespace rack {


json_t *ParamWidget::toJson() {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "paramId", json_integer(paramId));
	json_object_set_new(rootJ, "value", json_real(value));
	return rootJ;
}

void ParamWidget::fromJson(json_t *rootJ) {
	json_t *valueJ = json_object_get(rootJ, "value");
	if (valueJ)
		setValue(json_number_value(valueJ));
}

void ParamWidget::randomize() {
	if (randomizable)
		setValue(rescalef(randomf(), 0.0, 1.0, minValue, maxValue));
}

void ParamWidget::onMouseDown(EventMouseDown &e) {
	if (e.button == 1) {
		setValue(defaultValue);
	}
	e.consumed = true;
	e.target = this;
}

void ParamWidget::onChange(EventChange &e) {
	if (!module)
		return;

	engineSetParam(module, paramId, value);
}


} // namespace rack
