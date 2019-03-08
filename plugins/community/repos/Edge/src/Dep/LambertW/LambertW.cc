/*
  Implementation of the Lambert W function

  Copyright (C) 2011 Darko Veberic, darko.veberic@ijs.si

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cmath>
#include "Horner.h"

using namespace std;

// fork macro to keep the tree balanced
#define Y2(d1, c12, d2) \
  ((c12) ? (d1) : (d2))
#define Y3(d1, c12, d2, c23, d3) \
  Y2(Y2(d1, c12, d2), c23, d3)
#define Y4(d1, c12, d2, c23, d3, c34, d4) \
  Y3(d1, c12, d2, c23, Y2(d3, c34, d4))
#define Y5(d1, c12, d2, c23, d3, c34, d4, c45, d5) \
  Y4(Y2(d1, c12, d2), c23, d3, c34, d4, c45, d5)
#define Y6(d1, c12, d2, c23, d3, c34, d4, c45, d5, c56, d6) \
  Y5(d1, c12, d2, c23, d3, c34, d4, c45, Y2(d5, c56, d6))
#define Y7(d1, c12, d2, c23, d3, c34, d4, c45, d5, c56, d6, c67, d7) \
  Y6(d1, c12, d2, c23, Y2(d3, c34, d4), c45, d5, c56, d6, c67, d7)


namespace utl {

  class BranchPoint { };
  HORNER_COEFF(BranchPoint,  0, -1);
  HORNER_COEFF(BranchPoint,  1,  1);
  HORNER_COEFF(BranchPoint,  2, -0.333333333333333333e0);
  HORNER_COEFF(BranchPoint,  3,  0.152777777777777777e0);
  HORNER_COEFF(BranchPoint,  4, -0.79629629629629630e-1);
  HORNER_COEFF(BranchPoint,  5,  0.44502314814814814e-1);
  HORNER_COEFF(BranchPoint,  6, -0.25984714873603760e-1);
  HORNER_COEFF(BranchPoint,  7,  0.15635632532333920e-1);
  HORNER_COEFF(BranchPoint,  8, -0.96168920242994320e-2);
  HORNER_COEFF(BranchPoint,  9,  0.60145432529561180e-2);
  HORNER_COEFF(BranchPoint, 10, -0.38112980348919993e-2);
  HORNER_COEFF(BranchPoint, 11,  0.24408779911439826e-2);
  HORNER_COEFF(BranchPoint, 12, -0.15769303446867841e-2);
  HORNER_COEFF(BranchPoint, 13,  0.10262633205076071e-2);
  HORNER_COEFF(BranchPoint, 14, -0.67206163115613620e-3);
  HORNER_COEFF(BranchPoint, 15,  0.44247306181462090e-3);
  HORNER_COEFF(BranchPoint, 16, -0.29267722472962746e-3);
  HORNER_COEFF(BranchPoint, 17,  0.19438727605453930e-3);
  HORNER_COEFF(BranchPoint, 18, -0.12957426685274883e-3);
  HORNER_COEFF(BranchPoint, 19,  0.86650358052081260e-4);

  template<unsigned int order>
  class AsymptoticPolynomialB { };
  //HORNER_COEFF(AsymptoticPolynomialB<0>, 0, 0);
  //HORNER_COEFF(AsymptoticPolynomialB<0>, 1, -1);
  //HORNER_COEFF(AsymptoticPolynomialB<1>, 0, 0);
  //HORNER_COEFF(AsymptoticPolynomialB<1>, 1, 1);
  HORNER_COEFF(AsymptoticPolynomialB<2>, 0, 0);
  HORNER_COEFF(AsymptoticPolynomialB<2>, 1, -1);
  HORNER_COEFF(AsymptoticPolynomialB<2>, 2, 1./2);
  HORNER_COEFF(AsymptoticPolynomialB<3>, 0, 0);
  HORNER_COEFF(AsymptoticPolynomialB<3>, 1, 1);
  HORNER_COEFF(AsymptoticPolynomialB<3>, 2, -3./2);
  HORNER_COEFF(AsymptoticPolynomialB<3>, 3, 1./3);
  HORNER_COEFF(AsymptoticPolynomialB<4>, 0, 0);
  HORNER_COEFF(AsymptoticPolynomialB<4>, 1, -1);
  HORNER_COEFF(AsymptoticPolynomialB<4>, 2, 3);
  HORNER_COEFF(AsymptoticPolynomialB<4>, 3, -11./6);
  HORNER_COEFF(AsymptoticPolynomialB<4>, 4, 1./4);
  HORNER_COEFF(AsymptoticPolynomialB<5>, 0, 0);
  HORNER_COEFF(AsymptoticPolynomialB<5>, 1, 1);
  HORNER_COEFF(AsymptoticPolynomialB<5>, 2, -5);
  HORNER_COEFF(AsymptoticPolynomialB<5>, 3, 35./6);
  HORNER_COEFF(AsymptoticPolynomialB<5>, 4, -25./12);
  HORNER_COEFF(AsymptoticPolynomialB<5>, 5, 1./5);
  class AsymptoticPolynomialA { };
  //HORNER_COEFF2(AsymptoticPolynomialA, 0, (Horner<Float, AsymptoticPolynomialB<0>, 1>::Eval(y)));
  HORNER_COEFF2(AsymptoticPolynomialA, 0, -y);
  //HORNER_COEFF2(AsymptoticPolynomialA, 1, (Horner<Float,  AsymptoticPolynomialB<1>, 1>::Eval(y)));
  HORNER_COEFF2(AsymptoticPolynomialA, 1, y);
  HORNER_COEFF2(AsymptoticPolynomialA, 2, (Horner<Float,  AsymptoticPolynomialB<2>, 2>::Eval(y)));
  HORNER_COEFF2(AsymptoticPolynomialA, 3, (Horner<Float,  AsymptoticPolynomialB<3>, 3>::Eval(y)));
  HORNER_COEFF2(AsymptoticPolynomialA, 4, (Horner<Float,  AsymptoticPolynomialB<4>, 4>::Eval(y)));
  HORNER_COEFF2(AsymptoticPolynomialA, 5, (Horner<Float,  AsymptoticPolynomialB<5>, 5>::Eval(y)));


  template<typename Float, int order>
  inline
  Float
  AsymptoticExpansionImpl(const Float a, const Float b)
  {
    return a + Horner<Float, AsymptoticPolynomialA, order>::Eval(1/a, b);
  }


  template<typename Float, int branch, int order>
  struct LogRecursionImpl {
    enum { eSign = 2*branch + 1 };
    static Float Step(const Float logsx)
    { return logsx - log(eSign * LogRecursionImpl<Float, branch, order-1>::Step(logsx)); }
  };

  template<typename Float, int branch>
  struct LogRecursionImpl<Float, branch, 0> {
    static Float Step(const Float logsx) { return logsx; }
  };


  template<typename Float, int branch>
  class Branch {
  public:
    template<int order>
    static Float BranchPointExpansion(const Float x)
    { return Horner<Float, BranchPoint, order>::Eval(eSign * sqrt(2*(Float(M_E)*x+1))); }

    // Asymptotic expansion: Corless et al. 1996, de Bruijn (1981)
    template<int order>
    static
    Float
    AsymptoticExpansion(const Float x)
    {
      const Float logsx = log(eSign * x);
      const Float logslogsx = log(eSign * logsx);
      return AsymptoticExpansionImpl<Float, order>(logsx, logslogsx);
    }

    // Logarithmic recursion
    template<int order>
    static Float LogRecursion(const Float x)
    { return LogRecursionImpl<Float, branch, order>::Step(log(eSign * x)); }

  private:
    enum { eSign = 2*branch + 1 };
  };


  // iterations

  template<typename Float>
  inline
  Float
  HalleyStep(const Float x, const Float w)
  {
    const Float ew = exp(w);
    const Float wew = w * ew;
    const Float wewx = wew - x;
    const Float w1 = w + 1;
    return w - wewx / (ew * w1 - (w + 2) * wewx/(2*w1));
  }


  template<typename Float>
  inline
  Float
  FritschStep(const Float x, const Float w)
  {
    const Float z = log(x/w) - w;
    const Float w1 = w + 1;
    const Float q = 2 * w1 * (w1 + Float(2./3)*z);
    const Float eps = z / w1 * (q - z) / (q - 2*z);
    return w * (1 + eps);
  }


  template<typename Float>
  inline
  Float
  SchroederStep(const Float x, const Float w)
  {
    const Float y = x * exp(-w);
    const Float f0 = w - y;
    const Float f1 = 1 + y;
    const Float f00 = f0*f0;
    const Float f11 = f1*f1;
    const Float f0y = f0*y;
    return w - 4*f0*(6*f1*(f11 + f0y) + f00*y) / (f11*(24*f11 + 36*f0y) + f00*(6*y*y + 8*f1*y + f0y));
  }


  template<
    typename Float,
    double IterationStep(const Float x, const Float w)
  >
  struct Iterator {

    template<int n, class = void>
    struct Depth {
      static Float Recurse(const Float x, Float w)
      { return Depth<n-1>::Recurse(x, IterationStep(x, w)); }
    };

    // stop condition
    template<class T>
    struct Depth<1, T> {
      static Float Recurse(const Float x, const Float w) { return IterationStep(x, w); }
    };

    // identity
    template<class T>
    struct Depth<0, T> {
      static Float Recurse(const Float x, const Float w) { return w; }
    };

  };


  // Rational approximations

  template<typename Float, int branch, int n>
  struct Pade {
    static inline Float Approximation(const Float x);
  };


  template<typename Float>
  struct Pade<Float, 0, 1> {
    static inline Float Approximation(const Float x)
    { return x * HORNER4(Float, x, 0.07066247420543414, 2.4326814530577687, 6.39672835731526, 4.663365025836821, 0.99999908757381) /
                 HORNER4(Float, x, 1.2906660139511692, 7.164571775410987, 10.559985088953114, 5.66336307375819, 1); }
  };


  template<typename Float>
  struct Pade<Float, 0, 2> {
    static inline Float Approximation(const Float x)
    { const Float y = log(Float(0.5)*x) - 2;
      return 2 + y * HORNER3(Float, y, 0.00006979269679670452, 0.017110368846615806, 0.19338607770900237, 0.6666648896499793) /
                     HORNER2(Float, y, 0.0188060684652668, 0.23451269827133317, 1); }
  };


  template<typename Float>
  struct Pade<Float, -1, 4> {
    static inline Float Approximation(const Float x)
    { return HORNER4(Float, x, -2793.4565508841197, -1987.3632221106518, 385.7992853617571, 277.2362778379572, -7.840776922133643) /
             HORNER4(Float, x, 280.6156995997829, 941.9414019982657, 190.64429338894644, -63.93540494358966, 1); }
  };


  template<typename Float>
  struct Pade<Float, -1, 5> {
    static inline Float Approximation(const Float x)
    { const Float y = log(-x);
      return -exp(
        HORNER3(Float, y, 0.16415668298255184, -3.334873920301941, 2.4831415860003747, 4.173424474574879) /
        HORNER3(Float, y, 0.031239411487374164, -1.2961659693400076, 4.517178492772906, 1)
      ); }
  };


  template<typename Float>
  struct Pade<Float, -1, 6> {
    static inline Float Approximation(const Float x)
    { const Float y = log(-x);
      return -exp(
        HORNER4(Float, y, 0.000026987243254533254, -0.007692106448267341, 0.28793461719300206, -1.5267058884647018, -0.5370669268991288) /
        HORNER4(Float, y, 3.6006502104930343e-6, -0.0015552463555591487, 0.08801194682489769, -0.8973922357575583, 1)
      ); }
  };


  template<typename Float>
  struct Pade<Float, -1, 7> {
    static inline Float Approximation(const Float x)
    { return -1 - sqrt(
        HORNER4(Float, x, 988.0070769375508, 1619.8111957356814, 989.2017745708083, 266.9332506485452, 26.875022558546036) /
        HORNER4(Float, x, -205.50469464210596, -270.0440832897079, -109.554245632316, -11.275355431307334, 1)
      ); }
  };


  template<int branch>
  double LambertW(const double x);


  template<>
  double
  LambertW<0>(const double x)
  {
    typedef double d;
    return Y5(
      (Branch<d, 0>::BranchPointExpansion<8>(x)),
      x < -0.367679,
      (Iterator<d, HalleyStep<d> >::Depth<1>::Recurse(x, Branch<d, 0>::BranchPointExpansion<10>(x))),
      x < -0.311,
      (Iterator<d, HalleyStep<d> >::Depth<1>::Recurse(x, Pade<d, 0, 1>::Approximation(x))),
      x < 1.38,
      (Iterator<d, HalleyStep<d> >::Depth<1>::Recurse(x, Pade<d, 0, 2>::Approximation(x))),
      x < 236,
      (Iterator<d, HalleyStep<d> >::Depth<1>::Recurse(x, Branch<d, 0>::AsymptoticExpansion<6-1>(x)))
    );
  }


  template<>
  double
  LambertW<-1>(const double x)
  {
    typedef double d;
    return Y7(
      (Branch<d, -1>::BranchPointExpansion<8>(x)),
      x < -0.367579,
      (Iterator<d, HalleyStep<d> >::Depth<1>::Recurse(x, Branch<d, -1>::BranchPointExpansion<4>(x))),
      x < -0.366079,
      (Iterator<d, HalleyStep<d> >::Depth<1>::Recurse(x, Pade<d, -1, 7>::Approximation(x))),
      x < -0.289379,
      (Iterator<d, HalleyStep<d> >::Depth<1>::Recurse(x, Pade<d, -1, 4>::Approximation(x))),
      x < -0.0509,
      (Iterator<d, HalleyStep<d> >::Depth<1>::Recurse(x, Pade<d, -1, 5>::Approximation(x))),
      x < -0.000131826,
      (Iterator<d, HalleyStep<d> >::Depth<1>::Recurse(x, Pade<d, -1, 6>::Approximation(x))),
      x < -6.30957e-31,
      (Iterator<d, HalleyStep<d> >::Depth<1>::Recurse(x, Branch<d, -1>::LogRecursion<3>(x)))
    );
  }


  // instantiations
  template double LambertW<0>(const double x);
  template double LambertW<-1>(const double x);

}
