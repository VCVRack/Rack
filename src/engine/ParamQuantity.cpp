#include <engine/ParamQuantity.hpp>
#include <app.hpp>
#include <engine/Engine.hpp>


namespace rack {
namespace engine {


engine::Param* ParamQuantity::getParam() {
	assert(module);
	return &module->params[paramId];
}

void ParamQuantity::setSmoothValue(float smoothValue) {
	if (!module)
		return;
	smoothValue = math::clampSafe(smoothValue, getMinValue(), getMaxValue());
	APP->engine->setSmoothParam(module, paramId, smoothValue);
}

float ParamQuantity::getSmoothValue() {
	if (!module)
		return 0.f;
	return APP->engine->getSmoothParam(module, paramId);
}

void ParamQuantity::setValue(float value) {
	if (!module)
		return;
	value = math::clampSafe(value, getMinValue(), getMaxValue());
	APP->engine->setParam(module, paramId, value);
}

float ParamQuantity::getValue() {
	if (!module)
		return 0.f;
	return APP->engine->getParam(module, paramId);
}

float ParamQuantity::getMinValue() {
	return minValue;
}

float ParamQuantity::getMaxValue() {
	return maxValue;
}

float ParamQuantity::getDefaultValue() {
	return defaultValue;
}

float ParamQuantity::getDisplayValue() {
	if (!module)
		return Quantity::getDisplayValue();
	float v = getSmoothValue();
	if (displayBase == 0.f) {
		// Linear
		// v is unchanged
	}
	else if (displayBase < 0.f) {
		// Logarithmic
		v = std::log(v) / std::log(-displayBase);
	}
	else {
		// Exponential
		v = std::pow(displayBase, v);
	}
	return v * displayMultiplier + displayOffset;
}

void ParamQuantity::setDisplayValue(float displayValue) {
	if (!module)
		return;
	float v = displayValue - displayOffset;
	if (displayMultiplier == 0.f)
		v = 0.f;
	else
		v /= displayMultiplier;
	if (displayBase == 0.f) {
		// Linear
		// v is unchanged
	}
	else if (displayBase < 0.f) {
		// Logarithmic
		v = std::pow(-displayBase, v);
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
	return label;
}

std::string ParamQuantity::getUnit() {
	return unit;
}


} // namespace engine
} // namespace rack
