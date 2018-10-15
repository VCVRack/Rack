#pragma once
#include "widgets/Widget.hpp"


namespace rack {


/** A Widget representing a float value */
struct QuantityWidget : virtual Widget {
	float value = 0.0;
	float minValue = 0.0;
	float maxValue = 1.0;
	float defaultValue = 0.0;
	std::string label;
	/** Include a space character if you want a space after the number, e.g. " Hz" */
	std::string unit;
	/** The decimal place to round for displaying values.
	A precision of 2 will display as "1.00" for example.
	*/
	int precision = 2;

	void reset() {
		setValue(defaultValue);
	}

	void setValue(float value) {
		this->value = clampBetween(value, minValue, maxValue);
		event::Change e;
		onChange(e);
	}

	void setLimits(float minValue, float maxValue) {
		this->minValue = minValue;
		this->maxValue = maxValue;
		setValue(value);
	}

	void setDefaultValue(float defaultValue) {
		this->defaultValue = defaultValue;
	}

	/** Generates the display value */
	std::string getText() {
		return string::f("%s: %.*f%s", label.c_str(), precision, value, unit.c_str());
	}
};


} // namespace rack
