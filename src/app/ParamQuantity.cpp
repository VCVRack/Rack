#include "app/ParamQuantity.hpp"


namespace rack {


Param *ParamQuantity::getParam() {
	assert(module);
	return &module->params[paramId];
}

void ParamQuantity::commitSnap() {
	// TODO
}

void ParamQuantity::setValue(float value) {
	value = math::clamp(value, getMinValue(), getMaxValue());
	// TODO Smooth
	// TODO Snap
	getParam()->value = value;
}

float ParamQuantity::getValue() {
	return getParam()->value;
}

float ParamQuantity::getMinValue() {
	return getParam()->minValue;
}

float ParamQuantity::getMaxValue() {
	return getParam()->maxValue;
}

float ParamQuantity::getDefaultValue() {
	return getParam()->defaultValue;
}

float ParamQuantity::getDisplayValue() {
	if (getParam()->displayBase == 0.f) {
		// Linear
		return getParam()->value * getParam()->displayMultiplier;
	}
	else {
		// Exponential
		return std::pow(getParam()->displayBase, getParam()->value) * getParam()->displayMultiplier;
	}
}

void ParamQuantity::setDisplayValue(float displayValue) {
	if (getParam()->displayBase == 0.f) {
		// Linear
		getParam()->value = displayValue / getParam()->displayMultiplier;
	}
	else {
		// Exponential
		getParam()->value = std::log(displayValue / getParam()->displayMultiplier) / std::log(getParam()->displayBase);
	}
}

int ParamQuantity::getDisplayPrecision() {
	return getParam()->displayPrecision;
}

std::string ParamQuantity::getLabel() {
	return getParam()->label;
}

std::string ParamQuantity::getUnit() {
	return getParam()->unit;
}


} // namespace rack
