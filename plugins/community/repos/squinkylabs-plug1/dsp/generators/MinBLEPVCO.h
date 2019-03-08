#pragma once


// Need to make this compile in MS tools for unit tests
#if defined(_MSC_VER)
#define __attribute__(x)

#pragma warning (push)
#pragma warning ( disable: 4244 4267 )
#endif

#ifndef _CLAMP
#define _CLAMP
namespace std {
    inline float clamp(float v, float lo, float hi)
    {
        assert(lo < hi);
#define sMIN(a,b) (((a)>(b))?(b):(a))
#define sMAX(a,b) (((a)>(b))?(a):(b))
        //return std::min(hi, std::max(v, lo));
        return sMIN(hi, sMAX(v, lo));
#undef sMIN
#undef sMAX
    }
}
#endif

//#include "math.hpp"

#include "SqBlep.h"
#include "SqMath.h"

#include "dsp/filter.hpp"
#include "AudioMath.h"
#include "ObjectCache.h"

#include <functional>

// until c++17
#ifndef _CLAMP
#define _CLAMP
namespace std {
    inline float clamp(float v, float lo, float hi)
    {
        assert(lo < hi);
        return std::min(hi, std::max(v, lo));
    }
}
#endif

/* VCO core using MinBLEP to reduce aliasing.
 * Originally based on Befaco EvenVCO
 */

class MinBLEPVCO
{
public:
    friend class TestMB;

    /**
     * ph is the "phase (-1..0)"
     */
    using SyncCallback = std::function<void(float p)>;

    MinBLEPVCO();
    enum class Waveform
    {
        Sin, Tri, Saw, Square, Even, END
    };

    void step();

    void setNormalizedFreq(float f, float st)
    {
        normalizedFreq = std::clamp(f, 1e-6f, 0.5f);
        sampleTime = st;
    }

    void setWaveform(Waveform);

    float getOutput() const
    {
        return output;
    }

    /**
     * Send the sync waveform to VCO.
     * usually called from outside.
     */
    void onMasterSync(float phase);
    void setSyncCallback(SyncCallback);
    void setPulseWidth(float);
    void setSyncEnabled(bool f)
    {
        syncEnabled = f;
    }

private:

    float output = 0;
    Waveform waveform = Waveform::Saw;

    float phase = 0.0;
    float normalizedFreq = 0;
    float sampleTime = 0;
    SyncCallback syncCallback = nullptr;
    float tri = 0;
    bool syncEnabled = false;

    bool gotSyncCallback = false;
    float syncCallbackCrossing = 0;

    /**
    * References to shared lookup tables.
    * Destructor will free them automatically.
    */

    std::shared_ptr<LookupTableParams<float>> sinLookup = {ObjectCache<float>::getSinLookup()};

    /** Whether we are past the pulse width already */
    bool halfPhase = false;

    int loopCounter = 0;        // still used?
    float pulseWidth = .5;

    SqBlep syncMinBLEP;
    SqBlep aMinBLEP;
    SqBlep bMinBLEP;

    bool aIsNext = false;
    SqBlep* getNextMinBLEP();

    /**
     * Waveform generation helper
     */
    void step_even();
    void step_saw();
    void step_sq();
    void step_sin();
    void step_tri();

    /**
     * input = phase, 0..1
     * output = sin(2pi * input)
     */
    float sineLook(float input) const;

    float evenLook(float input) const;

    std::string name;
    bool lastSq = false;
    bool isSqHigh() const;
};

// Let's by lazy and use "using" to solve some v1/v6 issues/
#ifdef __V1
using namespace rack::dsp;
#else
using namespace rack;
#endif

inline MinBLEPVCO::MinBLEPVCO()
{
}

inline  SqBlep* MinBLEPVCO::getNextMinBLEP()
{
    aIsNext = !aIsNext;
    return aIsNext ? &aMinBLEP : &bMinBLEP;
}

inline void MinBLEPVCO::setSyncCallback(SyncCallback cb)
{
    assert(!syncCallback);
    syncCallback = cb;
}

