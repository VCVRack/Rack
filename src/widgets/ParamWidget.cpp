#include "Rack.hpp"


namespace rack {

json_t *ParamWidget::toJson() {
	json_t *paramJ = json_real(value);
	return paramJ;
}

void ParamWidget::fromJson(json_t *root) {
	setValue(json_number_value(root));
}

void ParamWidget::onMouseDown(int button) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		setValue(defaultValue);
	}
}

void ParamWidget::onChange() {
	assert(module);
	// moduleWidget->module->params[paramId] = value;
	rackSetParamSmooth(module, paramId, value);
}


} // namespace rack
