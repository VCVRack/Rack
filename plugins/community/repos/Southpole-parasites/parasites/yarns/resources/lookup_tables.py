#!/usr/bin/python2.5
#
# Copyright 2014 Olivier Gillet.
#
# Author: Olivier Gillet (ol.gillet@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 
# See http://creativecommons.org/licenses/MIT/ for more information.
#
# -----------------------------------------------------------------------------
#
# Lookup table definitions.

import numpy

"""----------------------------------------------------------------------------
LFO and portamento increments.
----------------------------------------------------------------------------"""

lookup_tables_32 = []

sample_rate = 4000
min_frequency = 1.0 / 2.0  # Hertz
max_frequency = 16.0  # Hertz

excursion = 1 << 32
num_values = 128
min_increment = excursion * min_frequency / sample_rate
max_increment = excursion * max_frequency / sample_rate

rates = numpy.linspace(numpy.log(min_increment),
                       numpy.log(max_increment), num_values)
lookup_tables_32.append(
    ('lfo_increments', numpy.exp(rates).astype(int))
)


# Create lookup table for portamento.
max_time = 6.0  # seconds
min_time = 3.0 / sample_rate
gamma = 0.25
min_increment = excursion / (max_time * sample_rate)
max_increment = excursion / (min_time * sample_rate)

rates = numpy.linspace(numpy.power(max_increment, -gamma),
                       numpy.power(min_increment, -gamma), num_values)

values = numpy.power(rates, -1/gamma).astype(int)
lookup_tables_32.append(
    ('portamento_increments', values)
)


sample_rate = 48000

# Create table for pitch.
a4_midi = 69
a4_pitch = 440.0
highest_octave = 116
notes = numpy.arange(
    highest_octave * 128.0,
    (highest_octave + 12) * 128.0 + 16,
    16)
pitches = a4_pitch * 2 ** ((notes - a4_midi * 128) / (128 * 12))
increments = excursion / sample_rate * pitches

lookup_tables_32.append(
    ('oscillator_increments', increments.astype(int)))



"""----------------------------------------------------------------------------
Envelope curves
-----------------------------------------------------------------------------"""

lookup_tables = []
lookup_tables_signed = []

env_linear = numpy.arange(0, 257.0) / 256.0
env_linear[-1] = env_linear[-2]
env_expo = 1.0 - numpy.exp(-4 * env_linear)
lookup_tables.append(('env_expo', env_expo / env_expo.max() * 65535.0))



"""----------------------------------------------------------------------------
Arpeggiator patterns
----------------------------------------------------------------------------"""

def XoxTo16BitInt(pattern):
  uint16 = 0
  i = 0
  for char in pattern:
    if char == 'o':
      uint16 += (2 ** i)
      i += 1
    elif char == '-':
      i += 1
  assert i == 16
  return uint16


def ConvertPatterns(patterns):
  return [XoxTo16BitInt(pattern) for pattern in patterns]


lookup_tables.append(
  ('arpeggiator_patterns', ConvertPatterns([
      'o-o- o-o- o-o- o-o-',
      'o-o- oooo o-o- oooo',
      'o-o- oo-o o-o- oo-o',
      'o-o- o-oo o-o- o-oo',
      'o-o- o-o- oo-o -o-o',
      'o-o- o-o- o--o o-o-',
      'o-o- o--o o-o- o--o',
      
      'o--o ---- o--o ----',
      'o--o --o- -o-- o--o',
      'o--o --o- -o-- o-o-',
      'o--o --o- o--o --o-',
      'o--o o--- o-o- o-oo',
      
      'oo-o -oo- oo-o -oo-',
      'oo-o o-o- oo-o o-o-',
      
      'ooo- ooo- ooo- ooo-',
      'ooo- oo-o o-oo -oo-',
      'ooo- o-o- ooo- o-o-',
      
      'oooo -oo- oooo -oo-',
      'oooo o-oo -oo- ooo-',
      
      'o--- o--- o--o -o-o',
      'o--- --oo oooo -oo-',
      'o--- ---- o--- o-oo'])))
      

      
"""----------------------------------------------------------------------------
Euclidean patterns
----------------------------------------------------------------------------"""

def Flatten(l):
  if hasattr(l, 'pop'):
    for item in l:
      for j in Flatten(item):
        yield j
  else:
    yield l


def EuclideanPattern(k, n):
  pattern = [[1]] * k + [[0]] * (n - k)
  while k:
    cut = min(k, len(pattern) - k)
    k, pattern = cut, [pattern[i] + pattern[k + i] for i in xrange(cut)] + \
      pattern[cut:k] + pattern[k + cut:]
  return pattern


table = []
for num_steps in xrange(1, 33):
  for num_notes in xrange(32):
    num_notes = min(num_notes, num_steps)
    bitmask = 0
    for i, bit in enumerate(Flatten(EuclideanPattern(num_notes, num_steps))):
      if bit:
        bitmask |= (1 << i)
    table.append(bitmask)

lookup_tables_32 += [('euclidean', table)]



"""----------------------------------------------------------------------------
Just intonation tuning table
----------------------------------------------------------------------------"""

