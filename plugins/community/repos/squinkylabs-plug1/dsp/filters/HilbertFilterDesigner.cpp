/**
 * HilbertFilterDesigner
 * generate a pair of Hilbert filters
 */

#include "HilbertFilterDesigner.h"
#include "DspFilter.h"
#include "BiquadFilter.h"
#include <memory>


const int hilbertOrder = 6;
const bool dumpHilbert = false;

namespace Dsp {

    /* with these looked up poles, and a "cutoff freq" of 4.
     * the filter is good from 4hz to 4k,
     * with a phase error ripple about +- .15 degrees
     */

    const double leftPoles[] = {
        .3609,
        2.7412,
        11.1573,
        44.7581,
        179.6242,
        798.4578
    };

    const double rightPoles[] = {
        1.2524,
        5.5671,
        22.3423,
        89.6271,
        364.7914,
        2770.1114
    };

    const double shift = 4;

    struct Hilb : Prototype
    {

        double log2(double x)
        {
            return log(x) / log(2.0);
        }
        void dump()
        {
            printf("log2(2) = %f 8=%f\n", log2(2), log2(8));
            double lastLog = 0;
            for (int i = 0; i < 6; ++i) {
                double l = shift * leftPoles[i];
                double r = shift * rightPoles[i];

                printf("i=%d left pole = %f right = %f ratio = %f\n", i, l, r, r / l);

                printf("  log(left)=%f, log(r)=%f\n", log2(l), log2(r));

                if (i == 0) {
                    printf("  delta log(r)=%f\n", log2(r) - log2(l));

                } else {
                    printf("  delta log(l0=%f (r)=%f\n", log2(l) - lastLog, log2(r) - log2(l));
                }
                lastLog = log2(r);


                if (i > 0) {
                    double l0 = leftPoles[i - 1];
                    double r0 = rightPoles[i - 1];
                    printf("L/Lprev = %f, R/Rprev=%f\n", l / l0, r / r0);
                }
            }
        }

        void Design(const Spec &spec)
        {

            int n = spec.order;
            assert(n == hilbertOrder);

            if (dumpHilbert)
                dump();

            const bool side = spec.rollOff > .5 ? true : false;

            SetPoles(n);
            SetZeros(n);

            for (int i = 0; i < n; ++i) {
                double f = side ? leftPoles[i] : rightPoles[i];
                Pole(i) = Complex(-f, 0);	// TODO: why do we need this negative sign?
                Zero(i) = Complex(f, 0);
            }

            m_normal.w = 0;
            m_normal.gain = 1;

#ifdef _LOG
            Roots & zeros = Zeros();
            Roots & poles = Poles();

            for (int i = 0; i < n; ++i) {
                char buf[512];
                sprintf(buf, "in hibdes i=%d pole = %f,%f i zero = %f,%f i\n", i,
                    poles.GetNth(i).real(),
                    poles.GetNth(i).imag(),
                    zeros.GetNth(i).real(),
                    zeros.GetNth(i).imag());
                DebugUtil::trace(buf);

            }
#endif
        }
    };


    struct Hilbert : PoleFilterSpace<Hilb, LowPass, hilbertOrder, 1>
    {
        void SetupAs(bool leftSide, double sampleRate)
        {
            Spec spec;
            spec.order = hilbertOrder;

            spec.cutoffFreq = shift;
            spec.sampleRate = sampleRate;

            spec.rollOff = leftSide ? 1 : 0;		// jam side parameter into this obscure member
            PoleFilterSpace<Hilb, LowPass, hilbertOrder, 1>::Setup(spec);
        }
    };


    /** just a simple allpass, for debugging
    */
#if 0
    struct HilbTest : Prototype
    {
    public:
        void Design(const Spec &spec)
        {
            int n = spec.order;
            assert(n == hilbertOrderTest);

            const bool side = spec.rollOff > .5 ? true : false;


            SetPoles(n);
            SetZeros(n);

            assert(n == 1);

            double f = .25;
            Pole(0) = Complex(-f, 0);
            Zero(0) = Complex(f, 0);


            m_normal.w = 0;
            m_normal.gain = 1;



#ifdef _LOG 
            Roots & zeros = Zeros();
            Roots & poles = Poles();

            for (int i = 0; i < n; ++i) {
                char buf[512];
                sprintf(buf, "i=%d pole = %f,%f i zero = %f,%f i\n", i,
                    poles.GetNth(i).real(),
                    poles.GetNth(i).imag(),
                    zeros.GetNth(i).real(),
                    zeros.GetNth(i).imag());
                DebugUtil::trace(buf);

            }
#endif
        }
    };

    struct HilbertTest : PoleFilterSpace<HilbTest, LowPass, hilbertOrderTest, 1>
    {
        void SetupAs(CalcT cutoffFreq, bool side)
        {
            Spec spec;
            spec.order = hilbertOrderTest;

            spec.cutoffFreq = cutoffFreq * 1;
            spec.sampleRate = 44100;

            spec.rollOff = side ? 1 : 0;		// jam side parameter into this obscure member
            PoleFilterSpace<HilbTest, LowPass, hilbertOrderTest, 1>::Setup(spec);
        }
    };
#endif
}




template <typename T>
void HilbertFilterDesigner<T>::design(double sampleRate, BiquadParams<T, 3>& outSin, BiquadParams<T, 3>& outCos)
{
    Dsp::Hilbert hilbert;

    hilbert.SetupAs(false, sampleRate);

    BiquadFilter<T>::fillFromStages(outSin, hilbert.Stages(), hilbert.GetStageCount());
    hilbert.SetupAs(true, sampleRate);
    BiquadFilter<T>::fillFromStages(outCos, hilbert.Stages(), hilbert.GetStageCount());
}

// Explicit instantiation, so we can put implementation into .cpp file
// TODO: option to take out double version (if we don't need it)
// Or put all in header
template class HilbertFilterDesigner<double>;
template class HilbertFilterDesigner<float>;