inline void MinBLEPVCO::setWaveform(Waveform wf)
{
    waveform = wf;
}

inline void MinBLEPVCO::setPulseWidth(float pw)
{
    pulseWidth = pw;
}

inline void MinBLEPVCO::step()
{
    // call the dedicated dispatch routines for the special case waveforms.
    switch (waveform) {
        case  Waveform::Saw:
            step_saw();
            break;
        case  Waveform::Square:
            step_sq();
            break;
        case  Waveform::Sin:
            step_sin();
            break;
        case  Waveform::Tri:
            // Tri sync doesn't work -> use sin
            if (syncEnabled) {
                step_sin();
            } else {
                step_tri();
            }
            break;
        case  Waveform::Even:
            step_even();
            break;
        case Waveform::END:
            output = 0;
            break;                  // don't do anything if no outputs
        default:
            assert(false);
    }
}

// callback from master sync when it rolls over
inline void MinBLEPVCO::onMasterSync(float masterPhase)
{
    gotSyncCallback = true;
    syncCallbackCrossing = masterPhase;
}

inline void MinBLEPVCO::step_saw()
{
    phase += normalizedFreq;
    const float predictedPhase = phase;
    if (gotSyncCallback) {
        const float excess = -syncCallbackCrossing * normalizedFreq;
        // Figure out where our sub-sample phase should be after reset
        // reset to zero
        const float newPhase = excess;
        phase = newPhase;
    }
    if (phase >= 1.0) {
        phase -= 1.0;
    }

    // see if we jumped
    if (phase != predictedPhase) {
        const float jump = phase - predictedPhase;
       // printf("%s jump = %f\n", name.c_str(), jump); fflush(stdout);
        if (gotSyncCallback) {
            const float crossing = syncCallbackCrossing;
            syncMinBLEP.jump(crossing, jump);
            if (syncCallback) {
                syncCallback(crossing);
            }
        } else {
            // phase overflowed
            const float crossing = -phase / normalizedFreq;
            aMinBLEP.jump(crossing, jump);
            if (syncCallback) {
                syncCallback(crossing);
            }
        }
    }

    float totalPhase = phase;
    totalPhase += aMinBLEP.shift();
    totalPhase += syncMinBLEP.shift();
    float saw = -1.0 + 2.0 * totalPhase;
    output = 5.0*saw;

    gotSyncCallback = false;
}

inline bool MinBLEPVCO::isSqHigh() const
{
    return phase >= pulseWidth;
}

inline void MinBLEPVCO::step_sq()
{
    bool phaseDidOverflow = false;
    phase += normalizedFreq;
    if (gotSyncCallback) {
        const float excess = -syncCallbackCrossing * normalizedFreq;
        // reset phase to near zero on sync
        phase = excess;
    }
    if (phase > 1.0f) {
        phase -= 1.0f;
        phaseDidOverflow = true;
    }

    // now examine for any pending edges,
    // and if found apply minBLEP and
    // send sync signal
    bool newSq = isSqHigh();
    if (newSq != lastSq) {
        lastSq = newSq;
        const float jump = newSq ? 2 : -2;
        if (gotSyncCallback) {
            const float crossing = syncCallbackCrossing;
            syncMinBLEP.jump(crossing, jump);
            if (syncCallback) {
                syncCallback(crossing);
            }
        } else if (phaseDidOverflow) {
            const float crossing = -phase / normalizedFreq;
            aMinBLEP.jump(crossing, jump);
            if (syncCallback) {
                syncCallback(crossing);
            }
        } else {
            // crossed PW boundary
            const float crossing = -(phase - pulseWidth) / normalizedFreq;
            bMinBLEP.jump(crossing, jump);
        }
    }

    float square = newSq ? 1.0f : -1.0f;
    square += aMinBLEP.shift();
    square += bMinBLEP.shift();
    square += syncMinBLEP.shift();

    output = 5.0*square;

    gotSyncCallback = false;
}

