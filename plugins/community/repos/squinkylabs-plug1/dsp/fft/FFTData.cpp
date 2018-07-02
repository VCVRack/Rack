
#include "FFTData.h"
#include "kiss_fft.h"
#include "kiss_fftr.h"

#include <assert.h>

int FFTDataCpx::_count = 0;
FFTDataCpx::FFTDataCpx(int numBins) :
    buffer(numBins)
{
    ++_count;
}

FFTDataCpx::~FFTDataCpx()
{
    // We need to manually delete the cfg, since only "we" know
    // what type it is.
    if (kiss_cfg) {
        free(kiss_cfg);
    }
    --_count;
}

cpx FFTDataCpx::get(int index) const
{
    assert(index < (int)buffer.size());
    return buffer[index];
}


void FFTDataCpx::set(int index, cpx value)
{
    assert(index < (int)buffer.size());
    buffer[index] = value;
}

/******************************************************************/
int FFTDataReal::_count = 0;
FFTDataReal::FFTDataReal(int numBins) :
    buffer(numBins)
{
    ++_count;
}

FFTDataReal::~FFTDataReal()
{
    // We need to manually delete the cfg, since only "we" know
    // what type it is.
    if (kiss_cfg) {
        free(kiss_cfg);
    }
    --_count;
}

float FFTDataReal::get(int index) const
{
    assert(index < (int)buffer.size());
    return buffer[index];
}


void FFTDataReal::set(int index, float value)
{
    assert(index < (int)buffer.size());
    buffer[index] = value;
}
