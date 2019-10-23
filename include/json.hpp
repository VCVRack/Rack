#pragma once
#include <common.hpp>
#include <jansson.h>


namespace rack {


/** JSON helpers */
namespace json {


/** Checks that a JSON type can be cast to the C++ type */
template <typename T>
bool is(json_t* valueJ);
template <>
inline bool is<std::string>(json_t* valueJ) {
	return json_is_string(valueJ);
}
template <>
inline bool is<json_int_t>(json_t* valueJ) {
	return json_is_integer(valueJ);
}
template <>
inline bool is<int>(json_t* valueJ) {
	return json_is_integer(valueJ);
}
template <>
inline bool is<double>(json_t* valueJ) {
	return json_is_number(valueJ);
}
template <>
inline bool is<float>(json_t* valueJ) {
	return json_is_number(valueJ);
}
template <>
inline bool is<bool>(json_t* valueJ) {
	return json_is_boolean(valueJ);
}

/** Converts C++ types to the JSON type */
template <typename T>
json_t* to(T value);
template <>
inline json_t* to(const std::string& value) {
	return json_stringn(value.c_str(), value.size());
}
template <>
inline json_t* to(json_int_t value) {
	return json_integer(value);
}
template <>
inline json_t* to(int value) {
	return json_integer(value);
}
template <>
inline json_t* to(double value) {
	return json_real(value);
}
template <>
inline json_t* to(float value) {
	return json_real(value);
}
template <>
inline json_t* to(bool value) {
	return json_boolean(value);
}

/** Converts the JSON type to C++ types */
template <typename T>
T from(json_t* valueJ);
template <>
inline std::string from(json_t* valueJ) {
	return std::string(json_string_value(valueJ), json_string_length(valueJ));
}
template <>
inline json_int_t from(json_t* valueJ) {
	return json_integer_value(valueJ);
}
template <>
inline int from(json_t* valueJ) {
	return json_integer_value(valueJ);
}
template <>
inline double from(json_t* valueJ) {
	return json_number_value(valueJ);
}
template <>
inline float from(json_t* valueJ) {
	return json_number_value(valueJ);
}
template <>
inline bool from(json_t* valueJ) {
	return json_boolean_value(valueJ);
}

/** Helper template function for array functions */
inline int multiplyDims() {
	return 1;
}
template <typename... Dims>
int multiplyDims(int dim, Dims... dims) {
	return dim * multiplyDims(dims...);
}

/** Converts a C++ array to a JSON multidimensional array */
template <typename T>
json_t* toArray(const T* x) {
	return to(*x);
}
template <typename T, typename... Dims>
json_t* toArray(const T* x, int dim, Dims... dims) {
	int stride = multiplyDims(dims...);
	json_t* arrayJ = json_array();
	for (int i = 0; i < dim; i++) {
		json_array_insert_new(arrayJ, i, toArray(&x[i * stride], dims...));
	}
	return arrayJ;
}

/** Converts a JSON multidimensional array to a C++ array */
template <typename T>
void fromArray(json_t* arrayJ, T* x) {
	*x = from<T>(arrayJ);
}
template <typename T, typename... Dims>
void fromArray(json_t* arrayJ, T* x, int dim, Dims... dims) {
	int stride = multiplyDims(dims...);
	for (int i = 0; i < dim; i++) {
		json_t* elJ = json_array_get(arrayJ, i);
		if (elJ) {
			fromArray(elJ, &x[i * stride], dims...);
		}
	}
}


} // namespace json
} // namespace rack
