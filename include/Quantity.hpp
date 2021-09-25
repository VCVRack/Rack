#pragma once
#include <common.hpp>
#include <math.hpp>
#include <random.hpp>


namespace rack {


/** A controller for manipulating a float value (which subclasses must store somehow) with limits and labels.

Often used as a decorator component for `widget::Widget`s that read or write a quantity.
*/
struct Quantity {
	virtual ~Quantity() {}

	/** Sets the value directly.
	Override this to change the state of your subclass to represent the new value.
	*/
	virtual void setValue(float value) {}

	/** Returns the value.
	Override this to return the state of your subclass.
	*/
	virtual float getValue() {
		return 0.f;
	}

	/** Returns the minimum recommended value. */
	virtual float getMinValue() {
		return 0.f;
	}

	/** Returns the maximum recommended value. */
	virtual float getMaxValue() {
		return 1.f;
	}

	/** Returns the default value, for resetting. */
	virtual float getDefaultValue() {
		return 0.f;
	}

	/** Returns the value, transformed for displaying.
	Useful for logarithmic scaling, multiplying by 100 for percentages, etc.
	*/
	virtual float getDisplayValue();

	/** Inversely transforms the display value and sets the value. */
	virtual void setDisplayValue(float displayValue);

	/** The number of total decimal places for generating the display value string. */
	virtual int getDisplayPrecision();

	/** Returns a string representation of the display value. */
	virtual std::string getDisplayValueString();

	/** Sets the value from a display string. */
	virtual void setDisplayValueString(std::string s);

	/** The name of the quantity. */
	virtual std::string getLabel() {
		return "";
	}

	/** The unit abbreviation of the quantity.
	Include an initial space character if you want a space after the number, e.g. "440 Hz". This allows space-less units, like "100%".
	*/
	virtual std::string getUnit() {
		return "";
	}

	/** Returns a string representation of the quantity. */
	virtual std::string getString();

	/** Resets the value to the default value. */
	virtual void reset();

	/** Sets the value to a uniform random value between the bounds. */
	virtual void randomize();

	// Helper methods

	/** Checks whether the value is at the min value. */
	bool isMin();
	/** Checks whether the value is at the max value. */
	bool isMax();
	/** Sets the value to the min value. */
	void setMin();
	/** Sets the value to the max value. */
	void setMax();
	/** Sets the value to max if the current value is min, otherwise sets the value to min. */
	void toggle();
	/** Adds an amount to the value. */
	void moveValue(float deltaValue);

	/** The difference between the max and min values. */
	float getRange();
	/** Checks whether the bounds are finite. */
	bool isBounded();
	/** Transforms a value to the range 0 to 1. */
	float toScaled(float value);
	/** Transforms a value from the range 0 to 1. */
	float fromScaled(float scaledValue);
	/** Sets value from the range 0 to 1. */
	void setScaledValue(float scaledValue);
	/** Returns the value scaled to the range 0 to 1. */
	float getScaledValue();
	/** Adds an amount to the value scaled to the range 0 to 1. */
	void moveScaledValue(float deltaScaledValue);
};


} // namespace rack
