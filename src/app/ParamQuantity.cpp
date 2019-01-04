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
	if (!module)
		return;
	value = math::clamp(value, getMinValue(), getMaxValue());
	// TODO Smooth
	// TODO Snap
	getParam()->value = value;
}

float ParamQuantity::getValue() {
	if (!module)
		return Quantity::getValue();
	return getParam()->value;
}

float ParamQuantity::getMinValue() {
	if (!module)
		return Quantity::getMinValue();
	return getParam()->minValue;
}

float ParamQuantity::getMaxValue() {
	if (!module)
		return Quantity::getMaxValue();
	return getParam()->maxValue;
}

float ParamQuantity::getDefaultValue() {
	if (!module)
		return Quantity::getDefaultValue();
	return getParam()->defaultValue;
}

float ParamQuantity::getDisplayValue() {
	if (!module)
		return Quantity::getDisplayValue();
	if (getParam()->displayBase == 0.f) {
		// Linear
		return getParam()->value * getParam()->displayMultiplier;
	}
	else if (getParam()->displayBase == 1.f) {
		// Fixed (special case of exponential)
		return getParam()->displayMultiplier;
	}
	else {
		// Exponential
		return std::pow(getParam()->displayBase, getParam()->value) * getParam()->displayMultiplier;
	}
}

void ParamQuantity::setDisplayValue(float displayValue) {
	if (!module)
		return;
	if (getParam()->displayBase == 0.f) {
		// Linear
		getParam()->value = displayValue / getParam()->displayMultiplier;
	}
	else if (getParam()->displayBase == 1.f) {
		// Fixed
		getParam()->value = getParam()->displayMultiplier;
	}
	else {
		// Exponential
		getParam()->value = std::log(displayValue / getParam()->displayMultiplier) / std::log(getParam()->displayBase);
	}
}

int ParamQuantity::getDisplayPrecision() {
	if (!module)
		return Quantity::getDisplayPrecision();
	return getParam()->displayPrecision;
}

std::string ParamQuantity::getLabel() {
	if (!module)
		return Quantity::getLabel();
	return getParam()->label;
}

std::string ParamQuantity::getUnit() {
	if (!module)
		return Quantity::getUnit();
	return getParam()->unit;
}


} // namespace rack
