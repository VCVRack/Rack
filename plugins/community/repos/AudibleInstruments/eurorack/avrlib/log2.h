// Copyright 2009 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// Template class for converting a power of 2 to its logarithm in base 2.
// Using log might simply not work for template arguments.

#ifndef AVRLIB_LOG2_H_
#define AVRLIB_LOG2_H_

namespace avrlib {

template<uint8_t x>
struct Log2 {
  enum {
    value = 0
  };
};

template<> struct Log2<1> { enum { value = 0 }; };
template<> struct Log2<2> { enum { value = 1 }; };
template<> struct Log2<4> { enum { value = 2 }; };
template<> struct Log2<8> { enum { value = 3 }; };
template<> struct Log2<16> { enum { value = 4 }; };
template<> struct Log2<32> { enum { value = 5 }; };
template<> struct Log2<64> { enum { value = 6 }; };
template<> struct Log2<128> { enum { value = 7 }; };

}  // namespace avrlib

#endif   // AVRLIB_LOG2_H_
