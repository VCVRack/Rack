#pragma once
#include <type_traits>
#include <dsp/common.hpp>


namespace rack {
namespace dsp {


/** 24-bit integer, using int32_t for conversions. */
struct
#ifdef __clang__
__attribute__((packed, aligned(1)))
#else
__attribute__((packed, aligned(1), gcc_struct))
#endif
Int24 {
	int32_t i : 24;
	Int24() {}
	Int24(int32_t i) : i(i) {}
	operator int32_t() {return i;}
};
static_assert(sizeof(Int24) == 3, "Int24 type must be 3 bytes");


/** Converts between normalized types.
*/
template <typename To, typename From>
To convert(From x) = delete;


/** Trivial conversions */
template <>
inline float convert(float x) {return x;}


/** Integer to float */
template <>
inline float convert(int8_t x) {return x / 128.f;}
template <>
inline float convert(int16_t x) {return x / 32768.f;}
template <>
inline float convert(Int24 x) {return x / 8388608.f;}
template <>
inline float convert(int32_t x) {return x / 2147483648.f;}
template <>
inline float convert(int64_t x) {return x / 9223372036854775808.f;}


/** Float to integer */
template <>
inline int8_t convert(float x) {return std::min(std::llround(x * 128.f), 127LL);}
template <>
inline int16_t convert(float x) {return std::min(std::llround(x * 32768.f), 32767LL);}
template <>
inline Int24 convert(float x) {return std::min(std::llround(x * 8388608.f), 8388607LL);}
template <>
inline int32_t convert(float x) {return std::min(std::llround(x * 2147483648.f), 2147483647LL);}
template <>
inline int64_t convert(float x) {return std::min(std::llround(x * 9223372036854775808.f), 9223372036854775807LL);}


/** Buffer conversion */
template <typename To, typename From>
void convert(const From* in, To* out, size_t len) {
	for (size_t i = 0; i < len; i++) {
		out[i] = convert<To, From>(in[i]);
	}
}


} // namespace dsp
} // namespace rack
