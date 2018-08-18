/*
  Command-line test of accuracy of Lambert W function implementations

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

#include <LambertW.h>
#include <FukushimaLambertW.h>
#include <iostream>
#include <stdlib.h>
#include <cmath>

using namespace std;


inline
bool
CloseTo(const double a, const double b, const double eps)
{
  return fabs(a - b) < eps;
}


int
main()
{
  const double eps = 3e-15;
  const double step1 = 0.0001;
  const double max2 = 30;
  const double step2 = 0.0001;

  for (int branch = -1; branch <= 0; ++branch)
    for (double x = -1/M_E + eps; x < 0; x += step1) {
      const double w = utl::LambertW(branch, x);
      const double fw = Fukushima::LambertW(branch, x);
      if (!CloseTo(w, fw, eps)) {
        cout << 'f' << branch << '(' << x << ") = " << (w - fw) << endl;
        return EXIT_FAILURE;
      }
    }

  for (double x = 0; x < max2; x += step2) {
    const double w = utl::LambertW(0, x);
    const double fw = Fukushima::LambertW(0, x);
    if (!CloseTo(w, fw, eps)) {
      cout << "f0(" << x << ") = " << (w - fw) << endl;
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
