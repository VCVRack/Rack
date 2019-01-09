#include "app/ParamQuantity.hpp"


namespace rack {


Param *ParamQuantity::getParam() {
	assert(module);
	return &module->params[paramId];
}

ParamInfo *ParamQuantity::getParamInfo() {
	assert(module);
	return &module->paramInfos[paramId];
}

void ParamQuantity::commitSnap() {
	// TODO
}

void ParamQuantity::setValue(float value) {
	if (!module)
		return;
	value = math::clamp(value, getMinValue(), getMaxValue());
	// TODO Smooth
	// TODO Snap
	getParam()->value = value;
}

float ParamQuantity::getValue() {
	if (!module)
		return 0.f;
	return getParam()->value;
}

float ParamQuantity::getMinValue() {
	if (!module)
		return -1.f;
	return getParam()->minValue;
}

float ParamQuantity::getMaxValue() {
	if (!module)
		return 1.f;
	return getParam()->maxValue;
}

float ParamQuantity::getDefaultValue() {
	if (!module)
		return 0.f;
	return getParam()->defaultValue;
}

float ParamQuantity::getDisplayValue() {
	if (!module)
		return Quantity::getDisplayValue();
	if (getParamInfo()->displayBase == 0.f) {
		// Linear
		return getValue() * getParamInfo()->displayMultiplier;
	}
	else if (getParamInfo()->displayBase == 1.f) {
		// Fixed (special case of exponential)
		return getParamInfo()->displayMultiplier;
	}
	else {
		// Exponential
		return std::pow(getParamInfo()->displayBase, getValue()) * getParamInfo()->displayMultiplier;
	}
}

void ParamQuantity::setDisplayValue(float displayValue) {
	if (!module)
		return;
	if (getParamInfo()->displayBase == 0.f) {
		// Linear
		setValue(displayValue / getParamInfo()->displayMultiplier);
	}
	else if (getParamInfo()->displayBase == 1.f) {
		// Fixed
		setValue(getParamInfo()->displayMultiplier);
	}
	else {
		// Exponential
		setValue(std::log(displayValue / getParamInfo()->displayMultiplier) / std::log(getParamInfo()->displayBase));
	}
}

int ParamQuantity::getDisplayPrecision() {
	if (!module)
		return Quantity::getDisplayPrecision();
	float displayValue = getDisplayValue();
	if (displayValue == 0.f)
		return 0;
	if (std::round(displayValue) == displayValue)
		return 0;
	float log = std::log10(std::abs(getDisplayValue()));
	return (int) std::ceil(math::clamp(-log + 3.f, 0.f, 6.f));
}

std::string ParamQuantity::getLabel() {
	if (!module)
		return Quantity::getLabel();
	return getParamInfo()->label;
}

std::string ParamQuantity::getUnit() {
	if (!module)
		return Quantity::getUnit();
	return getParamInfo()->unit;
}


} // namespace rack
