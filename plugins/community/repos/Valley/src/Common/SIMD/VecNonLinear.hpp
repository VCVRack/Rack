#pragma once
#include "SIMDUtilities.hpp"

inline __m128 vecDriveSignal(const __m128& x, const __m128& drive) {
    __m128 xd = _mm_mul_ps(x, drive);
    __m128 out = xd;
    __m128 a = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(2.6f), xd), _mm_set1_ps(1.69f));
    __m128 b = _mm_sub_ps(_mm_mul_ps(_mm_add_ps(_mm_mul_ps(xd, xd), a), _mm_set1_ps(0.833333f)), _mm_set1_ps(1.f));
    a = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(-2.6f), xd), _mm_set1_ps(1.69f));
    __m128 c = _mm_sub_ps(_mm_set1_ps(1.f), _mm_mul_ps(_mm_add_ps(_mm_mul_ps(xd, xd), a), _mm_set1_ps(0.833333f)));

    out = _mm_switch_ps(out, b, _mm_cmplt_ps(xd, _mm_set1_ps(-0.75f)));
    out = _mm_switch_ps(out, _mm_set1_ps(-1.f), _mm_cmplt_ps(xd, _mm_set1_ps(-1.3f)));

    out = _mm_switch_ps(out, c, _mm_cmpgt_ps(xd, _mm_set1_ps(0.75f)));
    return _mm_switch_ps(out, _mm_set1_ps(1.f), _mm_cmpgt_ps(xd, _mm_set1_ps(1.3f)));
}

inline __m128 VecPolyTanh(const __m128& x) {
    __m128 y;

    // f(x) = p1*x^4 + p2*x^3 + p3*x^2 + p4*x + p5
    /* p1 =   -0.009007  (-0.009359, -0.008655)
       p2 =      0.1179  (0.115, 0.1207)
       p3 =     -0.5748  (-0.5824, -0.5673)
       p4 =       1.246  (1.239, 1.253)
       p5 =    -0.02431  (-0.02645, -0.02217)
       */



    return y;
}
