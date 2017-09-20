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
	setValue(rescalef(randomf(), 0.0, 1.0, minValue, maxValue));
}

void ParamWidget::onMouseDownOpaque(int button) {
	if (button == 1) {
		setValue(defaultValue);
	}
}

void ParamWidget::onChange() {
	if (!module)
		return;

	module->params[paramId] = value;
}


} // namespace rack
