#pragma once
#include <math.hpp>


namespace rack {


/** A controller for manipulating a float value (which subclasses must store somehow) with limits and labels
Often used as a decorator component for widget::Widgets that read or write a quantity.
*/
struct Quantity {
	virtual ~Quantity() {}

	/** Sets the value directly
	Override this to change the state of your subclass to represent the new value.
	*/
	virtual void setValue(float value) {}

	/** Returns the value
	Override this to return the state of your subclass.
	*/
	virtual float getValue() {
		return 0.f;
	}

	/** Returns the minimum allowed value */
	virtual float getMinValue() {
		return 0.f;
	}

	/** Returns the maximum allowed value */
	virtual float getMaxValue() {
		return 1.f;
	}

	/** Returns the default value, for resetting */
	virtual float getDefaultValue() {
		return 0.f;
	}

	/** Returns the value, possibly transformed for displaying
	Useful for logarithmic scaling, multiplying by 100 for percentages, etc.
	*/
	virtual float getDisplayValue() {
		return getValue();
	}

	/** Inversely transforms the display value and sets the value */
	virtual void setDisplayValue(float displayValue) {
		setValue(displayValue);
	}

	/** The number of total decimal places for generating the display value string
	*/
	virtual int getDisplayPrecision();

	/** Returns a string representation of the display value */
	virtual std::string getDisplayValueString();

	virtual void setDisplayValueString(std::string s);

	/** The name of the quantity */
	virtual std::string getLabel() {
		return "";
	}

	/** The unit abbreviation of the quantity
	Include an initial space character if you want a space after the number, e.g. "440 Hz". This allows space-less units, like "100%".
	*/
	virtual std::string getUnit() {
		return "";
	}

	/** Returns a string representation of the quantity */
	virtual std::string getString();

	// Helper methods

	/** Resets the value to the default value */
	void reset() {
		setValue(getDefaultValue());
	}

	/** Checks whether the value is at the min value */
	bool isMin() {
		return getValue() <= getMinValue();
	}

	/** Checks whether the value is at the max value */
	bool isMax() {
		return getValue() >= getMaxValue();
	}

	/** Sets the value to the min value */
	void setMin() {
		setValue(getMinValue());
	}

	/** Sets the value to the max value */
	void setMax() {
		setValue(getMaxValue());
	}

	/** Sets value from the range 0 to 1 */
	void setScaledValue(float scaledValue) {
		setValue(math::rescale(scaledValue, 0.f, 1.f, getMinValue(), getMaxValue()));
	}

	/** Returns the value rescaled to the range 0 to 1 */
	float getScaledValue() {
		return math::rescale(getValue(), getMinValue(), getMaxValue(), 0.f, 1.f);
	}

	/** The difference between the max and min values */
	float getRange() {
		return getMaxValue() - getMinValue();
	}

	/** Checks whether the bounds are finite */
	bool isBounded() {
		return std::isfinite(getMinValue()) && std::isfinite(getMaxValue());
	}

	/** Adds an amount to the value */
	void moveValue(float deltaValue) {
		setValue(getValue() + deltaValue);
	}

	/** Adds an amount to the value scaled to the range 0 to 1 */
	void moveScaledValue(float deltaScaledValue) {
		moveValue(deltaScaledValue * getRange());
	}
};


} // namespace rack
