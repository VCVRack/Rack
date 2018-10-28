#pragma once

#include <assert.h>
#include <memory>

/**
 * When ignoring wrap, inputIndex > outputIndex.
 * so output "pull up the rear", reading the samples that were written
 * delayTime samples ago.
 */
class FractionalDelay
{
public:
    FractionalDelay(int numSamples) : numSamples(numSamples), delayMemory( new float[numSamples])
    {
        for (int i = 0; i < numSamples; ++i) {
            delayMemory[i] = 0;
        }
    }
    ~FractionalDelay()
    {
        delete delayMemory;
    }

    void setDelay(float samples)
    {
        assert(samples < numSamples);
        delayTime = samples;
    }
    float run(float input)
    {
        float ret = getOutput();
        setInput(input);
        return ret;
    }
protected:
  
    /**
     * get the fractional delayed output, based in delayTime
     */
    float getOutput();

    /**
     * send the next input to the delay line
     */
    void setInput(float);
private:
    /**
     * get delay output with integer (non-fractional) delay time
     */
    float getDelayedOutput(int delaySamples);

    double delayTime = 0;
    int inputPointerIndex = 0;

    /**
     * The size of the delay line, in samples
     */
    const int numSamples;

    float* delayMemory;
};

class RecirculatingFractionalDelay : public FractionalDelay
{
public:
    RecirculatingFractionalDelay(int numSamples) : FractionalDelay(numSamples)
    {
    }
#if 0
    void setDelay(float samples)
    {
        delay.setDelay(samples);
    }
#endif
    void setFeedback(float in_feedback)
    {
        assert(feedback < 1);
        assert(feedback > -1);
        feedback = in_feedback;
    }

    float run(float);
private:
  //  FractionalDelay delay;
    float feedback = 0;
};

