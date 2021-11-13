#include <tinyexpr.h>

#include <Quantity.hpp>
#include <string.hpp>
// for C4 and A4 frequencies
#include <dsp/common.hpp>


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
	if (std::isnan(v))
		return "NaN";
	return string::f("%.*g", getDisplayPrecision(), math::normalizeZero(v));
}

/** Build-in variables for tinyexpr */
struct TeVariable {
	std::string name;
	double value;
};
static std::vector<TeVariable> teVariables;
static std::vector<te_variable> teVars;

static void teVarsInit() {
	if (!teVars.empty())
		return;

	// Add variables
	teVariables.push_back({"inf", INFINITY});
	teVariables.push_back({"c", dsp::FREQ_C4});
	teVariables.push_back({"a", dsp::FREQ_A4});
	for (int i = 0; i <= 8; i++)
		teVariables.push_back({string::f("c%d", i), dsp::FREQ_C4 * std::pow(2.0, i - 4)});
	for (int i = 0; i <= 8; i++)
		teVariables.push_back({string::f("a%d", i), dsp::FREQ_A4 * std::pow(2.0, i - 4)});

	// Build teVars
	teVars.resize(teVariables.size());
	for (size_t i = 0; i < teVariables.size(); i++) {
		teVars[i].name = teVariables[i].name.c_str();
		teVars[i].address = &teVariables[i].value;
		teVars[i].type = TE_VARIABLE;
		teVars[i].context = NULL;
	}
}

void Quantity::setDisplayValueString(std::string s) {
	teVarsInit();

	// Uppercase letters aren't needed in formulas, so convert to lowercase in case user types INF or C4.
	s = string::lowercase(s);

	// Compile string with tinyexpr
	te_expr* expr = te_compile(s.c_str(), teVars.data(), teVars.size(), NULL);
	if (!expr)
		return;

	double result = te_eval(expr);
	te_free(expr);
	if (std::isnan(result))
		return;

	setDisplayValue(result);
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

void Quantity::moveValue(float deltaValue) {
	setValue(getValue() + deltaValue);
}

float Quantity::getRange() {
	return getMaxValue() - getMinValue();
}

bool Quantity::isBounded() {
	return std::isfinite(getMinValue()) && std::isfinite(getMaxValue());
}

float Quantity::toScaled(float value) {
	if (!isBounded())
		return value;
	else if (getMinValue() == getMaxValue())
		return 0.f;
	else
		return math::rescale(value, getMinValue(), getMaxValue(), 0.f, 1.f);
}

float Quantity::fromScaled(float scaledValue) {
	if (!isBounded())
		return scaledValue;
	else
		return math::rescale(scaledValue, 0.f, 1.f, getMinValue(), getMaxValue());
}

void Quantity::setScaledValue(float scaledValue) {
	setValue(fromScaled(scaledValue));
}

float Quantity::getScaledValue() {
	return toScaled(getValue());
}

void Quantity::moveScaledValue(float deltaScaledValue) {
	if (!isBounded())
		moveValue(deltaScaledValue);
	else
		moveValue(deltaScaledValue * getRange());
}


} // namespace rack
