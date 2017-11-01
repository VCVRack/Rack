#include "app.hpp"
#include "engine.hpp"


namespace rack {


json_t *ParamWidget::toJson() {
	json_t *paramJ = json_real(value);
	return paramJ;
}

void ParamWidget::fromJson(json_t *rootJ) {
	setValue(json_number_value(rootJ));
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
