#pragma once


class ClockMult
{
public:
    /**
     * Clocks the multiplier by one sample.
     * Returns 0 or more high speed clocks.
     */
    void sampleClock();

    /**
     * Sends one reference tick to the multiplier. This is
     * the "input" to the multiplier.
     */
    void refClock();

    /**
     * 0..1 saw at the same rate as multiplier output.
     */
    float getSaw() const
    {
        return sawPhase;
    }

    /**
     * The binary pulse output of the clock multiplier
     * (note: doesn't work yet)
     */
    bool getMultipliedClock() const
    {
        return clockOutValue;
    }

    /**
     * @param x >= 1 is the multiplier factor
     *      x == 0 : free run
     */
    void setMultiplier(int x);

    float _getFreq() const
    {
        return learnedFrequency;
    }

    void setFreeRunFreq(float f)
    {
        freeRunFreq = f;
    }
private:
    enum class State
    {
        INIT,
        TRAINING,
        RUNNING
    };
    int trainingCounter = 12345;
    int learnedPeriod = 999;
    float learnedFrequency = 0;
    int freqMultFactor = 0;         // if 0, free run, else mult by n.
    State state = State::INIT;

    bool clockOutValue = 0;
    int clockOutTimer = 0;

    float sawPhase = 0;
    float freeRunFreq = 0;

    void startNewClock();
    bool isFreeRun() const
    {
        return freqMultFactor == 0;
    }
    void sampleClockFreeRunMode();
    void sampleClockLockedMode();

};