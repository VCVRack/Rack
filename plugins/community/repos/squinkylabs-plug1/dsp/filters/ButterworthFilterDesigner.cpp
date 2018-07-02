/**
 * ButterworthFilterDesigner
 * a bunch of functions for generating the parameters of butterworth filters
 */

#include "ButterworthFilterDesigner.h"
#include "DspFilter.h"
#include "BiquadFilter.h"
#include <memory>

template <typename T>
void ButterworthFilterDesigner<T>::designThreePoleLowpass(BiquadParams<T, 2>& outParams, T frequency)
{
    using Filter = Dsp::ButterLowPass<3, 1>;
    std::unique_ptr<Filter> lp3(new Filter());      // std::make_unique is not until C++14
    lp3->SetupAs(frequency);
    assert(lp3->GetStageCount() == 2);
    BiquadFilter<T>::fillFromStages(outParams, lp3->Stages(), lp3->GetStageCount());
}

template <typename T>
void ButterworthFilterDesigner<T>::designTwoPoleLowpass(BiquadParams<T, 1>& outParams, T frequency)
{
    using Filter = Dsp::ButterLowPass<2, 1>;
    std::unique_ptr<Filter> lp2(new Filter());
    lp2->SetupAs(frequency);
    assert(lp2->GetStageCount() == 1);
    BiquadFilter<T>::fillFromStages(outParams, lp2->Stages(), lp2->GetStageCount());
}

// Explicit instantiation, so we can put implementation into .cpp file
// TODO: option to take out float version (if we don't need it)
// Or put all in header
template class ButterworthFilterDesigner<double>;
template class ButterworthFilterDesigner<float>;
