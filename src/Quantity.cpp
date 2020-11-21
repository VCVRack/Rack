#include <tinyexpr.h>

#include <Quantity.hpp>
#include <string.hpp>


namespace rack {


float Quantity::getDisplayValue() {
	return getValue();
}

void Quantity::setDisplayValue(float displayValue) {
	setValue(displayValue);
}

int Quantity::getDisplayPrecision() {
	return 5;
}

std::string Quantity::getDisplayValueString() {
	float v = getDisplayValue();
	if (v == INFINITY)
		return "∞";
	else if (v == -INFINITY)
		return "-∞";
	else if (std::isnan(v))
		return "NaN";
	return string::f("%.*g", getDisplayPrecision(), math::normalizeZero(v));
}

void Quantity::setDisplayValueString(std::string s) {
	double result = te_interp(s.c_str(), NULL);
	if (std::isfinite(result)) {
		setDisplayValue(result);
	}
}

std::string Quantity::getString() {
	std::string s;
	std::string label = getLabel();
	std::string valueString = getDisplayValueString() + getUnit();
	s += label;
	if (label != "" && valueString != "")
		s += ": ";
	s += valueString;
	return s;
}

void Quantity::reset() {
	setValue(getDefaultValue());
}

void Quantity::randomize() {
	if (isBounded())
		setScaledValue(random::uniform());
}

bool Quantity::isMin() {
	return getValue() <= getMinValue();
}

bool Quantity::isMax() {
	return getValue() >= getMaxValue();
}

void Quantity::setMin() {
	setValue(getMinValue());
}

void Quantity::setMax() {
	setValue(getMaxValue());
}

void Quantity::toggle() {
	setValue(isMin() ? getMaxValue() : getMinValue());
}

void Quantity::setScaledValue(float scaledValue) {
	if (!isBounded())
		setValue(scaledValue);
	else
		setValue(math::rescale(scaledValue, 0.f, 1.f, getMinValue(), getMaxValue()));
}

float Quantity::getScaledValue() {
	if (!isBounded())
		return getValue();
	else if (getMinValue() == getMaxValue())
		return 0.f;
	else
		return math::rescale(getValue(), getMinValue(), getMaxValue(), 0.f, 1.f);
}

float Quantity::getRange() {
	return getMaxValue() - getMinValue();
}

bool Quantity::isBounded() {
	return std::isfinite(getMinValue()) && std::isfinite(getMaxValue());
}

void Quantity::moveValue(float deltaValue) {
	setValue(getValue() + deltaValue);
}

void Quantity::moveScaledValue(float deltaScaledValue) {
	if (!isBounded())
		moveValue(deltaScaledValue);
	else
		moveValue(deltaScaledValue * getRange());
}


} // namespace rack
