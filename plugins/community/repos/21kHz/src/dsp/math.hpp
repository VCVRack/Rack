#include <array>


using std::array;


// four point, fourth-order b-spline polyblep, from:
// Välimäki, Pekonen, Nam. "Perceptually informed synthesis of bandlimited
// classical waveforms using integrated polynomial interpolation"
inline void polyblep4(array<float, 4> &buffer, float d, float u) {
    if (d > 1.0f) {
        d = 1.0f;
    }
    else if (d < 0.0f) {
        d = 0.0f;
    }
    
    float d2 = d * d;
    float d3 = d2 * d;
    float d4 = d3 * d;
    float dd3 = 0.16667 * (d + d3);
    float cd2 = 0.041667 + 0.25 * d2;
    float d4_1 = 0.041667 * d4;
    
    buffer[3] += u * (d4_1);
    buffer[2] += u * (cd2 + dd3 - 0.125 * d4);
    buffer[1] += u * (-0.5 + 0.66667 * d - 0.33333 * d3 + 0.125 * d4);
    buffer[0] += u * (-cd2 + dd3 - d4_1);
}


// four point, fourth-order b-spline polyblamp, from:
// Esqueda, Välimäki, Bilbao. "Rounding Corners with BLAMP".
inline void polyblamp4(array<float, 4> &buffer, float d, float u) {
    if (d > 1.0f) {
        d = 1.0f;
    }
    else if (d < 0.0f) {
        d = 0.0f;
    }
    
    float d2 = d * d;
    float d3 = d2 * d;
    float d4 = d3 * d;
    float d5 = d4 * d;
    float d5_1 = 0.0083333 * d5;
    float d5_2 = 0.025 * d5;
    
    buffer[3] += u * (d5_1);
    buffer[2] += u * (0.0083333 + 0.083333 * (d2 + d3) + 0.041667 * (d + d4) - d5_2);
    buffer[1] += u * (0.23333 - 0.5 * d + 0.33333 * d2 - 0.083333 * d4 + d5_2);
    buffer[0] += u * (0.0083333 + 0.041667 * (d4 - d) + 0.083333 * (d2 - d3) - d5_1);
}


// fast sine calculation. modified from the Reaktor 6 core library.
// takes a [0, 1] range and folds it to a triangle on a [0, 0.5] range.
inline float sin_01(float t) {
    if (t > 1.0f) {
        t = 1.0f;
    }
    else if (t > 0.5) {
        t = 1.0f - t;
    }
    else if (t < 0.0f) {
        t = 0.0f;
    }
    t = 2.0f * t - 0.5f;
    float t2 = t * t;
    t = (((-0.540347 * t2 + 2.53566) * t2 - 5.16651) * t2 + 3.14159) * t;
    return t;
}
