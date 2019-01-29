#pragma once
#include "common.hpp"
#include "math.hpp"
#include <jansson.h>


namespace rack {


namespace app {
	struct ParamQuantity;
} // namespace app


struct ParamQuantityFactory {
	virtual ~ParamQuantityFactory() {}
	virtual app::ParamQuantity *create() = 0;
};


struct Param {
	/** Unstable API. Use set/getValue() instead. */
	float value = 0.f;

	float minValue = 0.f;
	float maxValue = 1.f;
	float defaultValue = 0.f;

	/** The name of the parameter in sentence capitalization
	e.g. "Frequency", "Pulse width", "Alternative mode"
	*/
	std::string label;
	/** The numerical unit of measurement
	Use a space before non-abbreviations to separate the numerical value.
	e.g. " semitones", "Hz", "%", "V"
	*/
	std::string unit;
	/** Set to 0 for linear, nonzero for exponential */
	float displayBase = 0.f;
	float displayMultiplier = 1.f;
	float displayOffset = 0.f;
	/** An optional one-sentence description of the parameter */
	std::string description;
	ParamQuantityFactory *paramQuantityFactory = NULL;
	/** Determines whether this param will be randomized automatically when the user requests to randomize the module state */
	bool randomizable = true;

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

	void setValue(float value) {
		this->value = math::clamp(value, minValue, maxValue);
	}

	bool isBounded();
	json_t *toJson();
	void fromJson(json_t *rootJ);
	void reset();
	void randomize();
};


} // namespace rack
