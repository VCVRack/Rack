// Copyright 2015 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Sample rate conversion filters.

#ifndef WARPS_DSP_SAMPLE_RATE_CONVERSION_FILTERS_H_
#define WARPS_DSP_SAMPLE_RATE_CONVERSION_FILTERS_H_

namespace warps {

// Generated with:
// 6 * scipy.signal.remez(48, [0, 0.060000 / 6, 0.5 / 6, 0.5], [1, 0])
template<>
struct SRC_FIR<SRC_UP, 6, 48> {
  template<int32_t i> inline float Read() const {
    const float h[] = {
       4.357278576e-04, -2.297029461e-03, -4.703810602e-03, -8.774604727e-03,
      -1.433899145e-02, -2.112793398e-02, -2.853108802e-02, -3.552868193e-02,
      -4.069862931e-02, -4.228981313e-02, -3.836519645e-02, -2.700780696e-02,
      -6.569014106e-03,  2.407089704e-02,  6.526452513e-02,  1.164165703e-01,
       1.758932961e-01,  2.410483237e-01,  3.083744498e-01,  3.737697127e-01,
       4.328923682e-01,  4.815728403e-01,  5.162355916e-01,  5.342582974e-01,
    };
    return h[i];
  }
};

// Generated with:
// 1 * scipy.signal.remez(48, [0, 0.060000 / 6, 0.5 / 6, 0.5], [1, 0])
template<>
struct SRC_FIR<SRC_DOWN, 6, 48> {
  template<int32_t i> inline float Read() const {
    const float h[] = {
       7.262130960e-05, -3.828382434e-04, -7.839684337e-04, -1.462434121e-03,
      -2.389831909e-03, -3.521322331e-03, -4.755181337e-03, -5.921446989e-03,
      -6.783104885e-03, -7.048302188e-03, -6.394199409e-03, -4.501301159e-03,
      -1.094835684e-03,  4.011816173e-03,  1.087742085e-02,  1.940276171e-02,
       2.931554935e-02,  4.017472062e-02,  5.139574163e-02,  6.229495212e-02,
       7.214872804e-02,  8.026214006e-02,  8.603926526e-02,  8.904304957e-02,
    };
    return h[i];
  }
};

// Generated with:
// 4 * scipy.signal.remez(48, [0, 0.105000 / 4, 0.5 / 4, 0.5], [1, 0])
template<>
struct SRC_FIR<SRC_UP, 4, 48> {
  template<int32_t i> inline float Read() const {
    const float h[] = {
      -6.014371929e-04, -1.116027480e-03, -1.547569918e-03, -1.288608084e-03,
       2.786886230e-04,  3.529342828e-03,  8.203156385e-03,  1.308970614e-02,
       1.600199910e-02,  1.419074690e-02,  5.231038872e-03, -1.177915684e-02,
      -3.506738553e-02, -5.953252182e-02, -7.699933415e-02, -7.757902368e-02,
      -5.198496872e-02,  5.703716839e-03,  9.559598586e-02,  2.106660616e-01,
       3.371310483e-01,  4.566603688e-01,  5.500087786e-01,  6.012053946e-01,
    };
    return h[i];
  }
};

// Generated with:
// 1 * scipy.signal.remez(48, [0, 0.105000 / 4, 0.5 / 4, 0.5], [1, 0])
template<>
struct SRC_FIR<SRC_DOWN, 4, 48> {
  template<int32_t i> inline float Read() const {
    const float h[] = {
      -1.503592982e-04, -2.790068700e-04, -3.868924795e-04, -3.221520211e-04,
       6.967215575e-05,  8.823357070e-04,  2.050789096e-03,  3.272426536e-03,
       4.000499774e-03,  3.547686724e-03,  1.307759718e-03, -2.944789209e-03,
      -8.766846381e-03, -1.488313045e-02, -1.924983354e-02, -1.939475592e-02,
      -1.299624218e-02,  1.425929210e-03,  2.389899647e-02,  5.266651541e-02,
       8.428276207e-02,  1.141650922e-01,  1.375021946e-01,  1.503013486e-01,
    };
    return h[i];
  }
};

// Generated with:
// 3 * scipy.signal.remez(36, [0, 0.050000 / 3, 0.5 / 3, 0.5], [1, 0])
template<>
struct SRC_FIR<SRC_UP, 3, 36> {
  template<int32_t i> inline float Read() const {
    const float h[] = {
       2.111177486e-04,  9.399136027e-04,  2.516356933e-03,  4.847507152e-03,
       6.912087023e-03,  6.524576194e-03,  8.579855461e-04, -1.203466052e-02,
      -3.103696515e-02, -5.013495031e-02, -5.827142630e-02, -4.183809689e-02,
       1.038391226e-02,  1.014554664e-01,  2.222529437e-01,  3.515426263e-01,
       4.610075226e-01,  5.238640837e-01,
    };
    return h[i];
  }
};

// Generated with:
// 1 * scipy.signal.remez(36, [0, 0.050000 / 3, 0.5 / 3, 0.5], [1, 0])
template<>
struct SRC_FIR<SRC_DOWN, 3, 36> {
  template<int32_t i> inline float Read() const {
    const float h[] = {
       7.037258286e-05,  3.133045342e-04,  8.387856444e-04,  1.615835717e-03,
       2.304029008e-03,  2.174858731e-03,  2.859951820e-04, -4.011553507e-03,
      -1.034565505e-02, -1.671165010e-02, -1.942380877e-02, -1.394603230e-02,
       3.461304086e-03,  3.381848881e-02,  7.408431457e-02,  1.171808754e-01,
       1.536691742e-01,  1.746213612e-01,
    };
    return h[i];
  }
};

}  // namespace warps

#endif  // WARPS_DSP_SAMPLE_RATE_CONVERSION_FILTERS_H_
