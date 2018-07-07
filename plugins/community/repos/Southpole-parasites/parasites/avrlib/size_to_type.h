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
// Template class for converting an integer template argument to the
// corresponding size type.

#ifndef AVRLIB_SIZE_TO_TYPE_H_
#define AVRLIB_SIZE_TO_TYPE_H_

namespace avrlib {

template<uint8_t size>
struct DataTypeForSize {
  typedef uint16_t Type;
};

template<> struct DataTypeForSize<1> { typedef uint8_t Type; };
template<> struct DataTypeForSize<2> { typedef uint8_t Type; };
template<> struct DataTypeForSize<3> { typedef uint8_t Type; };
template<> struct DataTypeForSize<4> { typedef uint8_t Type; };
template<> struct DataTypeForSize<5> { typedef uint8_t Type; };
template<> struct DataTypeForSize<6> { typedef uint8_t Type; };
template<> struct DataTypeForSize<7> { typedef uint8_t Type; };
template<> struct DataTypeForSize<8> { typedef uint8_t Type; };

}  // namespace avrlib

#endif   // AVRLIB_SIZE_TO_TYPE_H_
