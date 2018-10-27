#include "widgets.hpp"

#ifdef USE_LOG_PRINTF
extern void log_printf(const char *logData, ...);
#undef Dprintf
#define Dprintf log_printf
#else
#define Dprintf printf
#endif // USE_LOG_PRINTF


namespace rack {

QuantityWidget::QuantityWidget() {
	EventChange e;
	onChange(e);
}

void QuantityWidget::setValue(float value) {
   // Dprintf("xxx QuantityWidget::setValue: value=%f\n", value);
   if(isfinite(minValue) && isfinite(maxValue))
   {
      // Dprintf("xxx QuantityWidget::setValue: isfinite value=%f\n", value);
      this->value = clamp(value, fminf(minValue, maxValue), fmaxf(minValue, maxValue));
      EventChange e;
      onChange(e);
   }
   else
   {
      // Dprintf("xxx QuantityWidget::setValue: !isfinite value=%f\n", value);
      // Rotary knob
      this->value = value;
      EventChange e;
      onChange(e);
   }
   // Dprintf("xxx QuantityWidget::setValue: LEAVE value=%f\n", value);
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
   // // revert_val = INVALID_REVERT_VAL;
}

} // namespace rack
