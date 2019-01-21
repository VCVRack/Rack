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
	float v = getSmoothValue();
	float displayBase = getParam()->displayBase;
	if (displayBase == 0.f) {
		// Linear
	}
	else if (displayBase < 0.f) {
		// Logarithmic
		v = std::log(v) / std::log(-displayBase);
	}
	else {
		// Exponential
		v = std::pow(displayBase, v);
	}
	return v * getParam()->displayMultiplier + getParam()->displayOffset;
}

void ParamQuantity::setDisplayValue(float displayValue) {
	if (!module)
		return;
	float v = (displayValue - getParam()->displayOffset) / getParam()->displayMultiplier;
	float displayBase = getParam()->displayBase;
	if (displayBase == 0.f) {
		// Linear
	}
	else if (displayBase < 0.f) {
		// Logarithmic
		v = std::pow(displayBase, v);
	}
	else {
		// Exponential
		v = std::log(v) / std::log(displayBase);
	}
	setValue(v);
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
