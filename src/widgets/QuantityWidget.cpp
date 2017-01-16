#include "rack.hpp"


namespace rack {

QuantityWidget::QuantityWidget() {
	onChange();
}

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

std::string QuantityWidget::getText() {
	std::string text = label;
	text += ": ";
	char valueStr[128];
	if (precision >= 0) {
		float factor = powf(10.0, precision);
		float v = roundf(value / factor) * factor;
		snprintf(valueStr, sizeof(valueStr), "%.0f", v);
	}
	else {
		snprintf(valueStr, sizeof(valueStr), "%.*f", -precision, value);
	}
	text += valueStr;
	text += unit;
	return text;
}


} // namespace rack
