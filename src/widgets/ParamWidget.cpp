#include "../5V.hpp"


json_t *ParamWidget::toJson() {
	json_t *paramJ = json_object();

	json_t *valueJ = json_real(value);
	json_object_set_new(paramJ, "value", valueJ);

	return paramJ;
}

void ParamWidget::fromJson(json_t *root) {
	json_t *valueJ = json_object_get(root, "value");
	setValue(json_number_value(valueJ));
}

void ParamWidget::onMouseDown(int button) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		setValue(defaultValue);
	}
}

void ParamWidget::onChange() {
	assert(moduleWidget);
	assert(moduleWidget->module);
	// moduleWidget->module->params[paramId] = value;
	rackSetParamSmooth(moduleWidget->module, paramId, value);
}
