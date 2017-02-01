#include "scene.hpp"
#include "engine.hpp"


namespace rack {

json_t *ParamWidget::toJson() {
	json_t *paramJ = json_real(value);
	return paramJ;
}

void ParamWidget::fromJson(json_t *root) {
	setValue(json_number_value(root));
}

void ParamWidget::onMouseDown(int button) {
	if (button == 1) {
		setValue(defaultValue);
	}
}

void ParamWidget::onChange() {
	if (!module)
		return;

	// moduleWidget->module->params[paramId] = value;
	engineSetParamSmooth(module, paramId, value);
}


} // namespace rack
