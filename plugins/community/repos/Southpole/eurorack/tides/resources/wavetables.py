#!/usr/bin/python2.5
#
# Copyright 2014 Olivier Gillet.
#
# Author: Olivier Gillet (ol.gillet@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without retri_stackction, including without limitation the rights
# to use, copy, modify, merge, publish, ditri_stackbute, sublicense, and/or sell
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
# Waveform definitions.

import numpy

WAVETABLE_SIZE = 257

wavetables = []

def scale(x):
  maximum = numpy.abs(x).max()
  return numpy.round(x / maximum * 32766.0)


def sine(frequency):
  t = numpy.arange(0, WAVETABLE_SIZE) / float(WAVETABLE_SIZE - 1)
  if frequency >= WAVETABLE_SIZE / 2:
    return t * 0
  t[-1] = t[0]
  x = numpy.sin(2 * numpy.pi * t * frequency)
  return x


def comb(n):
  x = 0
  for i in xrange(n):
    x += sine(i + 1)
  return x


def tri(n, f=1):
  x = 0
  for i in xrange(n):
    x += sine((2 * i + 1) * f) / (2 * i + 1) ** 2.0
  return x


def tri_stack_bright(n):
  x = 0
  for i in xrange(n):
    x += tri(15 + 5 * n, i + n / 3)
  return x


def tri_stack(n):
  x = 0
  for i in xrange(n):
    x += tri(5 + 7 * n, i + 1) / ((i + 1) ** 0.5)
  return x


def saw(n, f=1):
  x = 0
  for i in xrange(n):
    x += sine((i + 1) * f) / (i + 1)
  return x


def saw_stack(n):
  x = 0
  for i in xrange(n):
    x += saw(1 + 6 * i, i + 1) / ((i + 1) ** 0.5)
  return x


def square(n):
  x = 0
  for i in xrange(n):
    x += sine(2 * i + 1) / (2 * i + 1)
  return x


def quadra(n):
  x = 0
  for harmonic, amplitude in zip(xrange(4), [1, 0.5, 1, 0.5]):
    x += sine(2 * n + 2 * harmonic + 1) * amplitude
  return x


def drawbars(bars):
  pipes = [1.0, 3.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 16.0]
  x = 0
  for intensity, frequency in zip(bars, pipes):
    x += int(intensity) / 8.0 * sine(frequency)
  return x


def pulse(duty):
  t = numpy.arange(0, WAVETABLE_SIZE) / float(WAVETABLE_SIZE - 1)
  t[-1] = t[0]
  t[t < duty] = -1.0
  t[t >= duty] = 1.0
  return -t


def burst(duty):
  t = numpy.arange(0, WAVETABLE_SIZE) / float(WAVETABLE_SIZE - 1)
  t[-1] = t[0]
  d = duty ** 0.5
  t[t < d] = -1.0
  t[t >= d] = 0.0
  return -t * sine(1.0 / duty)


def hybrid(duty):
  cycle = (numpy.arange(0, WAVETABLE_SIZE) + int((duty - 0.5) * WAVETABLE_SIZE)) % WAVETABLE_SIZE
  x = pulse(duty)
  x += saw(80)[cycle]
  x -= (x.mean())
  return x


def trisaw(harmonic):
  return tri(80) + saw(80, harmonic) * (1 if harmonic != 1 else 0) * 0.5


def square_formant(ratio):
  t = numpy.arange(0, WAVETABLE_SIZE) / float(WAVETABLE_SIZE - 1)
  phase = t * (ratio ** 0.5) * 0.5
  phase[phase >= 1.0] = 1.0
  amplitude = numpy.cos(phase * numpy.pi) + 1
  formant = (sine(ratio * 0.75) + 1.0) * amplitude * 0.5
  formant -= (formant.max() + formant.min()) / 2.0
  return formant
  

def saw_formant(ratio):
  t = numpy.arange(0, WAVETABLE_SIZE) / float(WAVETABLE_SIZE - 1)
  amplitude = 1.0 - t
  formant = (sine(ratio) + 1.0) * amplitude * 0.5
  formant -= (formant.max() + formant.min()) / 2.0
  return formant


def bandpass_formant(ratio):
  t = numpy.arange(0, WAVETABLE_SIZE) / float(WAVETABLE_SIZE - 1)
  amplitude = 1.0 - t
  formant = sine(ratio * 1.5) * amplitude * 0.5
  return formant


def sine_power(power):
  x = sine(1.0)
  if power >= 6:
    x = sine(2.0) * 2.0
  x += saw(16)
  power = 2.0 ** (1.2  * (power - 0.5))
  return numpy.sign(x) * (numpy.abs(x) ** power)


