#pragma once

namespace meta
{

// Update if different =========================================================

template<typename T>
bool updateIfDifferent (T& lhs, const T& rhs)
{
    if (lhs == rhs)
        return false;

    lhs = rhs;

    return true;
}

template<typename T>
bool updateIfDifferent (T& lhs, const T&& rhs)
{
    if (lhs == rhs)
        return false;

    lhs = rhs;

    return true;
}

// Protected math call =========================================================

template<typename T>
T exp(T exponent)
{
	static const T maxExponent = std::nextafter(std::log(std::numeric_limits<T>::max()), T(0));
	static const T maxValue = std::exp(maxExponent);

	return (exponent < maxExponent) ? std::exp(exponent) : maxValue;
}

template<typename T>
T min(T a, T b)
{
    return (a < b) ? a : b;
}

template<typename T>
T max(T a, T b)
{
    return (a > b) ? a : b;
}

template<typename T>
T clamp(T value, T minValue, T maxValue)
{
    return max(min(value, maxValue), minValue);
}

} // namespace