inline float MinBLEPVCO::sineLook(float input) const
{
    // want cosine, but only have sine lookup
    float adjPhase = input + .25f;
    if (adjPhase >= 1) {
        adjPhase -= 1;
    }

    return -LookupTable<float>::lookup(*sinLookup, adjPhase, true);
}

inline void MinBLEPVCO::step_sin()
{
    if (gotSyncCallback) {
        gotSyncCallback = false;

        // All calculations based on slave sync discontinuity happening at 
        // the same sub-sample as the mater discontinuity.

        // First, figure out how much excess phase we are going to have after reset
        const float excess = -syncCallbackCrossing * normalizedFreq;

        // Figure out where our sub-sample phase should be after reset
        const float newPhase = .5 + excess;

        const float oldOutput = sineLook(phase);
        const float newOutput = sineLook(newPhase);
        const float jump = newOutput - oldOutput;

        syncMinBLEP.jump(syncCallbackCrossing, jump);
        this->phase = newPhase;
       // return;
    } else {

        phase += normalizedFreq;

        // Reset phase if at end of cycle
        if (phase >= 1.0) {
            phase -= 1.0;
            if (syncCallback) {
                float crossing = -phase / normalizedFreq;
                syncCallback(crossing);
            }
        }
    }

    float sine = sineLook(phase);
    sine += syncMinBLEP.shift();
    output = 5.0*sine;
}

inline void MinBLEPVCO::step_tri()
{
    if (gotSyncCallback) {
        gotSyncCallback = false;

        // All calculations based on slave sync discontinuity happening at 
        // the same sub-sample as the mater discontinuity.

        // First, figure out how much excess phase we are going to have after reset
        const float excess = -syncCallbackCrossing * normalizedFreq;

        // Figure out where our sub-sample phase should be after reset
        const float newPhase = .5 + excess;
        const float jump = -2.f * (phase - newPhase);
#ifdef _LOG 
        printf("%s: got sync ph=%.2f nph=%.2f excess=%.2f send cross %.2f jump %.2f \n", name.c_str(),
            phase, newPhase,
            excess,
            syncCallbackCrossing, jump);
#endif
        syncMinBLEP.jump(syncCallbackCrossing, jump);
        this->phase = newPhase;
        return;
    }
    float oldPhase = phase;
    phase += normalizedFreq;

    if (oldPhase < 0.5 && phase >= 0.5) {
        const float crossing = -(phase - 0.5) / normalizedFreq;
        aMinBLEP.jump(crossing, 2.0);
    }

    // Reset phase if at end of cycle
    if (phase >= 1.0) {
        phase -= 1.0;
        float crossing = -phase / normalizedFreq;
        aMinBLEP.jump(crossing, -2.0);
        halfPhase = false;
        if (syncCallback) {
            syncCallback(crossing);
        }
    }

    // Outputs
    float triSquare = (phase < 0.5) ? -1.0 : 1.0;
    triSquare += aMinBLEP.shift();
    triSquare += syncMinBLEP.shift();

    // Integrate square for triangle
    tri += 4.0 * triSquare * normalizedFreq;
    tri *= (1.0 - 40.0 * sampleTime);

    // Set output
    output = 5.0*tri;
}


inline float calcDoubleSaw(float phase)
{
    return (phase < 0.5) ? (-1.0 + 4.0*phase) : (-1.0 + 4.0*(phase - 0.5));
}


inline float MinBLEPVCO::evenLook(float input) const
{
    float doubleSaw = calcDoubleSaw(input);
    const float sine = sineLook(input);
    const float even = 0.55 * (doubleSaw + 1.27 * sine);
    return even;
}

