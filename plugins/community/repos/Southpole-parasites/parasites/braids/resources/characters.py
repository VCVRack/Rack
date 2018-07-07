# Copyright 2012 Olivier Gillet.
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
# Characters definitions.

characters = []


MASKS = {
  'a': 0x8000,
  'b': 0x4000,
  'c': 0x2000,
  'd': 0x1000,
  'e': 0x0800,
  'f': 0x0400,
  'g': 0x0200,
  'h': 0x0100,
  'j': 0x80,
  'k': 0x40,
  'l': 0x20,
  'm': 0x10,
  'n': 0x08,
  'p': 0x04,
}

characters = {
  'A': 'afepkbc',
  'B': 'adhmbck',
  'C': 'afed',
  'D': 'adhmbc',
  'E': 'afedkp',
  'F': 'afepk',
  'G': 'afedck',
  'H': 'febcpk',
  'I': 'adhm',
  'J': 'bcde',
  'K': 'efpjl',
  'L': 'def',
  'M': 'efgjbc',
  'N': 'efglcb',
  'O': 'abcdef',
  'P': 'abpkef',
  'Q': 'abcdefl',
  'R': 'abpkefl',
  'S': 'afpkcd',
  'T': 'ahm',
  'U': 'bcdef',
  'V': 'fenj',
  'W': 'fenlcb',
  'X': 'gjln',
  'Y': 'gjm',
  'Z': 'ajnd',
  
  'a': 'abpkecd',
  'b': 'fedlp',
  'c': 'pked',
  'd': 'bcdnk',
  'e': 'afped',
  'f': 'afpe',
  'g': 'agkbcd',
  'h': 'fpkec',
  'i': 'mpkd',
  'j': 'kcd',
  'k': 'hmjl',
  'l': 'hm',
  'm': 'epkmc',
  'n': 'mkc',
  'o': 'pkecd',
  'p': 'afpje',
  'q': 'afpkbl',
  'r': 'mk',
  's': 'agkcd',
  't': 'fedp',
  'u': 'edc',
  'v': 'en',
  'w': 'enlc',
  'x': 'gnjl',
  'y': 'gkbcd',
  'z': 'pnd',
  
  '0': 'abcdefnj',
  '1': 'jbc',
  '2': 'abpked',
  '3': 'abcdk',
  '4': 'fpkbc',
  '5': 'afpkcd',
  '6': 'afpkcde',
  '7': 'ajm',
  '8': 'abcdefpk',
  '9': 'abcpkfd',

  '!': 'hm',
  '"': 'fh',
  '#': 'pkdhmbc',
  '$': 'afpkcdhm',
  '%': 'jnfc',
  '&': 'aghpeld',
  '\'': 'h',
  '(': 'afed',
  ')': 'abcd',
  '*': 'ghjmnl',
  '+': 'hmpk',
  ',': 'n',
  '-': 'pk',
  '.': 'm',
  '/': 'jn',
  ':': 'hm',
  ';': 'hn',
  '<': 'jl',
  '>': 'gn',
  '?': 'fajm',
  '=': 'pkd',
  '@': 'kmcbafed',
  '[': 'afed',
  ']': 'abcd',
  '\\': 'gl',
  '^': 'nl',
  '_': 'd',
  '`': 'g',
  '{': 'pgnad',
  '|': 'hm',
  '}': 'ajldk',
  '~': 'pk',
  
  # LRDU
  '\x80': 'jlbc',
  '\x81': 'efgn',
  '\x82': 'agj',
  '\x83': 'dnl',
  
  # LRDU arrow
  '\x84': 'jkl',
  '\x85': 'gpn',
  '\x86': 'ghj',
  '\x87': 'nml',
  
  # Waveforms
  '\x88': 'njbc',  # Saw
  '\x89': 'enjb',  # Centered saw
  '\x8A': 'mn',    # Baby saw
  '\x8B': 'nl',    # Tri
  '\x8C': 'efabc', # Square
  '\x8D': 'epkc',   # Baby square
  '\x8E': 'dhm',   # Pulse
  '\x8F': 'efgl', # AD
  
  # Spinner
  '\x90': 'abcdefn',
  '\x91': 'abcdefp',
  '\x92': 'abcdefg',
  '\x93': 'abcdefh',
  '\x94': 'abcdefj',
  '\x95': 'abcdefk',
  '\x96': 'abcdefl',
  '\x97': 'abcdefm',

  # Spinner 2
  '\x98': 'ab',
  '\x99': 'abc',
  '\x9A': 'bcd',
  '\x9B': 'cde',
  '\x9C': 'de',
  '\x9D': 'def',
  '\x9E': 'efa',
  '\x9F': 'fab',
  '\xA0': 'abcdefghjklmnp',
  '\xFF': 'abcdefghjklmnp',
  
  'null': 'null'
}

character_table = []
for i in xrange(256):
  segments = characters.get(chr(i), '')
  character_table.append(sum(MASKS[segment] for segment in segments))
  
characters = [('characters', character_table)]
