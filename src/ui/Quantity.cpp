#include "ui/Quantity.hpp"
#include "string.hpp"


namespace rack {


std::string Quantity::getDisplayValueString() {
	return string::f("%.*f", getDisplayPrecision(), math::normalizeZero(getDisplayValue()));
}

void Quantity::setDisplayValueString(std::string s) {
	float displayValue = 0.f;
	int n = std::sscanf(s.c_str(), "%f", &displayValue);
	if (n == 1)
		setDisplayValue(displayValue);
}

std::string Quantity::getString() {
	std::string s;
	std::string label = getLabel();
	if (!label.empty())
		s += label + ": ";
	s += getDisplayValueString() + getUnit();
	return s;
}


} // namespace rack
