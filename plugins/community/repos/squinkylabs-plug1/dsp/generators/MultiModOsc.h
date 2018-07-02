#pragma once

#include <cmath>

#include "SawOscillator.h"

/**
 * A bunch of LFOs at slightly different frequencies added together in different ways.
 * Taken from Bernie Hutchins' ElectroNotes.
 */
template <typename T, int NOsc, int NOut>
class MultiModOsc
{
public:
    MultiModOsc() = delete;
    /**
     * Make state and params be nested classes so we don't have
     * to type as many template arguments.
     */
    class State
    {
    public:
        friend MultiModOsc;
    private:
        SawOscillatorState<T> states[NOsc];
    };
    class Params
    {
    public:
        friend MultiModOsc;
        Params();
        /**
         * @param rate is -1..1 arbitrary "low frequency" units
         */
        void setRateAndSpread(T rate, T spread, int matrixMode, T inverseSampleRate);
    private:
        SawOscillatorParams<T> params[NOsc];
        int matrixMode = 0;
    };

    static void run(T * output, State&, const Params&);
};

template <typename T, int NOsc, int NOut>
inline MultiModOsc<T, NOsc, NOut>::Params::Params()
{
    setRateAndSpread(.5, .5, 0, T(1.0 / 44100));
}

template <typename T, int NOsc, int NOut>
inline void MultiModOsc<T, NOsc, NOut>::Params::setRateAndSpread(T rate, T spread, int inMatrixMode, T inverseSampleRate)
{
    assert(rate >= -10 && rate <= 10);        // just a sanity check
    assert(inverseSampleRate > (1.0 / 200000));
    assert(inverseSampleRate < (1.0 / 2000)); // same

    T BaseRate = 1.0;
    BaseRate *= std::pow(T(3), rate);
    const T dNormSpread = spread * T((3.0 / 2.0) + .5);
    for (int i = 0; i < NOsc; i++) {
        T dMult = 1;
        switch (i) {
            case 1:
                dMult = 1 / T(1.11);
                break;
            case 0:
                dMult = 1.0;
                break;
            case 2:
                dMult = T(1.32);
                break;
            case 3:
                dMult = 1.25;
                break;
            case 4:
                dMult = 1.5;
                break;
            default:
                assert(false);
        }
        dMult -= 1.0;	// norm to 0
        dMult *= dNormSpread;
        dMult += 1.0;

        const T x = BaseRate * dMult;
        const T actual = x * inverseSampleRate;
        SawOscillator<T, false>::setFrequency(params[i], actual);
        this->matrixMode = inMatrixMode;

    }
}

template <typename T, int NOsc, int NOut>
inline void MultiModOsc<T, NOsc, NOut>::run(T* output, State& state, const Params& params)
{
    T modulators[NOsc];
    for (int i = 0; i < NOsc; ++i) {
        modulators[i] = SawOscillator<T, false>::runTri(state.states[i], params.params[i]);
    }
    // The old implementation had a smarter algorithm, but
    // for now hard-wiring it for 4/3 is OK
    if ((NOsc == 4) && (NOut == 3)) {
        switch (params.matrixMode) {
            case 0:     // classic mix
                output[0] = modulators[0] + modulators[1] + modulators[2];  // not 3
                output[1] = modulators[0] + modulators[1] + modulators[3];  // not 2
                output[2] = modulators[0] + modulators[2] + modulators[3];  // not 1
                break;
            case 1:
                    // slight variation on classic
                output[0] = modulators[0] + modulators[1] + modulators[2];  // not 3
                output[1] = modulators[1] + modulators[2] + modulators[3];  // not 0
                output[2] = modulators[0] + modulators[2] + modulators[3];  // not 1
                break;
            case 2:
                    // invert some
                output[0] = modulators[0] + modulators[1] + modulators[2];  // not 3
                output[1] = -modulators[0] + modulators[2] + modulators[3];  // not 0
                output[2] = -modulators[1] - modulators[2] - modulators[3];  // not 1
                break;
            default:
                assert(false);

        }
    } else {
        assert(false);  // need to return something
    }
}
