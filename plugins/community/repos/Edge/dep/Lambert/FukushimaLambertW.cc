/*
  Implementation of the Fukushima method for the Lambert W function

  Copyright (C) 2015 Darko Veberic, darko.veberic@ijs.si

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

/*
  This code is based on the following publication and its author's fortran code:
  Toshio Fukushima, "Precise and fast computation of Lambert W-functions without
  transcendental function evaluations", J. Comp. Appl. Math. 244 (2013) 77-89.
*/

#include <FukushimaLambertW.h>
#include <cmath>
#include <limits>
#include <iostream>

using namespace std;


namespace Fukushima {

  double
  LambertWSeries(const double p)
  {
    static const double q[] = {
      -1,
      +1,
      -0.333333333333333333,
      +0.152777777777777778,
      -0.0796296296296296296,
      +0.0445023148148148148,
      -0.0259847148736037625,
      +0.0156356325323339212,
      -0.00961689202429943171,
      +0.00601454325295611786,
      -0.00381129803489199923,
      +0.00244087799114398267,
      -0.00157693034468678425,
      +0.00102626332050760715,
      -0.000672061631156136204,
      +0.000442473061814620910,
      -0.000292677224729627445,
      +0.000194387276054539318,
      -0.000129574266852748819,
      +0.0000866503580520812717,
      -0.0000581136075044138168
    };
    const double ap = abs(p);
    if (ap < 0.01159)
      return
        -1 +
        p*(1 +
        p*(q[2] +
        p*(q[3] +
        p*(q[4] +
        p*(q[5] +
        p*q[6]
        )))));
    else if (ap < 0.0766)
      return
        -1 +
        p*(1 +
        p*(q[2] +
        p*(q[3] +
        p*(q[4] +
        p*(q[5] +
        p*(q[6] +
        p*(q[7] +
        p*(q[8] +
        p*(q[9] +
        p*q[10]
        )))))))));
    else
      return
        -1 +
        p*(1 +
        p*(q[2] +
        p*(q[3] +
        p*(q[4] +
        p*(q[5] +
        p*(q[6] +
        p*(q[7] +
        p*(q[8] +
        p*(q[9] +
        p*(q[10] +
        p*(q[11] +
        p*(q[12] +
        p*(q[13] +
        p*(q[14] +
        p*(q[15] +
        p*(q[16] +
        p*(q[17] +
        p*(q[18] +
        p*(q[19] +
        p*q[20]
        )))))))))))))))))));
  }


  inline
  double
  LambertW0ZeroSeries(const double z)
  {
    return
      z*(1 -
      z*(1 -
      z*(1.5 -
      z*(2.6666666666666666667 -
      z*(5.2083333333333333333 -
      z*(10.8 -
      z*(23.343055555555555556 -
      z*(52.012698412698412698 -
      z*(118.62522321428571429 -
      z*(275.57319223985890653 -
      z*(649.78717234347442681 -
      z*(1551.1605194805194805 -
      z*(3741.4497029592385495 -
      z*(9104.5002411580189358 -
      z*(22324.308512706601434 -
      z*(55103.621972903835338 -
      z*136808.86090394293563
      ))))))))))))))));
  }


  inline
  double
  FinalResult(const double w, const double y)
  {
    const double f0 = w - y;
    const double f1 = 1 + y;
    const double f00 = f0 * f0;
    const double f11 = f1 * f1;
    const double f0y = f0 * y;
    return w - 4 * f0 * (6 * f1 * (f11 + f0y) + f00 * y) /
                        (f11 * (24 * f11 + 36 * f0y) +
                         f00 * (6 * y * y + 8 * f1 * y + f0y));
  }


