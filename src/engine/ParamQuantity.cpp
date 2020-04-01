#include <algorithm>

#include <engine/ParamQuantity.hpp>
#include <context.hpp>
#include <engine/Engine.hpp>
#include <string.hpp>


namespace rack {
namespace engine {


engine::Param* ParamQuantity::getParam() {
	assert(module);
	assert(paramId < (int) module->params.size());
	return &module->params[paramId];
}

void ParamQuantity::setSmoothValue(float smoothValue) {
	if (!module)
		return;
	smoothValue = math::clampSafe(smoothValue, getMinValue(), getMaxValue());
	if (snapEnabled)
		smoothValue = std::round(smoothValue);
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
	if (snapEnabled)
		value = std::round(value);
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
	return displayPrecision;
}

std::string ParamQuantity::getDisplayValueString() {
	return Quantity::getDisplayValueString();
}

void ParamQuantity::setDisplayValueString(std::string s) {
	Quantity::setDisplayValueString(s);
}

std::string ParamQuantity::getLabel() {
	if (name == "")
		return string::f("#%d", paramId + 1);
	return name;
}

std::string ParamQuantity::getUnit() {
	return unit;
}

void ParamQuantity::reset() {
	Quantity::reset();
}

void ParamQuantity::randomize() {
	if (!isBounded())
		return;

	if (snapEnabled) {
		// Randomize inclusive of the maximum value
		float value = math::rescale(random::uniform(), 0.f, 1.f, getMinValue(), getMaxValue() + 1.f);
		value = std::floor(value);
		setValue(value);
	}
	else {
		// Same as Quantity::randomize
		setScaledValue(random::uniform());
	}
}

std::string ParamQuantity::getDescription() {
	return description;
}


std::string SwitchQuantity::getDisplayValueString() {
	int index = (int) std::floor(getValue());
	if (!(0 <= index && index < (int) labels.size()))
		return "";
	return labels[index];
}

void SwitchQuantity::setDisplayValueString(std::string s) {
	auto it = std::find_if(labels.begin(), labels.end(), [&](const std::string& a) {
		return string::lowercase(a) == string::lowercase(s);
	});
	if (it == labels.end())
		return;
	int index = std::distance(labels.begin(), it);
	setValue(index);
}


} // namespace engine
} // namespace rack
