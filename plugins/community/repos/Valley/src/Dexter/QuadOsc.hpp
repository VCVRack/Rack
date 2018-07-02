//
//  QuadOsc.hpp
//  QuadOsc - A syncronous, SIMD optimised oscillator.
//
//  Created by Dale Johnson on 02/02/2018.
//  Copyright © 2018 Dale Johnson. All rights reserved.
//

#ifndef QuadOsc_hpp
#define QuadOsc_hpp

#include <pmmintrin.h>
#include <cmath>
#include <cstdint>
#include <vector>

#ifdef _WIN32
    inline void* aligned_alloc_16(size_t __size) {
        return _aligned_malloc(__size, 16);
    }

    inline void aligned_free_16(void* ptr) {
        _aligned_free(ptr);
    }
#elif __APPLE__
    inline void* aligned_alloc_16(size_t __size) {
        return malloc(__size);
    }

    inline void aligned_free_16(void* ptr) {
        free(ptr);
    }
#elif __linux__
    inline void* aligned_alloc_16(size_t __size) {
        return aligned_alloc(16, __size);
    }

    inline void aligned_free_16(void* ptr) {
        free(ptr);
    }
#endif

inline __m128 _mm_linterp_ps(const __m128& a, const __m128& b, const __m128& frac) {
    return _mm_add_ps(a, _mm_mul_ps(frac, _mm_sub_ps(b, a)));
}

inline __m128 _mm_abs_ps(const __m128& a) {
    __m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
    return _mm_and_ps(a,mask);
}

inline __m128 _mm_switch_ps(const __m128& a, const __m128& b, const __m128& cond) {
    return _mm_or_ps(_mm_andnot_ps(cond, a), _mm_and_ps(cond, b));
}

inline __m128i _mm_switch_int32(const __m128i& a, const __m128i& b, const __m128i& cond) {
    return _mm_or_si128(_mm_andnot_si128(cond, a), _mm_and_si128(cond, b));
}

inline __m128 _mm_clamp_ps(const __m128& a, const __m128& min, const __m128& max) {
    return _mm_min_ps(_mm_max_ps(a, min), max);
}

inline __m128i _mm_clamp_int32(const __m128i& a, const __m128i& min, const __m128i& max) {
    return _mm_switch_int32(_mm_switch_int32(a, min, _mm_cmplt_epi32(a, min)), max, _mm_cmpgt_epi32(a, max));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class Shaper {
public:
    Shaper();
    __m128 process(const __m128& a, const __m128& f);
    void setShapeMode(int mode);
private:
    int _shapeMode;

    // Common vars
    __m128 __aScale, __x, __y, __z, __f, __xx, __ff, __k, __mask, __midMask, __highMask;
    __m128 __m, __c, __denom;
    __m128 __output;
    __m128i __aInt, __xInt, __yInt;
    __m128 __aIntF, __xIntF, __yIntF;

    // Numbers
    __m128 __third, __twoThird, __half, __minusHalf;
    __m128 __minus, __zeros, __ones, __twos, __threes, __fours, __eights, __nines, __sixteens;

    // Shaping functions
    void bend(const __m128& a, const __m128& f);
    void tilt(const __m128& a, const __m128& f);
    void lean(const __m128& a, const __m128& f);
    void twist(const __m128& a, const __m128& f);
    void wrap(const __m128& a, const __m128& f);
    void mirror(const __m128& a, const __m128& f);
    void reflect(const __m128& a, const __m128& f);
    void pulse(const __m128& a, const __m128& f);
    void step4(const __m128& a, const __m128& f);
    void step8(const __m128& a, const __m128& f);
    void step16(const __m128& a, const __m128& f);
    void varStep(const __m128& a, const __m128& f);
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class QuadOsc {
public:
    QuadOsc();
    ~QuadOsc();
    virtual void tick();
    void resetPhase();
    void sync(const __m128& syncSource);

    float getOutput(int channel) const;
    const __m128& getOutput() const;
    const __m128& getPhasor() const;
    const __m128& getEOCPulse() const;

    void setWavetable(float* wavetable, long size);
    void setFrequency(float frequency);
    void setFrequency(float f0, float f1, float f2, float f3);
    void setFrequency(const __m128& frequency);
    void setPhase(const __m128& phase);

    void setShape(float shape);
    void setShapeMethod(int shapeMethod);
    void setPMPostShape(bool PMPostShape);

    void setSyncMode(int syncMode);
    void enableSync(bool enableSync);
    void enableWeakSync(bool weakSync);

    void setSampleRate(float sampleRate);
protected:
    float* _wavetable;
    float _frequency;

    int32_t* _aPos;
    int32_t* _bPos;
    float* _lowSample;
    float* _highSample;
    float* _output;
    float _shape;
    bool _PMPostShape;
    int32_t _syncMode;
    bool _sync, _weakSync;

    __m128 __frequency, __samplerate, __nyquist;

    __m128 __readPhase, __inputPhase, __negMask, __syncSource, __syncState, __syncing;
    __m128 __shifts;
    __m128i __shiftsI;

    __m128 __shape;
    __m128 __a, __aPrev, __b, __stepSize, __tabSize, __tabSize_1;
    __m128 __ones, __zeros, __twos, __fours, __minus, __1_5, __half, __quarter;
    __m128 __dir;
    __m128 __mtMask, __ltMask;
    __m128 __mask, __sub, __eoc, __syncOut;
    __m128i __aInt;
    __m128i __bInt;

    __m128 __lowSamp, __highSamp, __frac, __output;
    Shaper _shaper;
    void calcStepSize();
    __m128 (*shapeMethod)(const __m128&, const __m128&);
    void onChangeSyncMode();
    void hardSync(const __m128& syncSource);
    void softSync(const __m128& syncSource);
    void reverseSync(const __m128& syncSource);
    void octaveSync(const __m128& syncSource);
    void fifthSync(const __m128& syncSource);
    void subOctaveSync(const __m128& syncSource);
    void riseASync(const __m128& syncSource);
    void riseBSync(const __m128& syncSource);
    void fallASync(const __m128& syncSource);
    void fallBSync(const __m128& syncSource);
    void pullASync(const __m128& syncSource);
    void pullBSync(const __m128& syncSource);
    void pushASync(const __m128& syncSource);
    void pushBSync(const __m128& syncSource);
    void holdSync(const __m128& syncSource);
    void oneShot(const __m128& syncSource);
    void lockShot(const __m128& syncSource);
};

class ScanningQuadOsc : public QuadOsc {
public:
    ScanningQuadOsc();
    ~ScanningQuadOsc();
    ScanningQuadOsc(const ScanningQuadOsc&) = delete;
    ScanningQuadOsc& operator=(const ScanningQuadOsc&) = delete;

    void tick() override;
    void setWavebank(float** wavebank, int32_t numWaves, int32_t tableSize);
    void setScanPosition(float position);
    void mm_setScanPosition(const __m128& position);
    int32_t getNumwaves() const;
private:
    float**  _wavebank;
    int32_t _numWaves;
    __m128i __numWaves, __numWaves_1;

    int32_t* _lowBank;
    int32_t* _highBank;
    float* _lowSample2;
    float* _highSample2;
    __m128 __fade, __result1, __result2;
    __m128i __lowBank, __highBank;
};

#endif /* QuadOsc_hpp */
