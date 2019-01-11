#pragma once
#include "common.hpp"


namespace rack {


struct ParamQuantity;


struct ParamQuantityFactory {
	virtual ~ParamQuantityFactory() {}
	virtual ParamQuantity *create() = 0;
};


struct ParamInfo {
	std::string label;
	std::string unit;
	/** Set to 0 for linear, nonzero for exponential */
	float displayBase = 0.f;
	float displayMultiplier = 1.f;
	std::string description;
	ParamQuantityFactory *paramQuantityFactory = NULL;

	~ParamInfo() {
		if (paramQuantityFactory)
			delete paramQuantityFactory;
	}

	template<class TParamQuantity = ParamQuantity>
	void config(std::string label = "", std::string unit = "", float displayBase = 0.f, float displayMultiplier = 1.f) {
		this->label = label;
		this->unit = unit;
		this->displayBase = displayBase;
		this->displayMultiplier = displayMultiplier;

		struct TParamQuantityFactory : ParamQuantityFactory {
			ParamQuantity *create() override {return new TParamQuantity;}
		};
		if (paramQuantityFactory)
			delete paramQuantityFactory;
		paramQuantityFactory = new TParamQuantityFactory;
	}
};


} // namespace rack