intervals = [
    (1, 1),
    (9, 8),
    (5, 4),
    (4, 3),
    (3, 2),
    (5, 3),
    (15, 8),
    (256, 243),
    (16, 15),
    (10, 9),
    (32, 27),
    (6, 5),
    (81, 64),
    (45, 32),
    (1024, 729),
    (64, 45),
    (729, 512),
    (128, 81),
    (8, 5),
    (27, 16),
    (16, 9),
    (9, 5),
    (243, 128),
    # (7, 4),
    # (7, 5),
    # (7, 6),
    # (10, 7),
    # (11, 7),
    # (15, 14),
    # (21, 11)
    ]

consonant_intervals = []
for interval in intervals:
  p, q = interval
  midi_ratio = int(numpy.round(1536 * numpy.log2(float(p) / q)))
  # The larger the number in the numerator and denominator of the fraction
  # representing the interval, the more dissonant it is.
  consonance_score = (numpy.log2(p * q) ** 2)
  consonant_intervals.append((midi_ratio, consonance_score))

consonance_table = [0] * 1536
for i in xrange(1536):
  nearest = numpy.argmin(
      [min(abs(i - p), abs(i - p - 1536)) for (p, _) in consonant_intervals])
  index, consonance_score = consonant_intervals[nearest]
  consonance_score += min((i - index) ** 2, (i - index - 1536) ** 2)
  consonance_table[i] = consonance_score

lookup_tables.append(('consonance', consonance_table))



"""----------------------------------------------------------------------------
List of 22 shrutis with different notation schemes.

The most common notation scheme is in the 3th column.
----------------------------------------------------------------------------"""

shrutis = [
  # Swara ref 1, Swara ref 2, Swara, Swara (carnatic, common), Just, Ratio
  ('S', 'sa', 's', 'C', 1),
  ('r1', 'ra', 'r1', 'pC#', 256.0/243.0),
  ('r2', 'ri', 'r2', 'C#', 16.0/15.0),
  ('R1', 'ru', 'r3', '?', 10.0/9.0),
  ('R2', 're', 'r4', 'D', 9.0/8.0),
  ('g1', 'ga', 'g1', 'pD#', 32.0/27.0),
  ('g2', 'gi', 'g2', 'D#', 6.0/5.0),
  ('G1', 'gu', 'g3', 'E', 5.0/4.0),
  ('G2', 'ge', 'g4', 'pE', 81.0/64.0),
  ('m1', 'ma', 'm1', 'F', 4.0/3.0),
  ('m2', 'mi', 'm2', '?', 27.0/20.0),
  ('M1', 'mu', 'm3', 'F#', 45.0/32.0),
  ('M2', 'me', 'm4', 'pF#', 729.0/512.0),
  ('P', 'pa', 'p', 'G', 3.0/2.0),
  ('d1', 'dha', 'd1', 'pG#', 128.0/81.0),
  ('d2', 'dhi', 'd2', 'G#', 8.0/5.0),
  ('D1', 'dhu', 'd3', 'A', 5.0/3.0),
  ('D2', 'dhe', 'd4', 'pA', 27.0/16.0),
  ('n1', 'na', 'n1', 'A#', 16.0/9.0),
  ('n2', 'ni', 'n2', '?', 9.0/5.0),
  ('N1', 'nu', 'n3', 'B', 15.0/8.0),
  ('N2', 'ne', 'n4', 'pB', 243.0/128.0),
  ('!', '!', '!', '!', 100),
]

def DecodeShrutiChart(line):
  values = [
      's',
      'r1',
      'r2',
      'r3',
      'r4',
      'g1',
      'g2',
      'g3',
      'g4',
      'm1',
      'm2',
      'm3',
      'm4',
      'p',
      'd1',
      'd2',
      'd3',
      'd4',
      'n1',
      'n2',
      'n3',
      'n4'
  ]
  decoded = []
  for i, x in enumerate(line):
    if x != '-':
      decoded.append(values[i])
  return ' '.join(decoded)


"""----------------------------------------------------------------------------
A recommended key on the keyboard for each of the swara.

From:
http://commons.wikimedia.org/wiki/Melakarta_ragams_(svg)
----------------------------------------------------------------------------"""

recommended_keys = {
  's': 0,
  'r1': 1,
  'r2': 1,
  'r3': 2,
  'r4': 2,
  'g1': 3,
  'g2': 3,
  'g3': 4,
  'g4': 4,
  'm1': 5,
  'm2': 6,
  'm3': 6,
  'm4': 6,
  'p': 7,
  'd1': 8,
  'd2': 8,
  'd3': 9,
  'd4': 9,
  'n1': 10,
  'n2': 10,
  'n3': 11,
  'n4': 11
}


shruti_dictionary = {}
for entry in shrutis:
  for name in entry[:-1]:
    shruti_dictionary[name] = entry[-1]


def Compute(scale):
  """Translate a list of 12 note/swaras names into pitch corrections."""
  values = [shruti_dictionary.get(x) for x in scale.split(' ')]
  while 100 in values:
    for i, v in enumerate(values):
      if v == 100:
        values[i] = values[i- 1]
  equal = 2 ** (numpy.arange(12.0) / 12.0)
  shifts = numpy.round((numpy.log2(values / equal) * 12 * 128)).astype(int)
  silences = numpy.where(shifts > 127)
  if len(silences[0]):
    shifts[silences[0]] = 32767
  return shifts