inline void MinBLEPVCO::step_even()
{
    float oldPhase = phase;
    phase += normalizedFreq;
    float syncJump = 0;
    if (gotSyncCallback) {
       

        // All calculations based on slave sync discontinuity happening at 
        // the same sub-sample as the mater discontinuity.

        // First, figure out how much excess phase we are going to have after reset
        const float excess = -syncCallbackCrossing * normalizedFreq;

        // Figure out where our sub-sample phase should be after reset
        const float newPhase = .5 + excess;
      //  const float jump = -2.f * (phase - newPhase);
#ifdef _LOG 
        printf("%s: got sync ph=%.2f nph=%.2f excess=%.2f send cross %.2f jump %.2f \n", name.c_str(),
            phase, newPhase,
            excess,
            syncCallbackCrossing, jump);
#endif
      //  syncMinBLEP.jump(syncCallbackCrossing, jump);
        syncJump = evenLook(newPhase) - evenLook(this->phase);
        this->phase = newPhase;
    }

    bool jump5 = false;
    bool jump1 = false;
    if (oldPhase < 0.5 && phase >= 0.5) {
        jump5 = true;
    }

    // Reset phase if at end of cycle
    if (phase >= 1.0) {
        phase -= 1.0;
        jump1 = true;
    }

    if (gotSyncCallback) {
        const float crossing = syncCallbackCrossing;

        // FIXME!!
        float jump = syncJump;
        syncMinBLEP.jump(crossing, jump);
        if (syncCallback) {
            syncCallback(crossing);
        }

    } else if (jump1) {
        const float jump = -2;
        float crossing = -phase / normalizedFreq;
        aMinBLEP.jump(crossing, jump);
        if (syncCallback) {
            syncCallback(crossing);
        }

    } else if (jump5) {
        const float jump = -2;
        const float crossing = -(phase - 0.5) / normalizedFreq;
        aMinBLEP.jump(crossing, jump);
    }

    
    // note that non-sync minBLEP is added to double saw,
    // but for sync it's added to even. 
    const float sine = sineLook(phase);
    float doubleSaw = (phase < 0.5) ? (-1.0 + 4.0*phase) : (-1.0 + 4.0*(phase - 0.5));
    doubleSaw += aMinBLEP.shift();
    float even = 0.55 * (doubleSaw + 1.27 * sine);
    even += syncMinBLEP.shift();

    output = 5.0*even;
    gotSyncCallback = false;
}
#if 0 // old way
inline void MinBLEPVCO::step_even()
{
    if (gotSyncCallback) {
        gotSyncCallback = false;

        // All calculations based on slave sync discontinuity happening at 
        // the same sub-sample as the mater discontinuity.

        // First, figure out how much excess phase we are going to have after reset
        const float excess = -syncCallbackCrossing * normalizedFreq;

        // Figure out where our sub-sample phase should be after reset
        const float newPhase = .5 + excess;
        const float jump = -2.f * (phase - newPhase);
#ifdef _LOG 
        printf("%s: got sync ph=%.2f nph=%.2f excess=%.2f send cross %.2f jump %.2f \n", name.c_str(),
            phase, newPhase,
            excess,
            syncCallbackCrossing, jump);
#endif
        syncMinBLEP.jump(syncCallbackCrossing, jump);
        this->phase = newPhase;
        return;
    }
    float oldPhase = phase;
    phase += normalizedFreq;

    if (oldPhase < 0.5 && phase >= 0.5) {
        float crossing = -(phase - 0.5) / normalizedFreq;
        aMinBLEP.jump(crossing, -2.0);
    }

    // Reset phase if at end of cycle
    if (phase >= 1.0) {
        phase -= 1.0;
        float crossing = -phase / normalizedFreq;
        aMinBLEP.jump(crossing, -2.0);
        if (syncCallback) {
            syncCallback(crossing);
        }
    }

    //sine = -cosf(2*AudioMath::Pi * phase);
    // want cosine, but only have sine lookup
    float adjPhase = phase + .25f;
    if (adjPhase >= 1) {
        adjPhase -= 1;
    }
    const float sine = -LookupTable<float>::lookup(*sinLookup, adjPhase, true);

    float doubleSaw = (phase < 0.5) ? (-1.0 + 4.0*phase) : (-1.0 + 4.0*(phase - 0.5));
    doubleSaw += aMinBLEP.shift();
    doubleSaw += syncMinBLEP.shift();
    const float even = 0.55 * (doubleSaw + 1.27 * sine);

    //TBase::outputs[SINE_OUTPUT].value = 5.0*sine;
    output = 5.0*even;
}
#endif

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

