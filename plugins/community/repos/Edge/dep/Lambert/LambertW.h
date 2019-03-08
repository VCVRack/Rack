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

#ifndef _utl_LambertW_h_
#define _utl_LambertW_h_

#include <limits>


namespace utl {

  template<int branch>
  double LambertW(const double x);


  inline
  double
  LambertW(const int branch, const double x)
  {
    switch (branch) {
    case -1: return LambertW<-1>(x);
    case  0: return LambertW<0>(x);
    default: return std::numeric_limits<double>::quiet_NaN();
    }
  }

}


#endif