def LayoutRaga(raga, silence_other_notes=False):
  """Find a good assignments of swaras to keys for a raga."""
  raga = raga.lower()
  scale = numpy.zeros((12,))
  mapping = ['' for i in range(12)]
  for swara in raga.split(' '):
    key = recommended_keys.get(swara)
    mapping[key] = swara

  # Fill unassigned notes
  for i, n in enumerate(mapping):
    if n == '':
      if silence_other_notes:
        mapping[i] = '!'
      else:
        candidates = []
        for swara, key in recommended_keys.items():
          if key == i:
            candidates.append(swara)
        for candidate in candidates:
          if candidate[0] != mapping[i - 1]:
            mapping[i] = candidate
            break
        else:
          mapping[i] = candidates[0]
    
  scale = [shruti_dictionary.get(swara) for swara in mapping]
  return Compute(' '.join(mapping))


altered_e_b = [0, 0, 0, 0, -64, 0, 0, 0, 0, 0, 0, -64]
altered_e = [0, 0, 0, 0, -64, 0, 0, 0, 0, 0, 0, 0]
altered_e_a = [0, 0, 0, 0, -64, 0, 0, 0, 0, -64, 0, 0]

scales = [
    ('pythagorean', Compute('C pC# D pD# pE F pF# G pG# pA A# pB')),
    ('1/4 eb', numpy.array(altered_e_b, dtype=int)),
    ('1/4 e', numpy.array(altered_e, dtype=int)),
    ('1/4 ea', numpy.array(altered_e_a, dtype=int)),
    ('bhairav',
     LayoutRaga(DecodeShrutiChart('sr-----g-m---pd-----n-'), True)),
    ('gunakri',
     LayoutRaga(DecodeShrutiChart('s-r------m---p-d------'), True)),
    ('marwa',
     LayoutRaga(DecodeShrutiChart('s-r----g---m----d---n-'), True)),
    ('shree',
     LayoutRaga(DecodeShrutiChart('sr-----g---m-pd-----n-'), True)),
    ('purvi',
     LayoutRaga(DecodeShrutiChart('s-r----g---m-p-d----n-'), True)),
    ('bilawal',
     LayoutRaga(DecodeShrutiChart('s---r--g-m---p---d--n-'), True)),
    ('yaman',
     LayoutRaga(DecodeShrutiChart('s---r---g---mp---d---n'), True)),
    ('kafi',
     LayoutRaga(DecodeShrutiChart('s--r-g---m---p--d-n---'), True)),
    ('bhimpalasree',
     LayoutRaga(DecodeShrutiChart('s---r-g--m---p---d-n--'), True)),
    ('darbari',
     LayoutRaga(DecodeShrutiChart('s---rg---m---pd---n---'), True)),
    ('bageshree',
     LayoutRaga(DecodeShrutiChart('s--r-g---m---p--d-n---'), True)),
    ('rageshree',
     LayoutRaga(DecodeShrutiChart('s---r--g-m---p--d-n---'), True)),
    ('khamaj',
     LayoutRaga(DecodeShrutiChart('s---r--g-m---p---dn--n'), True)),
    ('mi\'mal',
     LayoutRaga(DecodeShrutiChart('s---rg---m---p--d-n-n-'), True)),
    ('parameshwari',
     LayoutRaga(DecodeShrutiChart('sr---g---m------d-n---'), True)),
    ('rangeshwari',
     LayoutRaga(DecodeShrutiChart('s---rg---m---p------n-'), True)),
    ('gangeshwari',
     LayoutRaga(DecodeShrutiChart('s------g-m---pd---n---'), True)),
    ('kameshwari',
     LayoutRaga(DecodeShrutiChart('s---r------m-p--d-n---'), True)),
    ('pa. kafi',
     LayoutRaga(DecodeShrutiChart('s---rg---m---p---dn---'), True)),
    ('natbhairav',
     LayoutRaga(DecodeShrutiChart('s---r--g-m---pd-----n-'), True)),
    ('m.kauns',
     LayoutRaga(DecodeShrutiChart('s---r---gm----d---n---'), True)),
    ('bairagi',
     LayoutRaga(DecodeShrutiChart('sr-------m---p----n---'), True)),
    ('b.todi',
     LayoutRaga(DecodeShrutiChart('sr---g-------p----n---'), True)),
    ('chandradeep',
     LayoutRaga(DecodeShrutiChart('s----g---m---p----n---'), True)),
    ('kaushik todi',
     LayoutRaga(DecodeShrutiChart('s----g---m-m--d-------'), True)),
    ('jogeshwari',
     LayoutRaga(DecodeShrutiChart('s----g-g-m------d-n---'), True)),
    ('rasia',
     LayoutRaga(DecodeShrutiChart('s---r---g---mp---d---n'), True)),
]

for scale, values in scales:
  lookup_tables_signed.append(('scale_%s' % scale, values))
