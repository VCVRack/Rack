#!/usr/bin/python2.5
#
# Copyright 2013 Olivier Gillet.
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
# Streaming .wav file writer.

import copy
import numpy
import struct

_DATA_CHUNK_HEADER_SIZE = 8
_FMT_CHUNK_DATA_SIZE = 16
_FMT_CHUNK_HEADER_SIZE = 8
_RIFF_FORMAT_DESCRIPTOR_SIZE = 4

_UNSIGNED_CHAR_TO_FLOAT_SCALE = 128.0
_FLOAT_TO_UNSIGNED_CHAR_SCALE = 127.0


class AudioStreamWriter(object):
  
  def __init__(self, file_name, sample_rate, bitdepth=16, num_channels=1):
    self._sr = sample_rate
    self._file_name = file_name
    self._sample_rate = sample_rate
    self._bitdepth = bitdepth
    self._num_channels = num_channels
    self._f = file(file_name, 'wb')
    self._num_bytes = 0
    self._write_header(False)
  
  @staticmethod
  def quantize(signal, bitdepth):
    """Convert an array of float to an array of integers.

    Args:
      signal: numpy array. source signal.
      bitdepth: int. size of the integer in bits.

    Returns:
      array of integers.
    """
    norm = numpy.abs(signal).max()

    # Normalization or clipping.
    scaled_signal = copy.copy(signal)
    if norm > 1.0:
      logging.warning('Some samples will be clipped.')
      # Clip samples above 1 and below -1.
      scaled_signal[scaled_signal < -1] = -1
      scaled_signal[scaled_signal > 1] = 1

    if bitdepth == 8:
      scaled_signal = (scaled_signal + 1.0) * _FLOAT_TO_UNSIGNED_CHAR_SCALE
      scaled_signal = numpy.array(scaled_signal, dtype=numpy.uint8)
    else:
      scale = (1 << (bitdepth - 1)) - 1
      # pylint: disable-msg=C6407
      scaled_signal = scaled_signal * scale
      scaled_signal = numpy.array(scaled_signal, dtype='i%d' % (bitdepth / 8))

    return scaled_signal
  
  def _write_header(self, restore_position):
    f = self._f
    
    total_size = _RIFF_FORMAT_DESCRIPTOR_SIZE
    total_size += _FMT_CHUNK_HEADER_SIZE + _FMT_CHUNK_DATA_SIZE
    total_size += _DATA_CHUNK_HEADER_SIZE + self._num_bytes
    
    current_position = f.tell()
    f.seek(0)
    f.write('RIFF')
    f.write(struct.pack('<L', total_size))
    f.write('WAVEfmt ')
    bitrate = self._sample_rate * self._num_channels * (self._bitdepth / 8)
    bits_per_sample = self._num_channels * (self._bitdepth / 8)
    f.write(struct.pack(
        '<LHHLLHH',
        16,
        1,
        self._num_channels,
        self._sample_rate,
        bitrate,
        bits_per_sample,
        self._bitdepth))
    f.write('data')
    f.write(struct.pack('<L', self._num_bytes))
    if restore_position:
      f.seek(current_position)
    
  def append(self, signal):
    if signal.dtype == numpy.uint8 or signal.dtype == numpy.int16:
      assert self._bitdepth == signal.dtype.itemsize * 8
      scaled_signal = signal
    else:
      scaled_signal = self.quantize(signal, self._bitdepth)

    if scaled_signal.ndim == 1:
      assert self._num_channels == 1
    else:
      assert self._num_channels == scaled_signal.shape[1]
    scaled_signal.tofile(self._f)
    self._num_bytes += scaled_signal.nbytes
    self._write_header(True)
