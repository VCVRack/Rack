#include "widgets.hpp"


namespace rack {

QuantityWidget::QuantityWidget() {
	EventChange e;
	onChange(e);
}

void QuantityWidget::setValue(float value) {
   // printf("xxx QuantityWidget::setValue: value=%f\n", value);
   if(isfinite(minValue) && isfinite(maxValue))
   {
      this->value = clamp(value, fminf(minValue, maxValue), fmaxf(minValue, maxValue));
      EventChange e;
      onChange(e);
   }
   else
   {
      // Rotary knob
      this->value = value;
      EventChange e;
      onChange(e);
   }
   // printf("xxx QuantityWidget::setValue: LEAVE value=%f\n", value);
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

void QuantityWidget::onMouseLeave(EventMouseLeave &e) {
   revert_val = INVALID_REVERT_VAL;
}

} // namespace rack
