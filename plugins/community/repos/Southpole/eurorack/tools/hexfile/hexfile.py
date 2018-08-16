#!/usr/bin/python2.5
#
# Copyright 2009 Olivier Gillet.
#
# Author: Olivier Gillet (ol.gillet@gmail.com)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# -----------------------------------------------------------------------------
#
# Python module for loading/writing Hex files.

"""Intel .hex file loader/writer"""

import logging
import sys


def LoadHexFile(lines):
  """Loads a Hex file."""

  data = []
  for line_number, line in enumerate(lines):
    line = line.strip()
    if len(line) < 9:
      logging.info('Line %(line_number)d: line too short' % locals())
      return None

    if not all(x in '0123456789abcdefABCDEF' for x in line[1:]):
      logging.info('Line %(line_number)d: unknown character' % locals())
      return None

    bytes = [int(line[i:i+2], 16) for i in xrange(1, len(line), 2)]
    if bytes[0] != len(bytes) - 5:
      logging.info('Line %(line_number)d: invalid byte count' % locals())
      return None

    if sum(bytes) % 256 != 0:
      logging.info('Line %(line_number)d: corrupted line' % locals())
      return None

    if bytes[3] == 1:
      if bytes[0] != 0 or bytes[1] != 0 or bytes[2] != 0:
        logging.info('Line %(line_number)d: invalid end of file' % locals())
        return None
      else:
        break
    elif bytes[3] == 0:
      address = bytes[1] << 8 | bytes[2]
      padding_size = address + bytes[0] - len(data)
      if padding_size > 0:
        data += [0] * padding_size
      data[address:address + bytes[0]] = bytes[4:-1]
  return data


def WriteHexFile(data, file_object, chunk_size=32):
  """Writes a Hex file."""

  for address in xrange(0, len(data), chunk_size):
    chunk = data[address:address+chunk_size]
    chunk_len = len(chunk)
    address_l = address & 255
    address_h = address >> 8
    file_object.write(':%(chunk_len)02x%(address_h)02x%(address_l)02x00' % vars())
    file_object.write(''.join('%02x' % value for value in chunk))
    checksum = (-(chunk_len + address_l + address_h + sum(chunk))) & 255
    file_object.write('%02x\n' % checksum)
  file_object.write(':00000001FF\n')