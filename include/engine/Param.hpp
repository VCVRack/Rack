#pragma once
#include "common.hpp"
#include "math.hpp"
#include <jansson.h>


namespace rack {


namespace app {
	struct ParamQuantity;
} // namespace app


namespace engine {


struct ParamQuantityFactory {
	virtual ~ParamQuantityFactory() {}
	virtual app::ParamQuantity *create() = 0;
};


struct Param {
	/** Unstable API. Use setValue() and getValue() instead. */
	float value = 0.f;

	/** The minimum allowed value. */
	float minValue = 0.f;
	/** The maximum allowed value. Must be greater than minValue. */
	float maxValue = 1.f;
	/** The initial value. */
	float defaultValue = 0.f;

	/** The name of the parameter, using sentence capitalization.
	e.g. "Frequency", "Pulse width", "Alternative mode"
	*/
	std::string label;
	/** The numerical unit of measurement appended to the value.
	Units that are words should have a space to separate the numerical value from the number (e.g. " semitones", " octaves").
	Unit abbreviations and symbols should have no space (e.g. "V", "ms", "%", "º").
	*/
	std::string unit;
	/** Set to 0 for linear, positive for exponential, negative for logarithmic. */
	float displayBase = 0.f;
	float displayMultiplier = 1.f;
	float displayOffset = 0.f;
	/** An optional one-sentence description of the parameter. */
	std::string description;
	ParamQuantityFactory *paramQuantityFactory = NULL;

	~Param() {
		if (paramQuantityFactory)
			delete paramQuantityFactory;
	}

	template<class TParamQuantity = app::ParamQuantity>
	void config(float minValue, float maxValue, float defaultValue, std::string label = "", std::string unit = "", float displayBase = 0.f, float displayMultiplier = 1.f, float displayOffset = 0.f) {
		this->value = defaultValue;
		this->minValue = minValue;
		this->maxValue = maxValue;
		this->defaultValue = defaultValue;
		if (!label.empty())
			this->label = label;
		this->unit = unit;
		this->displayBase = displayBase;
		this->displayMultiplier = displayMultiplier;
		this->displayOffset = displayOffset;

		struct TParamQuantityFactory : ParamQuantityFactory {
			app::ParamQuantity *create() override {return new TParamQuantity;}
		};
		if (paramQuantityFactory)
			delete paramQuantityFactory;
		paramQuantityFactory = new TParamQuantityFactory;
	}

	float getValue() {
		return value;
	}

	/* Clamps and sets the value. */
	void setValue(float value) {
		this->value = math::clamp(value, minValue, maxValue);
	}

	/** Returns whether the Param has finite range between minValue and maxValue. */
	bool isBounded() {
		return std::isfinite(minValue) && std::isfinite(maxValue);
	}

	json_t *toJson();
	void fromJson(json_t *rootJ);
};


} // namespace engine
} // namespace rack
