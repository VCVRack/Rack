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
	if (std::isnan(v))
		return "NaN";
	return string::f("%.*g", getDisplayPrecision(), math::normalizeZero(v));
}

/** Build-in variables for tinyexpr
Because formulas are lowercased for case-insensitivity, all variables must be lowercase.
*/
struct TeVariable {
	std::string name;
	double value;
};
static std::vector<TeVariable> teVariables;
static std::vector<te_variable> teVars;

static void teVarsInit() {
	if (!teVars.empty())
		return;

	// Math variables
	teVariables.push_back({"inf", INFINITY});

	// Note names
	struct Note {
		std::string name;
		int semi;
	};
	const static std::vector<Note> notes = {
		{"c", 0},
		{"d", 2},
		{"e", 4},
		{"f", 5},
		{"g", 7},
		{"a", 9},
		{"b", 11},
	};
	for (const Note& note : notes) {
		// Example: c, cs (or c#), and cb
		teVariables.push_back({string::f("%s", note.name.c_str()), 440.f * std::pow(2.0, (note.semi - 9) / 12.f)});
		teVariables.push_back({string::f("%ss", note.name.c_str()), 440.f * std::pow(2.0, (note.semi - 9 + 1) / 12.f)});
		teVariables.push_back({string::f("%sb", note.name.c_str()), 440.f * std::pow(2.0, (note.semi - 9 - 1) / 12.f)});
		for (int oct = 0; oct <= 9; oct++) {
			// Example: c4, cs4 (or c#4), and cb4
			teVariables.push_back({string::f("%s%d", note.name.c_str(), oct), 440.f * std::pow(2.0, oct - 4 + (note.semi - 9) / 12.f)});
			teVariables.push_back({string::f("%ss%d", note.name.c_str(), oct), 440.f * std::pow(2.0, oct - 4 + (note.semi - 9 + 1) / 12.f)});
			teVariables.push_back({string::f("%sb%d", note.name.c_str(), oct), 440.f * std::pow(2.0, oct - 4 + (note.semi - 9 - 1) / 12.f)});
		}
	}

	// Build teVars from teVariables
	teVars.reserve(teVariables.size());
	for (size_t i = 0; i < teVariables.size(); i++) {
		teVars.push_back({teVariables[i].name.c_str(), &teVariables[i].value, TE_VARIABLE, NULL});
	}

	// Add custom functions
	teVars.push_back({"log2", (void*) (double(*)(double)) std::log2, TE_FUNCTION1 | TE_FLAG_PURE, NULL});
}

void Quantity::setDisplayValueString(std::string s) {
	teVarsInit();

	// Uppercase letters aren't needed in formulas, so convert to lowercase in case user types INF or C4.
	s = string::lowercase(s);

	// Replace "#" with "s" so note names can be written as c#.
	std::replace(s.begin(), s.end(), '#', 's');

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
