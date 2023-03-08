#pragma once
#include <vector>

#include <jansson.h>

#include <Quantity.hpp>
#include <engine/Param.hpp>


namespace rack {
namespace engine {


struct Module;


/** A Quantity that wraps an engine::Param.

If `smoothEnabled` is true, all methods access/modify the Param's target value of the Engine's per-sample smoothing algorithm instead of the immediate value.
*/
struct ParamQuantity : Quantity {
	Module* module = NULL;
	int paramId = -1;

	/** The minimum allowed value. */
	float minValue = 0.f;
	/** The maximum allowed value. Must be greater than minValue. */
	float maxValue = 1.f;
	/** The initial value. */
	float defaultValue = 0.f;

	/** The name of the parameter, using sentence capitalization.
	e.g. "Frequency", "Pulse width", "Alternative mode"
	*/
	std::string name;
	/** The numerical unit of measurement appended to the value.
	Unit words and abbreviations should have a space to separate the numerical value from the number (e.g. " semitones", " V", " ms").
	Unit symbols should have no space (e.g. "%", "º").
	*/
	std::string unit;
	/** Set to 0 for linear, positive for exponential, negative for logarithmic.
	The formula is \f$displayValue = f(value) * displayMultiplier + displayOffset\f$ where \f$f(value)\f$ is
	- \f$value\f$ for \f$displayBase = 0\f$.
	- \f$\log_{-displayBase}(value)\f$ for \f$displayBase < 0\f$.
	- \f$displayBase^{value}\f$ for \f$displayBase > 0\f$.
	*/
	float displayBase = 0.f;
	float displayMultiplier = 1.f;
	float displayOffset = 0.f;
	/** Number of digits of precision to display.
	With displayPrecision = 5 for example, numbers will print as 12.345, 0.12345, 1.2345e6, or 1.2345e-6.
	*/
	int displayPrecision = 5;
	/** An optional one-sentence description of the parameter. */
	std::string description;

	/** Enables parameter resetting when the module or parameter itself is reset.
	*/
	bool resetEnabled = true;
	/** Enables parameter randomization when the module is randomized.
	Unbounded (infinite) parameters are not randomizable, regardless of this setting.
	*/
	bool randomizeEnabled = true;
	/** Enables per-sample Engine parameter smoothing when setSmoothValue() is called. */
	bool smoothEnabled = false;
	/** Rounds values to the nearest integer. */
	bool snapEnabled = false;

	Param* getParam();

	/** Sets the target value of the Engine's smoothing algorithm, or immediate value if smoothing is disabled.

	Before Rack 2.3.0, this always set the Param's immediate value.
	For this behavior, use `setImmediateValue()` instead.
	*/
	void setValue(float value) override;
	/** Gets the Param's smoothing target value, or immediate value if smoothing is disabled.

	Before Rack 2.3.0, this always got the Param's immediate value.
	For this behavior, use `getImmediateValue()` instead.
	*/
	float getValue() override;
	/** Sets the Param's value immediately without smoothing.

	If the Param's value is currently being smoothed by the Engine, smoothing is canceled.
	*/
	void setImmediateValue(float value);
	/** Gets the Param's value post-smoothing.

	If (and only if) the Param's value is currently being smoothed by the Engine, the return value is different than getValue().
	*/
	float getImmediateValue();

	float getMinValue() override;
	float getMaxValue() override;
	float getDefaultValue() override;
	float getDisplayValue() override;
	/** Always sets immediate value. */
	void setDisplayValue(float displayValue) override;
	std::string getDisplayValueString() override;
	/** Always sets immediate value. */
	void setDisplayValueString(std::string s) override;
	int getDisplayPrecision() override;
	std::string getLabel() override;
	std::string getUnit() override;
	void reset() override;
	void randomize() override;

	virtual std::string getDescription();

	virtual json_t* toJson();
	virtual void fromJson(json_t* rootJ);

	/** Deprecated. Use setValue() instead, which is identical since Rack 2.3.0. */
	DEPRECATED void setSmoothValue(float value);
	/** Deprecated. Use getValue() instead, which is identical since Rack 2.3.0. */
	DEPRECATED float getSmoothValue();
};


struct SwitchQuantity : ParamQuantity {
	std::vector<std::string> labels;

	std::string getDisplayValueString() override;
	void setDisplayValueString(std::string s) override;
};


} // namespace engine
} // namespace rack
