#include "../5V.hpp"


void QuantityWidget::setValue(float value) {
	this->value = clampf(value, minValue, maxValue);
	onChange();
}

void QuantityWidget::setLimits(float minValue, float maxValue) {
	this->minValue = minValue;
	this->maxValue = maxValue;
}

void QuantityWidget::setDefaultValue(float defaultValue) {
	this->defaultValue = defaultValue;
	setValue(defaultValue);
}
