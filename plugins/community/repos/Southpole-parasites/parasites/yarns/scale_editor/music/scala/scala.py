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
# Scala reader.

"""Scala reader."""

import math
import re



class ParseError(Exception):
  
  def __init__(self, message):
    self.message = message
    
  def __str__(self):
    return self.message



def _parse_note(x):
  x = re.split('[ !]+', x.strip())[0]
  x = x.replace(',', '')  # Some files use , as a decimal separator in fractions
  if '.' in x:
    return float(x)
  elif '/' in x:
    tokens = x.split('/')
    return 1200.0 * math.log(float(tokens[0]) / float(tokens[1])) / math.log(2)
  else:
    return 1200.0 * math.log(float(x)) / math.log(2)



def parse(lines):
  lines = lines.split('\n')
  lines = [line.strip('\r ') for line in lines]
  lines = [line for line in lines if not line.startswith('!') and line]
  if not lines:
    raise ParseError('Empty file')
    
  comment = lines[0]
  lines = lines[1:]

  if not lines:
    raise ParseError('Invalid number of notes')
  
  try:
    num_notes = int(lines[0])
  except Exception as e:
    raise ParseError('Incorrect number of notes: %s' % lines[0])
    
  lines = lines[1:]
  
  if len(lines) != num_notes:
    raise ParseError('Declared number of notes (%d) does not match number of lines (%d)' % (num_notes, len(lines)))
  
  items = []
  for line in lines:
    try:
      items.append(_parse_note(line))
    except Exception as e:
      raise ParseError('Cannot parse: %s' % line)

  return items



def assign_to_octave(notes):
  adjustments = [0.0] * 12
  allocated = [False] * 12
  for n in [0.0] + notes:
    index = int(round(n / 100.0))
    shift = None
    for candidate_shift in [0, -1, 1, -2, 2, -3, 3, -4, 4]:
      position = index + candidate_shift
      adjustment = n - position * 100.0
      if all([
          position >= 0,
          position < 12,
          adjustment <= 16383/16384.0 * 100.0,
          adjustment >= -100.0,
          not allocated[position % 12]]):
        shift = candidate_shift
        break
    
    if shift != None:
      position = index + shift
      adjustments[position] = n - position * 100.0
      allocated[position] = True
  
  return adjustments



if __name__ == '__main__':
  pass
