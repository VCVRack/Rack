#pragma once

#include "DSPMath.hpp"
#include "DSPEffect.hpp"

namespace dsp {

    /**
     * @brief Basic WaveShaper class with build-in dynamic oversampling
     * @tparam OVERSAMPLE
     */
    struct WaveShaper : DSPEffect {
        /* oversampling channel */
        static const int STD_CHANNEL = 0;
        static constexpr double MAX_BIAS_LEVEL = 5.0; // +/- 5V

    protected:
        Resampler<1> *rs;

        double in, gain, bias, k;
        double out;
        Vec amp;

    public:

        WaveShaper(float sr);

        double getIn() const;
        void setIn(double in);
        double getGain() const;
        void setGain(double gain);
        double getBias() const;
        void setBias(double bias);
        double getK() const;
        void setK(double k);
        double getOut() const;
        void setOut(double out);


        /**
         * @brief Returns the actual sample-rate which is used by oversampled computation
         * @return
         */
        double getOversampledRate() {
            return sr * rs->getFactor();
        }


        void setAmplitude(double kpos, double kneg) {
            amp = Vec(kpos, kneg);
        }


        const Vec &getAmplitude() const;


        /**
         * @brief Implements the oversamping of compute method
         */
        void process() override;


        void init() override {
            gain = 0;
            out = 0;
            k = 0;
            bias = 0;
            amp = Vec(0, 0);
        }


        /**
         * @brief To be implemented by subclass, automaticaly oversampled
         *
         * @param x Input sample
         * @return Output sample
         */
        virtual double compute(double x) { return x; }
    };


}

