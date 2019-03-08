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

#ifndef _FukushimaLambertW_h_
#define _FukushimaLambertW_h_

#include <limits>


namespace Fukushima {

  double LambertW0(const double x);

  double LambertWm1(const double x);


  inline
  double
  LambertW(const int branch, const double x)
  {
    switch (branch) {
    case -1: return LambertWm1(x);
    case  0: return LambertW0(x);
    default: return std::numeric_limits<double>::quiet_NaN();
    }
  }

}


#endif
