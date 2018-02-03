#include "widgets.hpp"


namespace rack {

QuantityWidget::QuantityWidget() {
	EventChange e;
	onChange(e);
}

void QuantityWidget::setValue(float value) {
	this->value = clamp(value, fminf(minValue, maxValue), fmaxf(minValue, maxValue));
	EventChange e;
	onChange(e);
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
	text += stringf("%.*f", precision, value);
	text += unit;
	return text;
}


} // namespace rack