  double
  LambertW0(const double z)
  {
    static double e[66];
    static double g[65];
    static double a[12];
    static double b[12];

    if (!e[0]) {
      const double e1 = 1 / M_E;
      double ej = 1;
      e[0] = M_E;
      e[1] = 1;
      g[0] = 0;
      for (int j = 1, jj = 2; jj < 66; ++jj) {
        ej *= M_E;
        e[jj] = e[j] * e1;
        g[j] = j * ej;
        j = jj;
      }
      a[0] = sqrt(e1);
      b[0] = 0.5;
      for (int j = 0, jj = 1; jj < 12; ++jj) {
        a[jj] = sqrt(a[j]);
        b[jj] = b[j] * 0.5;
        j = jj;
      }
    }
    if (abs(z) < 0.05)
      return LambertW0ZeroSeries(z);
    if (z < -0.35) {
      const double p2 = 2 * (M_E * z + 1);
      if (p2 > 0)
        return LambertWSeries(sqrt(p2));
      if (p2 == 0)
        return -1;
      cerr << "(lambertw0) Argument out of range. z=" << z << endl;
      return numeric_limits<double>::quiet_NaN();
    }
    int n;
    for (n = 0; n <= 2; ++n)
      if (g[n] > z)
        goto line1;
    n = 2;
    for (int j = 1; j <= 5; ++j) {
      n *= 2;
      if (g[n] > z)
        goto line2;
    }
    cerr << "(lambertw0) Argument too large. z=" << z << endl;
    return numeric_limits<double>::quiet_NaN();
  line2:
    {
      int nh = n / 2;
      for (int j = 1; j <= 5; ++j) {
        nh /= 2;
        if (nh <= 0)
          break;
        if (g[n-nh] > z)
          n -= nh;
      }
    }
  line1:
    --n;
    int jmax = 8;
    if (z <= -0.36)
      jmax = 12;
    else if (z <= -0.3)
      jmax = 11;
    else if (n <= 0)
      jmax = 10;
    else if (n <= 1)
      jmax = 9;
    double y = z * e[n+1];
    double w = n;
    for (int j = 0; j < jmax; ++j) {
      const double wj = w + b[j];
      const double yj = y * a[j];
      if (wj < yj) {
        w = wj;
        y = yj;
      }
    }
    return FinalResult(w, y);
  }


  double
  LambertWm1(const double z)
  {
    static double e[64];
    static double g[64];
    static double a[12];
    static double b[12];

    if (!e[0]) {
      const double e1 = 1 / M_E;
      double ej = e1;
      e[0] = M_E;
      g[0] = -e1;
      for (int j = 0, jj = 1; jj < 64; ++jj) {
        ej *= e1;
        e[jj] = e[j] * M_E;
        g[jj] = -(jj+1) * ej;
        j = jj;
      }
      a[0] = sqrt(M_E);
      b[0] = 0.5;
      for (int j = 0, jj = 1; jj < 12; ++jj) {
        a[jj] = sqrt(a[j]);
        b[jj] = b[j] * 0.5;
        j = jj;
      }
    }
    if (z >= 0) {
      cerr << "(lambertwm1) Argument out of range. z=" << z << endl;
      return numeric_limits<double>::quiet_NaN();
    }
    if (z < -0.35) {
      const double p2 = 2 * (M_E * z + 1);
      if (p2 > 0)
        return LambertWSeries(-sqrt(p2));
      if (p2 == 0)
        return -1;
      cerr << "(lambertwm1) Argument out of range. z=" << z << endl;
      return numeric_limits<double>::quiet_NaN();
    }
    int n = 2;
    if (g[n - 1] > z)
      goto line1;
    for (int j = 1; j <= 5; ++j) {
      n *= 2;
      if (g[n - 1] > z)
        goto line2;
    }
    cerr << "(lambertwm1) Argument too small. z=" << z << endl;
    return numeric_limits<double>::quiet_NaN();
  line2:
    {
      int nh = n / 2;
      for (int j = 1; j <= 5; ++j) {
        nh /= 2;
        if (nh <= 0)
          break;
        if (g[n-nh - 1] > z)
          n -= nh;
      }
    }
  line1:
    --n;
    int jmax = 11;
    if (n >= 8)
      jmax = 8;
    else if (n >= 3)
      jmax = 9;
    else if (n >= 2)
      jmax = 10;
    double w = -n;
    double y = z * e[n - 1];
    for (int j = 0; j < jmax; ++j) {
      const double wj = w - b[j];
      const double yj = y * a[j];
      if (wj < yj) {
        w = wj;
        y = yj;
      }
    }
    return FinalResult(w, y);
  }

}