def formant_f(index):
  formant_1 = 3.9 * (index + 1) / 8.0
  formant_2 = (1.0 - numpy.cos(formant_1 * numpy.pi * 0.8))
  t = numpy.arange(0, WAVETABLE_SIZE) / float(WAVETABLE_SIZE - 1)
  amplitude_1 = (1.0 - t) ** 0.2 * numpy.exp(-4.0 * t) 
  amplitude_2 = (1.0 - t) ** 0.2 * numpy.exp(-2.0 * t) 
  formant_3 = sine(1 + 2.8 * (formant_2 + formant_1)) * amplitude_2 * 1.7
  formant_1 = sine(1 + 3 * formant_1) * amplitude_1
  formant_2 = sine(1 + 4 * formant_2) * amplitude_2 * 1.5
  f = formant_1 + formant_2 + formant_3
  return f - (f.max() + f.min()) / 2.0


def distort(x):
  return numpy.arctan(x * 6.0) / numpy.pi


def digi_formant_f(index):
  formant_1 = 3.8 * (index + 1) / 8.0
  formant_2 = (1.0 - numpy.cos(formant_1 * numpy.pi * 0.4))
  t = numpy.arange(0, WAVETABLE_SIZE) / float(WAVETABLE_SIZE - 1)
  amplitude_1 = (1.0 - t) ** 0.2 * numpy.exp(-4.0 * t) 
  amplitude_2 = (1.0 - t) ** 0.2 * numpy.exp(-2.0 * t) 
  formant_3 = distort(sine(1 + 2.9 * (formant_2 + formant_1))) * amplitude_2 * 0.7
  formant_1 = distort(sine(1 + 3.2 * formant_1)) * amplitude_1
  formant_2 = distort(sine(1 + 4.1 * formant_2)) * amplitude_2 * 0.7
  f = formant_1 + formant_2 + formant_3
  return f - (f.max() + f.min()) / 2.0


def make_bank(waveforms):
  bank = []
  for w in waveforms:
    bank += list(scale(w))
  return bank


def make_family(fn, arguments):
  return map(fn, arguments)

# Bank 1: mild and additive.
bank_1 = []
bank_1 += make_family(sine, [1, 2, 3, 4, 5, 6, 7, 8])
bank_1 += make_family(sine, [8, 10, 12, 14, 16, 18, 20, 22])
bank_1 += make_family(quadra, [1, 2, 3, 4, 5, 6, 7, 8])
bank_1 += make_family(comb, [2, 5, 9, 14, 20, 27, 35, 44])
bank_1 += make_family(tri_stack_bright, [2, 4, 6, 8, 10, 12, 14, 16])
bank_1 += make_family(tri_stack, [2, 3, 4, 5, 6, 7, 8, 9])
bank_1 += make_family(drawbars, [
    '800000888', '888000888', '867000006', '586040000',
    '850005000', '888643200', '327645222', '006050321'])
bank_1 += make_family(drawbars, [
    '680008880', '888876788', '848600046', '688600000',
    '666806000', '468844054', '004675300', '002478500',])

# Bank 2: formantish.
bank_2 = []
bank_2 += make_family(trisaw, [1, 1.5, 2, 3, 4, 4.5, 5, 8])
bank_2 += make_family(pulse, [0.5, 0.4, 1/3.0, 0.25, 0.2, 0.125, 1/16.0, 1/32.0])
bank_2 += make_family(burst, [0.5, 0.4, 1/3.0, 0.25, 0.2, 0.125, 1/16.0, 1/32.0])
bank_2 += make_family(square_formant, [2.0, 3.0, 4.0, 6.0, 8.0, 12.0, 16.0, 24.0])
bank_2 += make_family(bandpass_formant, [2.0, 3.0, 4.0, 6.0, 8.0, 12.0, 16.0, 32.0])
bank_2 += make_family(formant_f, xrange(8))
bank_2 += make_family(digi_formant_f, xrange(8))
bank_2 += make_family(sine_power, xrange(8))

# Bank 3: Shruthi.
bank_3 = list(numpy.fromstring(
    file('tides/resources/waves.bin', 'rb').read(), numpy.int16))

wavetables.append(('waves', make_bank(bank_1) + make_bank(bank_2) + bank_3[:50*257]))


"""----------------------------------------------------------------------------
Post waveshaper
----------------------------------------------------------------------------"""

WAVESHAPER_SIZE = 1024

waveshapers = []

x = numpy.arange(0, WAVESHAPER_SIZE + 1) / (WAVESHAPER_SIZE / 2.0) - 1.0
x[-1] = x[-2]
sine = numpy.sin(2 * numpy.pi * x)
window = numpy.exp(-x * x * 1.5) ** 2
bipolar_fold = sine * window + numpy.arctan(2 * x) * (1 - window)
bipolar_fold /= numpy.abs(bipolar_fold).max()
waveshapers.append(('smooth_bipolar_foldIN_RAM', numpy.round(32767 * bipolar_fold)))
