#include <Quantity.hpp>
#include <string.hpp>
#include <tinyexpr.h>


namespace rack {


int Quantity::getDisplayPrecision() {
	return 5;
}

std::string Quantity::getDisplayValueString() {
	return string::f("%.*g", getDisplayPrecision(), math::normalizeZero(getDisplayValue()));
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
	if (!label.empty())
		s += label + ": ";
	s += getDisplayValueString();
	s += getUnit();
	return s;
}


} // namespace rack
