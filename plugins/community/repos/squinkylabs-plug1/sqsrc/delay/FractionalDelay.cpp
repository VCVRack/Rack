#include "FractionalDelay.h"

#include <assert.h>


float FractionalDelay::getOutput()
{
#ifdef _LOG
    printf("\n");
#endif
    int delayTimeSamples = (int) delayTime;
    const double x = delayTime - delayTimeSamples;

    const double y0 = getDelayedOutput(delayTimeSamples - 1);
    const double y1 = getDelayedOutput(delayTimeSamples);
    const double y2 = getDelayedOutput(delayTimeSamples + 1);
    const double y3 = getDelayedOutput(delayTimeSamples + 2);
#ifdef _LOG
    printf("dt=%.2f, dts=%d x=%.2f ", delayTime, delayTimeSamples, x);
    printf("y0=%.2f y1=%.2f y2=%.2f y3=%.2f\n", y0, y1, y2, y3);
#endif
 
    const double x0 = -1.0;
    const double x1 = 0.0;
    const double x2 = 1.0;
    const double x3 = 2.0;
    assert(x >= x1);
    assert(x <= x2);

    double dRet = -(1.0 / 6.0)*y0*(x - x1)*(x - x2)*(x - x3);
    dRet += (1.0 / 2.0)* y1*(x - x0) * (x - x2) * (x - x3);
    dRet += (-1.0 / 2.0)*y2*(x - x0) * (x - x1) * (x - x3);
    dRet += (1.0 / 6.0) * y3*(x - x0) * (x - x1) * (x - x2);

#if 0
    static int ct = 0;
    if (++ct > 100) {
        ct = 0;
        char buf[500];
        sprintf(buf, "del = %f int=%d, rem=%f ret=%f\n", time, delayTimeSamples, x, dRet);
        DebugUtil::trace(buf);
        sprintf(buf, "  y0=%f y1=%f y2=%f y3=%f\n", y0, y1, y2, y3);
        DebugUtil::trace(buf);


    }
#endif

    return float(dRet);

}

float FractionalDelay::getDelayedOutput(int delaySamples)
{
    int index = inputPointerIndex;

    index -= delaySamples;			// indexes increase as time goes by,
                                    // to the output (in the past), is at a lower index
    if (index < 0) {
        //int n = state.numSamples'
        index += numSamples;

        assert(index >= 0 && index < numSamples);
    }
#ifdef _LOG
    printf("getting output from area of %d\n", index);
#endif

    return delayMemory[index];

}

void FractionalDelay::setInput(float input)
{
    //printf("setting input at %d\n", inputPointerIndex);
    delayMemory[inputPointerIndex++] = input;
    if (inputPointerIndex >= numSamples) {
        inputPointerIndex = 0;
    }
}


/*
float run(float input)
{
float ret = getOutput();
setInput(input);
return ret;
}
*/
float RecirculatingFractionalDelay::run(float input)
{
    float output = getOutput();
    input += (output * feedback);
    setInput(input);
    return output;
}
