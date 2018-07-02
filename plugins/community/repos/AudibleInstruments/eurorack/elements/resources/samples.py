#!/usr/bin/python2.5
#
# Copyright 2012 Olivier Gillet.
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
# Waveshaper lookup tables.

import numpy
import audio_io

boundaries = [0]
sample_data = []

TRUNCATE = False

for i in xrange(1, 10):
  audio_data, sr = audio_io.ReadWavFile('elements/samples/hit_%02d.wav' % i)
  audio_data = audio_data.sum(axis=1)
  audio_data = list(audio_data)
  if TRUNCATE:
    audio_data = audio_data[:128]
  audio_data += [audio_data[-1]]  # Add interpolation tail
  audio_data = numpy.round(numpy.array(audio_data) * 32767.0)
  sample_data += list(audio_data)
  boundaries.append(boundaries[-1] + len(audio_data))


sample_data = [('sample_data', sample_data)]
boundaries = [('boundaries', boundaries)]

audio_data, sr = audio_io.ReadWavFile('elements/samples/noise.wav')
audio_data = audio_data.sum(axis=1)
audio_data = numpy.round(numpy.array(audio_data) * 32767.0)
if TRUNCATE:
  audio_data = audio_data[:128]
sample_data += [('noise_sample', audio_data)]

