#include <Quantity.hpp>
#include <string.hpp>


namespace rack {


int Quantity::getDisplayPrecision() {
	return 5;
}

std::string Quantity::getDisplayValueString() {
	return string::f("%.*g", getDisplayPrecision(), math::normalizeZero(getDisplayValue()));
}

void Quantity::setDisplayValueString(std::string s) {
	float v = 0.f;
	char suffix[2];
	int n = std::sscanf(s.c_str(), "%f%1s", &v, suffix);
	if (n >= 2) {
		// Parse SI prefixes
		switch (suffix[0]) {
			case 'n': v *= 1e-9f; break;
			case 'u': v *= 1e-6f; break;
			case 'm': v *= 1e-3f; break;
			case 'k': v *= 1e3f; break;
			case 'M': v *= 1e6f; break;
			case 'G': v *= 1e9f; break;
			default: break;
		}
	}
	if (n >= 1)
		setDisplayValue(v);
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
