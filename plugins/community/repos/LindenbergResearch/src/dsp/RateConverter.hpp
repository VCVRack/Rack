/*                                                                     *\
**       __   ___  ______                                              **
**      / /  / _ \/_  __/                                              **
**     / /__/ , _/ / /    Lindenberg                                   **
**    /____/_/|_| /_/  Research Tec.                                   **
**                                                                     **
**                                                                     **
**	  https://github.com/lindenbergresearch/LRTRack	                   **
**    heapdump@icloud.com                                              **
**		                                                               **
**    Sound Modules for VCV Rack                                       **
**    Copyright 2017/2018 by Patrick Lindenberg / LRT                  **
**                                                                     **
**    For Redistribution and use in source and binary forms,           **
**    with or without modification please see LICENSE.                 **
**                                                                     **
\*                                                                     */

#pragma once

namespace dsp {

class RateConverter {
public:
    RateConverter(void);
    virtual ~RateConverter(void);

    // buffer for the impulse response h[n] of the FIR
    float *irBuffer;

    // buffers for the transversal delay lines, one for each input
    float *inputBuffer;
    //  float *m_pRightInputBuffer;

    // read index for delay lines (input x buffers)
    int delayPos;

    // read index for impulse response buffers
    int irBufferPos;

    // write index for input x buffer
    int inputPos;

    int ratio; // OS value, 4 = 4X Oversampling

    // counters and index values for the convolutions
    int osPos;
    int length;

    // initializer - creates the buffers and loads the FIR IR
    void init(int _ratio, int _length, float *pIRBuffer);

    // flush buffers
    void reset();


    // overrides for derived objects
    //
    // interpolateSamples: take one pair of L/R samples and produce L-length buffers of samples
    virtual void interpolateSamples(float xnL, float *pLeftInterpBuffer) {};


    // inner loop function that processes just one pair of inputs
    virtual void interpolateNextOutputSample(float xnL, float &fLeftOutput) {};


    // decimateSamples: take one pai rL-length buffers of samples and decimate down to just one pair of output samples
    virtual void decimateSamples(float *pLeftDeciBuffer, float &ynL) {};


    // inner loop function that processes just one pair of inputs
    virtual bool decimateNextOutputSample(float xnL, float &fLeftOutput) { return true; };
};


class Decimator : public RateConverter {
public:
    Decimator(void);
    ~Decimator(void);

    bool decimateNextOutputSample(float x, float &out) override;
    void decimateSamples(float *buffer, float &out) override;
};


class Interpolator : public RateConverter {
public:
    Interpolator(void);
    ~Interpolator(void);

    void interpolateNextOutputSample(float x, float &fLeftOutput) override;
    void interpolateSamples(float x, float *buffer) override;

};


/**
 * @brief New oversampling class that uses polyphase
 */
class NeoOversampler {
private:
    int irSize;
    float irBuffer[1024];
    int ratio = 4;

    void init();

    Interpolator interpolator;
    Decimator decimator;

    float *interpBuffer;
    float *decimBuffer;

public:
    bool enabled;

    NeoOversampler();


    virtual float process(float x) { return x; };

    float compute(float x);

};


/**
 * @brief Just for test!
 */
struct TanhOS : NeoOversampler {

    float gain = 0.f;

    TanhOS();

    virtual float process(float x) override;

};


}

