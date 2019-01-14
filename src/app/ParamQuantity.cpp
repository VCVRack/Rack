#include "app/ParamQuantity.hpp"
#include "app.hpp"
#include "engine/Engine.hpp"


namespace rack {


Param *ParamQuantity::getParam() {
	assert(module);
	return &module->params[paramId];
}

void ParamQuantity::setSmoothValue(float smoothValue) {
	if (!module)
		return;
	smoothValue = math::clamp(smoothValue, getMinValue(), getMaxValue());
	app()->engine->setSmoothParam(module, paramId, smoothValue);
}

float ParamQuantity::getSmoothValue() {
	return app()->engine->getSmoothParam(module, paramId);
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
	float displayBase = getParam()->displayBase;
	if (displayBase == 0.f) {
		// Linear
		return getSmoothValue() * getParam()->displayMultiplier;
	}
	else if (displayBase == 1.f) {
		// Fixed (special case of exponential)
		return getParam()->displayMultiplier;
	}
	else if (displayBase < 0.f) {
		// Logarithmic
		return std::log(getSmoothValue()) / std::log(-displayBase) * getParam()->displayMultiplier;
	}
	else {
		// Exponential
		return std::pow(displayBase, getSmoothValue()) * getParam()->displayMultiplier;
	}
}

void ParamQuantity::setDisplayValue(float displayValue) {
	if (!module)
		return;
	float displayBase = getParam()->displayBase;
	if (displayBase == 0.f) {
		// Linear
		setValue(displayValue / getParam()->displayMultiplier);
	}
	else if (displayBase == 1.f) {
		// Fixed
		setValue(getParam()->displayMultiplier);
	}
	else if (displayBase < 0.f) {
		// Logarithmic
		setValue(std::pow(displayBase, displayValue / getParam()->displayMultiplier));
	}
	else {
		// Exponential
		setValue(std::log(displayValue / getParam()->displayMultiplier) / std::log(displayBase));
	}
}

int ParamQuantity::getDisplayPrecision() {
	return Quantity::getDisplayPrecision();
}

std::string ParamQuantity::getDisplayValueString() {
	return Quantity::getDisplayValueString();
}

void ParamQuantity::setDisplayValueString(std::string s) {
	Quantity::setDisplayValueString(s);
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
